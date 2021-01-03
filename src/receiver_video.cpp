#include "../inc/header.h"

using namespace std;
using namespace cv;

typedef enum {
    resolutionPkt,
    frameSizePkt,
    framePkt,
    endVideo
} RECVTASK;

BUFFER *recvBuffer;
RECVTASK recvTask = resolutionPkt;

/* for playing video */
Mat imgClient;
int height, width;
int frameSize;
uchar *frameBuffer;
int putFrameSizeTotal, curPutFrameSize;

/* parameter:    in reality
** 
** senderSocket:  receiverSocket
** receiver:      agent
** receiver_size: agent_size */
void sendAck(int ackNum, int isFin, int senderSocket, struct sockaddr_in *receiver, socklen_t receiver_size)
{
    SEGMENT *sendSegment = createSendSegment(NULL, 0, ackNum, 0, 1);
    if(!isFin) {
        printf("send\tack\t#%d\n", ackNum);
    }
    else {
        printf("send\tfinack\n");
        sendSegment->head.fin = 1;
    }
    sendData(senderSocket, sendSegment, receiver, receiver_size);
    freeSegment(sendSegment);
    return;
}

/* return 0 if recvBuffer is full, so we should drop it. */
int addToBuffer(SEGMENT *recvSegment)
{
    if(recvBuffer->nodeNum >= MAXRECVBUF) {
        return 0;
    }

    BUFFERNODE *bufferNode = (BUFFERNODE *)malloc(sizeof(BUFFERNODE));
    bufferNode->segment = recvSegment;
    bufferNode->next = NULL;
    if(recvBuffer->nodeNum == 0) {
        recvBuffer->head = bufferNode;
        recvBuffer->tail = bufferNode;
    }
    else {
        recvBuffer->tail->next = bufferNode;
        recvBuffer->tail = bufferNode;
    }
    ++(recvBuffer->nodeNum);
    return 1;
}

void flushBuffer()
{
    printf("flush\n");
    while(recvBuffer->nodeNum > 0 && recvTask != endVideo) {
        printf("nodeNum = %d\n", recvBuffer->nodeNum);
        switch(recvTask) {
            case resolutionPkt:
                sscanf(recvBuffer->head->segment->data, "%d%d", &width, &height);
                imgClient = Mat::zeros(height, width, CV_8UC3);
                if(!imgClient.isContinuous()){
                    imgClient = imgClient.clone();
                }
                recvTask = frameSizePkt;
                break;
            case frameSizePkt:
                if(recvBuffer->head->segment->head.fin) {
                    /* frameSize == 0 */
                    recvTask = endVideo;
                }
                else {
                    sscanf(recvBuffer->head->segment->data, "%d", &frameSize);
                    frameBuffer = (uchar *)malloc(sizeof(uchar) * frameSize);
                    printf("frameSize = %d\n", frameSize);
                    putFrameSizeTotal = 0;
                    recvTask = framePkt;
                }
                break;
            case framePkt:
                curPutFrameSize = recvBuffer->head->segment->head.length;
                if(putFrameSizeTotal + curPutFrameSize < frameSize) {
                    memcpy(&frameBuffer[putFrameSizeTotal], recvBuffer->head->segment->data, sizeof(uchar) * curPutFrameSize);
                    putFrameSizeTotal += curPutFrameSize;
                }
                else {
                    memcpy(&frameBuffer[putFrameSizeTotal], recvBuffer->head->segment->data, sizeof(uchar) * curPutFrameSize);
                    putFrameSizeTotal += curPutFrameSize; 

                    /* we can play video */
                    memcpy(imgClient.data, frameBuffer, sizeof(uchar) * frameSize);
                    printf("play\n");
                    //imshow("Video", imgClient);
                    //char c = (char)waitKey(33.3333);
                    
                    if(frameBuffer != NULL) {
                        free(frameBuffer);
                        frameBuffer = NULL;
                    }
                    else {
                        printf("frameBuffer is NULL\n");
                    }

                    printf("after free\n");
                    recvTask = frameSizePkt;                   
                }
                break;
            case endVideo:
                break;
            default:
                fprintf(stderr, "Error: Unknown recvTask\n");
                exit(1);
        }
        freeOneBufferNode(recvBuffer);
    }
    return;
}

void recvVideoAndPlay(int receiverSocket, struct sockaddr_in receiver, struct sockaddr_in agent,
    socklen_t receiver_size, socklen_t agent_size)
{
    int completeRecvVideo = 0;

    int expectSeqNum = 1;
    int expectBase = 0, expectWinSize = 1;
    recvBuffer = initBuffer();

    while(!completeRecvVideo) {
        SEGMENT *recvSegment = recvData(receiverSocket);
        if(recvSegment->head.winSize != expectWinSize
                || recvSegment->head.base != expectBase) {
            
            /* not in the same round */
            expectWinSize = recvSegment->head.winSize;
            expectBase = recvSegment->head.base;
            if(recvBuffer->nodeNum >= MAXRECVBUF) {
                flushBuffer();
            }

            /* need to drop this pkt? */
        }
        
        /* in the same or new round */
        if(recvSegment->head.seqNumber != expectSeqNum) {
            /* #<expectSeqNum> pkt is lost. Drop this pkt. */
            // drop
            printf("drop\tdata\t#%d\n", recvSegment->head.seqNumber);
            /* send ack back */
            sendAck(expectSeqNum - 1, 0, receiverSocket, &agent, agent_size);
        }
        else {
            /* Recv expect pkt? */
            if( addToBuffer(recvSegment) ) {
                if(recvSegment->head.fin) {
                    completeRecvVideo = 1;
                    printf("recv\tfin\n");
                    sendAck(0, 1, receiverSocket, &agent, agent_size);
                    flushBuffer();
                }
                else {
                    printf("recv\tdata\t#%d\n", expectSeqNum);
                    sendAck(expectSeqNum, 0, receiverSocket, &agent, agent_size);
                    ++expectSeqNum;
                }
            }
            else {
                /* recvBuffer is full. */
                if(recvSegment->head.fin) {
                    printf("drop\tfin\n");
                    sendAck(expectSeqNum - 1, 0, receiverSocket, &agent, agent_size);
                }
                else {
                    printf("drop\tdata\t#%d\n", recvSegment->head.seqNumber);
                    sendAck(expectSeqNum - 1, 0, receiverSocket, &agent, agent_size);
                }
                freeSegment(recvSegment);
            }
            
        }
    }
    return;
}