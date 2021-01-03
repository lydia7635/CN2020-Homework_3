#include "../inc/header.h"

using namespace std;
using namespace cv;

extern VideoCapture cap;

typedef enum {      // for creating nodes in buffer
    resolutionPkt,  // pkt of video resolution
    frameSizePkt,   // pkt of frame size
    framePkt,       // pkt of part of frame
    endVideo        // no need to create nodes
} SENDTASK;

BUFFER *sendBuffer;
int completeSendVideo = 0;

void addToBuffer(char *inputStr, int inputLen, int seqNum, int isFin, int isAck)
{
    BUFFERNODE *bufferNode = (BUFFERNODE *)malloc(sizeof(BUFFERNODE));
    bufferNode->segment = createSendSegment(inputStr, inputLen, seqNum, isFin, isAck);
    bufferNode->isSent = 0;
    bufferNode->next = NULL;
    if(sendBuffer->nodeNum == 0) {
        sendBuffer->head = bufferNode;
        sendBuffer->tail = bufferNode;
    }
    else {
        sendBuffer->tail->next = bufferNode;
        sendBuffer->tail = bufferNode;
    }
    ++(sendBuffer->nodeNum);
    return;
}

void sendVideo(int senderSocket, struct sockaddr_in sender, struct sockaddr_in agent,
    socklen_t sender_size, socklen_t agent_size)
{
    /* get the resolution of the video */
    int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
#ifdef DEBUG
    fprintf(stderr, "Video %d x %d\n", width, height);
#endif

    /* allocate container to load frames */
    Mat imgServer;
    imgServer = Mat::zeros(height, width, CV_8UC3);    
    if(imgServer.isContinuous()){
         imgServer = imgServer.clone();
    }

    /* init buffer and threshold */
    sendBuffer = initBuffer();
    int threshold = 16;
    int windowSize = 1;
    int seqNum = 0;

    SENDTASK sendTask = resolutionPkt;
    int sentNum = 0;    // number of pkts that have been sent
    int recvAckNum = 0;
    BUFFERNODE *nextSendPtr = NULL;

    char sendMessage[MAXDATA];

    /* about frame size and data */
    int frameSize;
    uchar *frameBuffer = NULL;
    int putFrameSizeTotal, curPutFrameSize;

    // /* create thread for recv ACK */
    // pthread_t recvAckTid;
    // if(pthread_create(&recvAckTid, NULL, recvAck, (void *)) != 0) {
    //     fprintf(stderr, "Error for creating thread.\n");
    //     exit(1);
    // }

    while(!completeSendVideo) {
        /* create at least <windowSize> number of nodes in buffer to be sent */
        while(sendBuffer->nodeNum < windowSize && sendTask != endVideo) {
            ++seqNum;
            bzero(sendMessage, sizeof(char) * MAXDATA);
            switch(sendTask) {
                case resolutionPkt:
                    sprintf(sendMessage, "%05d %05d\n", width, height);
                    addToBuffer(sendMessage, 11, seqNum, 0, 0);
                    sendTask = frameSizePkt;
                    break;
                case frameSizePkt:
                    cap >> imgServer;
                    if(imgServer.empty()) {
                        /* create fin pkt node*/
                        addToBuffer(sendMessage, 0, seqNum, 1, 0);
                        sendTask = endVideo;
                    }
                    else {
                        /* create frame size pkt and prepare frame data */
                        frameSize = imgServer.total() * imgServer.elemSize();
                        frameBuffer = (uchar *)malloc(sizeof(uchar) * frameSize);
                        memcpy(frameBuffer, imgServer.data, frameSize);
                        putFrameSizeTotal = 0;

                        sprintf(sendMessage, "%09d\n", frameSize);
                        addToBuffer(sendMessage, 9, seqNum, 0, 0);
                        sendTask = framePkt;
                    }
                    break;
                case framePkt:
                    if(putFrameSizeTotal + MAXDATA < frameSize) {
                        curPutFrameSize = MAXDATA;
                        memcpy(sendMessage, &frameBuffer[putFrameSizeTotal], sizeof(char) * curPutFrameSize);
                        addToBuffer(sendMessage, curPutFrameSize, seqNum, 0, 0);
                        putFrameSizeTotal += curPutFrameSize;
                    }
                    else {
                        curPutFrameSize = frameSize - putFrameSizeTotal;
                        memcpy(sendMessage, &frameBuffer[putFrameSizeTotal], sizeof(char) * curPutFrameSize);
                        addToBuffer(sendMessage, curPutFrameSize, seqNum, 0, 0);
                        putFrameSizeTotal += curPutFrameSize;

                        free(frameBuffer);
                        sendTask = frameSizePkt;
                    }
                    break;
                case endVideo:
                    /* no need to do something about buffer */
                    --seqNum;   // no need to add seqNum
                    break;
                default: 
                    fprintf(stderr, "Error. Unknown task.\n");
                    exit(1);
            }

            // if(buffer->nodeNum == 1) {
            //     curNodePtr = buffer->head;
            // }
            // else if(curNodePtr == NULL && buffer->nodeNum != 0) {
            //     /* when moving window */
            //     curNodePtr = buffer->tail;
            // }
        }

        nextSendPtr = sendBuffer->head;
        sentNum = nextSendPtr->segment->head.seqNumber - 1;
        /* send at most <window size> number of pkts*/
        while(sentNum < recvAckNum + windowSize && nextSendPtr != NULL) {
            nextSendPtr->segment->head.winSize = windowSize;
            nextSendPtr->segment->head.base = recvAckNum;
            sendData(senderSocket, nextSendPtr->segment, &agent, agent_size);
            if(nextSendPtr->isSent) {
                /* resend pkt */
                if(nextSendPtr->segment->head.fin) {
                    printf("resnd\tfin\n");
                }
                else {
                    printf("resnd\tdata\t#%d,\twinSize = %d\n", nextSendPtr->segment->head.seqNumber, windowSize);
                }
            }
            else {
                if(nextSendPtr->segment->head.fin) {
                    printf("send\tfin\n");
                }
                else {
                    printf("send\tdata\t#%d,\twinSize = %d\n", nextSendPtr->segment->head.seqNumber, windowSize);
                }
                nextSendPtr->isSent = 1;
            }
            ++sentNum;
            nextSendPtr = nextSendPtr->next;
        }

        /* preparing select fd_set and timeout */
        fd_set originalSet, workingSet;
        FD_ZERO(&originalSet);
        FD_SET(senderSocket, &originalSet);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int isTimeout = 0;
        int recvCorrectAckNum = 0; // in this round, not total

        /* receive correct ack or timeout */
        while(!isTimeout && recvCorrectAckNum < windowSize && !completeSendVideo) {
            memcpy(&workingSet, &originalSet, sizeof(fd_set));
            int selectResult;
            if((selectResult = select(MAXFD, &workingSet, NULL, NULL, &timeout)) < 0) {
                fprintf(stderr, "Error: select return -1.\n");
                exit(1);
            }
            else if(selectResult == 0) {
                /* timeout */
                isTimeout = 1;
                threshold = (windowSize / 2 > 1)? windowSize / 2 : 1;
                windowSize = 1;
                nextSendPtr = sendBuffer->head;
                printf("time\tout,\t\tthreshold = %d\n", threshold);
            }
            else {
                /* receive ACK */
                SEGMENT *recvSegment = recvData(senderSocket);
                if(recvSegment->head.fin) {
                    ++recvCorrectAckNum;
                    completeSendVideo = 1;
                    freeOneBufferNode(sendBuffer);
                    printf("recv\tfinack\n");
                }
                else if (recvSegment->head.ackNumber == sendBuffer->head->segment->head.seqNumber) {
                    /* get correct ACK */
                    ++recvCorrectAckNum;
                    freeOneBufferNode(sendBuffer);
                    printf("recv\tack\t#%d\n", recvSegment->head.ackNumber);
                }
                else {
                    /* we lose target pkt */
                    printf("recv\tack\t#%d\n", recvSegment->head.ackNumber);
                }
            }
        }
        if(!isTimeout) {
            if(windowSize < threshold)
                windowSize *= 2;
            else
                windowSize += 1;
        }
        recvAckNum += recvCorrectAckNum;
    }

    return;
}