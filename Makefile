CC = g++
OPENCV =  `pkg-config --cflags --libs opencv`
PTHREAD = -pthread

SENDER = src/sender.cpp src/library.cpp
RECEIVER = src/receiver.cpp src/library.cpp
AGENT = src/agent.c
SEND = server
RECV = receiver
AGE = agent

all: sender receiver agent
  
sender: $(SENDER)
	$(CC) $(SENDER) -o $(SEND) $(OPENCV) $(PTHREAD) 
receiver: $(RECEIVER)
	$(CC) $(RECEIVER) -o $(RECV) $(OPENCV) $(PTHREAD)
agent: $(AGENT)
	gcc $(AGENT) -o $(AGE)

debug: senderDEBUG receiverDEBUG

senderDEBUG: $(SENDER)
	$(CC) $(SENDER) -DDEBUG -o $(SEND) $(OPENCV) $(PTHREAD) 
receiverDEBUG: $(RECEIVER)
	$(CC) $(RECEIVER) -DDEBUG -o $(RECV) $(OPENCV) $(PTHREAD)

.PHONY: clean

clean:
	rm $(SEND) $(RECV)