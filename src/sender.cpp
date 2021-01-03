#include "../inc/header.h"

using namespace std;
using namespace cv;

VideoCapture cap;

void parseArg(int argc, char *argv[], char *sendIP, int *sendPort, char *agentIP, int *agentPort)
{
    if(argc != 4) {
        fprintf(stderr,"Usage: %s <sender port> <agent IP> <agent port>\n", argv[0]);
        fprintf(stderr, "E.g., ./sender 7001 local 7002\n");
        exit(1);
    }
    setIP(sendIP, "local");
    sscanf(argv[1], "%d", sendPort);
    setIP(agentIP, argv[2]);
    sscanf(argv[3], "%d", agentPort);
    return;
}

int main(int argc, char *argv[])
{
    /* for arguments*/
    int sendPort, agentPort;
    char sendIP[IPlen], agentIP[IPlen];
    parseArg(argc, argv, sendIP, &sendPort, agentIP, &agentPort);

#ifdef DEBUG
    fprintf(stderr, "sender info: ip = %s port = %d\n", sendIP, sendPort);
    fprintf(stderr, "agent info: ip = %s port = %d\n", agentIP, agentPort);
#endif

    /* create folder */
    createFolder("./sender_dir/\0");

    /* Create UDP socket */
    int senderSocket = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in sender, agent;
    socklen_t sender_size, agent_size;

    /* Configure settings in sender struct */
    sender.sin_family = AF_INET;
    sender.sin_port = htons(sendPort);
    sender.sin_addr.s_addr = inet_addr(sendIP);
    memset(sender.sin_zero, '\0', sizeof(sender.sin_zero));

    /*Configure settings in agent struct*/
    agent.sin_family = AF_INET;
    agent.sin_port = htons(agentPort);
    agent.sin_addr.s_addr = inet_addr(agentIP);
    memset(agent.sin_zero, '\0', sizeof(agent.sin_zero));

    /* Initialize size variable to be used later on */
    sender_size = sizeof(sender);
    agent_size = sizeof(agent);

    /* bind socket */
    bind(senderSocket, (struct sockaddr *)&sender, sender_size);

    /* receive play command */
    SEGMENT *recvSegment = recvData(senderSocket);
    char videoName[MAXDATA];
    memcpy(videoName, recvSegment->data, sizeof(char) * MAXDATA);
    freeSegment(recvSegment);

    /* start to figure out video */
    if(cap.open(videoName) == 0) {
        fprintf(stderr, "Open mpg file error.\n");
        exit(1);
    }

    sendVideo(senderSocket, sender, agent, sender_size, agent_size);

#ifdef DEBUG
    fprintf(stderr, "sendVideo() is successful\n");
#endif
    
    cap.release();
    return 0;
}