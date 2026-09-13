CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall

.PHONY: all clean

all: server client 

server: server_folder/server.cpp 
	$(CXX) $(CXXFLAGS) server_folder/server.cpp -o server_folder/server

client: client_folder/client.cpp
	$(CXX) $(CXXFLAGS) client_folder/client.cpp -o client_folder/client

clean:
	rm -f server_folder/server client_folder/client
