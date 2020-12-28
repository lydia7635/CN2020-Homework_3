#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "opencv2/opencv.hpp"
#include "../inc/header.h"

void parseArg(int argc, char *argv[], int *recvPort, char *agentIP, int *agentPort)
{
	if(argc != 4) {
		fprintf(stderr,"Usage: %s <receiver port> <agent IP> <agent port>\n", argv[0]);
        fprintf(stderr, "E.g., ./receiver 7003 local 7002\n");
        exit(1);
	}
	sscanf(argv[1], "%d", recvPort);
	setIP(agentIP, argv[2]);
	sscanf(argv[3], "%d", agentPort);
	return;
}

int main(int argc, char *argv[])
{
	int recvPort, agentPort;
	char agentIP[IPlen];
	parseArg(argc, argv, &recvPort, agentIP, &agentPort);
	
	return 0;
}