#ifndef _HEADER_CNHW3_H_
#define _HEADER_CNHW3_H_

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/select.h>
#include "opencv2/opencv.hpp"

#define IPlen 50
#define MAXDATA 4096
#define MAXFD 40
#define MAXRECVBUF 32

typedef struct {
    int length;       // specified data length actually
    int seqNumber;    // for sender
    int ackNumber;    // for receiver to ack
    int fin;          // boolean
    int syn;          // boolean, but we set it to 0
    int ack;          // boolean
} HEADER;

typedef struct{
    HEADER head;
    char data[MAXDATA];
} SEGMENT;

typedef struct BufferNode {
    SEGMENT *segment; 
    int isSent;             // use for resnd
    struct BufferNode *next;
} BUFFERNODE;

typedef struct Buffer {
    BUFFERNODE *head;
    BUFFERNODE *tail;
    int nodeNum;
} BUFFER;

void setIP(char *dst, const char *src);
void createFolder(const char *dirName);
BUFFER *initBuffer();
void freeOneBufferNode(BUFFER *buffer);
SEGMENT *recvData(int socket);
SEGMENT *createSendSegment(char *inputStr, int inputLen, int seqAckNum, int isFin, int isAck);
void sendData(int socket, SEGMENT *sendSegment, struct sockaddr_in *receiver, socklen_t receiver_size);
void freeSegment(SEGMENT *recvSegment);

// for sender
void sendVideo(int senderSocket, struct sockaddr_in sender, struct sockaddr_in agent,
    socklen_t sender_size, socklen_t agent_size);

// for receiver
void recvVideoAndPlay(int receiverSocket, struct sockaddr_in receiver, struct sockaddr_in agent,
    socklen_t receiver_size, socklen_t agent_size);

#endif