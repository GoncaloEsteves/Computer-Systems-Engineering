all: client multiclient server

CC = g++
CXXFLAGS += -O3 -Wall -std=c++11

client: Client/client.cpp
	$(CC) -o bin/client.exe $(CXXFLAGS) Client/client.cpp
multiclient: Client/multiclient.cpp
	$(CC) -pthread -o bin/multiclient.exe $(CXXFLAGS) Client/multiclient.cpp
server: Server/server.cpp include/hashtable.cpp include/serverthread.cpp
	$(CC) -pthread -o bin/server.exe $(CXXFLAGS) Server/server.cpp include/hashtable.cpp include/serverthread.cpp

.PHONY: clean

clean:
	rm -f bin/*.exe