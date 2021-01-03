CC = g++
OPENCV =  `pkg-config --cflags --libs opencv`

SENDER = src/sender.cpp src/library.cpp src/sender_video.cpp
RECEIVER = src/receiver.cpp src/library.cpp src/receiver_video.cpp
AGENT = src/agent.cpp
SEND = sender
RECV = receiver
AGE = agent

all: sender receiver agent
  
sender: $(SENDER)
	$(CC) $(SENDER) -o $(SEND) $(OPENCV)
receiver: $(RECEIVER)
	$(CC) $(RECEIVER) -o $(RECV) $(OPENCV)
agent: $(AGENT)
	$(CC) $(AGENT) -o $(AGE)

debug: senderDEBUG receiverDEBUG agentDEBUG

senderDEBUG: $(SENDER)
	$(CC) $(SENDER) -DDEBUG -o $(SEND) $(OPENCV)
receiverDEBUG: $(RECEIVER)
	$(CC) $(RECEIVER) -DDEBUG -o $(RECV) $(OPENCV)
agentDEBUG: $(AGENT)
	$(CC) $(AGENT) -DDEBUG -o $(AGE)

.PHONY: clean

clean:
	rm $(SEND) $(RECV) $(AGE)
