#include "../inc/header.h"

void setIP(char *dst, const char *src)
{
    if(strncmp(src, "0.0.0.0", 7) == 0 || strncmp(src, "local", 5) == 0
            || strncmp(src, "localhost", 9)) {
        sscanf("127.0.0.1", "%s", dst);
    } else {
        sscanf(src, "%s", dst);
    }
}

void createFolder(const char *dirName)
{
    struct stat folder_info;
    if(!stat(dirName, &folder_info) == 0 || !(folder_info.st_mode & S_IFDIR))
        mkdir(dirName, 0777);
    chdir(dirName);
    return;
}

BUFFER *initBuffer()
{
    BUFFER *buffer = (BUFFER *)malloc(sizeof(BUFFER));
    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->nodeNum = 0;
    return buffer;
}

void freeOneBufferNode(BUFFER *buffer)
{
    if(buffer->nodeNum == 0) {
        fprintf(stderr, "Error. freeOneBufferNode is empty.\n");
        exit(1);
    }
    else if(buffer->nodeNum == 1) {
        freeSegment(buffer->head->segment);
        free(buffer->head);
        buffer->head = NULL;
        buffer->tail = NULL;
    }
    else {
        BUFFERNODE *tmp = buffer->head;
        buffer->head = buffer->head->next;
        freeSegment(tmp->segment);
        free(tmp);
    }
    --(buffer->nodeNum);

    return;
}

SEGMENT *recvData(int socket)
{
    SEGMENT *recvSegment = (SEGMENT *)malloc(sizeof(SEGMENT));
    struct sockaddr_in tmp_addr;
    socklen_t tmp_size;
    int recvTotalSize = 0, recvSize;

    bzero(recvSegment, sizeof(SEGMENT));

    while(recvTotalSize < sizeof(SEGMENT)) {
        if( (recvSize = recvfrom(socket, recvSegment + recvTotalSize,
                (int)sizeof(SEGMENT) - recvTotalSize, 0, (struct sockaddr *)&tmp_addr, &tmp_size)) < 0 ) {
            fprintf(stderr, "[%d] recv data failed. recvSize = %d\n", socket, recvSize);
            exit(1);
        } else if (recvSize == 0) {
            fprintf(stderr, "[%d] socket received nothing %d\n", socket, recvSize);
            exit(1);
        }
        recvTotalSize += recvSize;
    }
    return recvSegment;
}

SEGMENT *createSendSegment(char *inputStr, int inputLen, int seqAckNum, int isFin, int isAck)
{
    SEGMENT *sendSegment = (SEGMENT *)malloc(sizeof(SEGMENT));
    sendSegment->head.length = inputLen;
    if(isAck) {
        sendSegment->head.ackNumber = seqAckNum;
        sendSegment->head.seqNumber = 0;
    }
    else {
        sendSegment->head.seqNumber = seqAckNum;        
        sendSegment->head.ackNumber = 0;
        bzero(sendSegment->data, MAXDATA);
        memcpy(sendSegment->data, inputStr, inputLen);
    }
    sendSegment->head.ack = isAck;
    sendSegment->head.fin = isFin;
    sendSegment->head.syn = 0;
    /* if use malloc to create space in inputStr, function caller,
    ** function caller should free it by themselves. */
    return sendSegment;
}

void sendData(int socket, SEGMENT *sendSegment, struct sockaddr_in *receiver, socklen_t receiver_size)
{
    sendto(socket, sendSegment, sizeof(SEGMENT), 0, (struct sockaddr *)receiver, receiver_size);
    return;
}

void freeSegment(SEGMENT *recvSegment)
{
    free(recvSegment);
    return;
}