CC = g++
CFLAGS = -Wall -std=c++11

all: sender receiver

sender: sender.cpp packet.hpp
	$(CC) $(CFLAGS) sender.cpp -o sender

receiver: receiver.cpp packet.hpp
	$(CC) $(CFLAGS) receiver.cpp -o receiver

clean:
	rm -f sender receiver