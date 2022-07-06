#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "../include/serverthread.cpp"

#define PORT 8080

hasht::HashTable initializeTable(int argc, char const *argv[]){
	if(argv[1][0] == 's')
		return hasht::HashTable(atoi(argv[2]));
	else if(argv[1][0] == 'f')
		return hasht::HashTable(argv[2]);
	else
		return hasht::HashTable(1024);
}

int main(int argc, char const *argv[]){

	if(argc != 3 || (argv[1][0] != 's' && argv[1][0] != 'f')){
		return 0;
	}

	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
	    perror("socket failed");
	    exit(EXIT_FAILURE);
	}
	
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
	    perror("setsockopt");
	    exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
	    perror("bind failed");
	    exit(EXIT_FAILURE);
	}

	int flag = 1;
	hasht::HashTable table = initializeTable(argc, argv);
  	
	std::vector<std::future<void>> tasks;
	std::atomic<int> acc(0);

	sthread::ServerThread backups(&table, &tasks);
	sthread::ServerThread acesses(&acc, &tasks);

	while(flag){
		if (listen(server_fd, 3) < 0){
		    perror("listen");
		    exit(EXIT_FAILURE);
	    }

	    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
		    perror("accept");
		    exit(EXIT_FAILURE);
	    }

		sthread::ServerThread thread(new_socket, &table, &acc, &tasks);
    }

	for(auto &t : tasks)
		t.get();
	
	table.freeTable();

	return 0;
}