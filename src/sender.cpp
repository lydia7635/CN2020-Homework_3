#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "opencv2/opencv.hpp"
#include "../inc/header.h"

using namespace std;
using namespace cv;

void parseArg(int argc, char *argv[], int *sendPort, char *agentIP, int *agentPort, char **fileName)
{
	if(argc != 5) {
		fprintf(stderr,"Usage: %s <sender port> <agent IP> <agent port> <source_file>\n", argv[0]);
        fprintf(stderr, "E.g., ./server 7001 local 7002 <source_file>\n");
        exit(1);
	}
	sscanf(argv[1], "%d", sendPort);
	setIP(agentIP, argv[2]);
	sscanf(argv[3], "%d", agentPort);
	*fileName = argv[4];
	return;
}

int main(int argc, char *argv[])
{
	int sendPort, agentPort;
	char agentIP[IPlen];
	char *fileName;
	parseArg(argc, argv, &sendPort, agentIP, &agentPort, &fileName);

	VideoCapture cap;
	Mat imgServer;
	return 0;
}