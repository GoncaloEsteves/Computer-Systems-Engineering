#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <future>
#include <vector>
#include <pthread.h>
#include <chrono>

#include "../include/hashtable.cpp"

namespace sthread {
    struct ServerThread {
        ServerThread(int new_socket, hasht::HashTable *table, std::atomic<int> *acc, std::vector<std::future<void>> *tasks){
			(*tasks).push_back(std::async(
				[&](int new_socket, hasht::HashTable *table, std::atomic<int> *acc) -> void {
					int size = 1 + 20 + 1024 + 2;
					int bytes;
					char* buffer = (char*) malloc(sizeof(char)*size);
            		long key = 0;
					char* value = NULL;
	        
            		while(read(new_socket, buffer, size)){

						bytes = strlen(buffer);

						char* token3 = NULL;
						char* token2 = NULL;
						char* token1 = NULL;
					
						if(bytes > 3){
							token1 = strndup(buffer, 1);

							if(token1 != NULL){
								token2 = strndup(buffer+2, 20);

								if(token2 != NULL){
									std::chrono::high_resolution_clock::time_point t1;
									std::chrono::high_resolution_clock::time_point t2;
									std::chrono::duration<double, std::milli> time_span;
					
									switch(token1[0]){
										case 'p':
											token3 = strndup(buffer+23, 1024);

											if(token3 != NULL){
												key = atol(token2);

  												t1 = std::chrono::high_resolution_clock::now();
												(*table).put(key, token3);
  												t2 = std::chrono::high_resolution_clock::now();
 												time_span = t2 - t1;

	        									sprintf(buffer, "%lfms", time_span.count());

	        									send(new_socket, buffer, strlen(buffer), 0);
											
												free(token3);
											}
											break;

										case 'g':
											key = atol(token2);

											t1 = std::chrono::high_resolution_clock::now();
											value = (*table).get(key);
  											t2 = std::chrono::high_resolution_clock::now();
 											time_span = t2 - t1;

	        								sprintf(buffer, "%lfms", time_span.count());

        									send(new_socket, buffer, strlen(buffer), 0);

											free(value);
											break;

										default:
											break;
									}

									free(token2);
								}

								free(token1);
							}
						}

						(*acc)++;
					}
					free(buffer);

					close(new_socket);

			}, new_socket, table, acc));
        }

		ServerThread(hasht::HashTable *table, std::vector<std::future<void>> *tasks){
			(*tasks).push_back(std::async(
				[&](hasht::HashTable *table) -> void {
					char *aux = NULL, *lastbackup = NULL;
					while(true){
						sleep(300);
						aux = (*table).createBackup();

						if(lastbackup != NULL){
							if(remove(lastbackup) != 0)
    							perror("Error deleting file");

							free(lastbackup);
						}

						lastbackup = strdup(aux);
						free(aux);
					}
			}, table));
        }

		ServerThread(std::atomic<int> *acc, std::vector<std::future<void>> *tasks){
			(*tasks).push_back(std::async(
				[&](std::atomic<int> *acc) -> void {
					while(true){
						sleep(1);
						printf("Acessos feitos: %d\n", (*acc).load());
						(*acc) = 0;
					}
			}, acc));
        }
    };
}