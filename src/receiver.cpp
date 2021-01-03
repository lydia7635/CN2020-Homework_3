#include "../inc/header.h"

using namespace std;
using namespace cv;

void parseArg(int argc, char *argv[], char *recvIP, int *recvPort, char *agentIP, int *agentPort)
{
    if(argc != 4) {
        fprintf(stderr,"Usage: %s <receiver port> <agent IP> <agent port>\n", argv[0]);
        fprintf(stderr, "E.g., ./receiver 7003 local 7002\n");
        exit(1);
    }
    setIP(recvIP, "local");
    sscanf(argv[1], "%d", recvPort);
    setIP(agentIP, argv[2]);
    sscanf(argv[3], "%d", agentPort);
    return;
}

char *parseCmd()
{
    char cmdStr[MAXDATA];
    bzero(cmdStr, sizeof(char) * MAXDATA);
    cin.getline(cmdStr, sizeof(char) * MAXDATA, '\n');

    char bufCMD[MAXDATA], bufOTHER[MAXDATA];    // use bufOTHER when too many arguments
    char *bufFILE = (char *)malloc(sizeof(char) * MAXDATA);
    bzero(bufCMD, sizeof(char) * MAXDATA);
    bzero(bufFILE, sizeof(char) * MAXDATA);
    int argcnt = sscanf(cmdStr, "%s%s%s", bufCMD, bufFILE, bufOTHER);
    if(argcnt != 2) {
        fprintf(stderr, "Error command. usage: play <video_file>\n");
        exit(1);
    }

    return bufFILE;
}

int main(int argc, char *argv[])
{
    /* for arguments*/
    int recvPort, agentPort;
    char recvIP[IPlen], agentIP[IPlen];
    parseArg(argc, argv, recvIP, &recvPort, agentIP, &agentPort);

#ifdef DEBUG
    fprintf(stderr, "agent info: ip = %s port = %d\n", agentIP, agentPort);
    fprintf(stderr, "receiver info: ip = %s port = %d\n", recvIP, recvPort);
#endif

    /* create folder */
    createFolder("./receiver_dir/\0");

    /* init buffer and threshold */
    BUFFER *recvBuffer = initBuffer();
    int threshold = 16;

    /* Create UDP socket */
    int receiverSocket = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in agent, receiver;
    socklen_t agent_size, receiver_size;


    /*Configure settings in agent struct*/
    agent.sin_family = AF_INET;
    agent.sin_port = htons(agentPort);
    agent.sin_addr.s_addr = inet_addr(agentIP);
    memset(agent.sin_zero, '\0', sizeof(agent.sin_zero));

    /* Configure settings in receiver struct */
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(recvPort);
    receiver.sin_addr.s_addr = inet_addr(recvIP);
    memset(receiver.sin_zero, '\0', sizeof(receiver.sin_zero));

    /* Initialize size variable to be used later on */
    agent_size = sizeof(agent);
    receiver_size = sizeof(receiver);

    /* bind socket */
    bind(receiverSocket, (struct sockaddr *)&receiver, receiver_size);

    /* specified video file to play */
    char *inputStr = parseCmd();
    SEGMENT *sendSegment = createSendSegment(inputStr, MAXDATA, 0, 0, 0);
    free(inputStr);
    sendData(receiverSocket, sendSegment, &agent, agent_size);
    freeSegment(sendSegment);

    recvVideoAndPlay(receiverSocket, receiver, agent, receiver_size, agent_size);
    
    return 0;
}