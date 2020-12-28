#ifndef _HEADER_CNHW3_H_
#define _HEADER_CNHW3_H_

#define IPlen 50

typedef struct {
	int length;
	int seqNumber;
	int ackNumber;
	int fin;
	int syn;
	int ack;
} header;

typedef struct{
	header head;
	char data[1000];
} segment;

void setIP(char *dst, char *src);

#endif