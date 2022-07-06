#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <future>
#include <vector>
#include <pthread.h>

#define PORT 8080

int main(int argc, char const *argv[]){

	if((argc < 4) ||
	   (argv[1][0] != 'g' && argv[1][0] != 'p'))
		return 0;

    std::vector<std::future<double>> tasks;

    long clients = atol(argv[2]);
	int requests = atoi(argv[3]);

    for(long key = 0; key < clients; key++){
        tasks.push_back(std::async(
			[&](long key, int requests) -> double {
	            int sock;
	            struct sockaddr_in serv_addr;

	            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		            printf("\n Socket creation error\n");
		            return -1;
	            }

	            serv_addr.sin_family = AF_INET;
	            serv_addr.sin_port = htons(PORT);
	
	            if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
		            printf("\nInvalid address/ Address not supported\n");
		            return -1;
	            }

	            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		            printf("\nConnection Failed\n");
		            return -1;
	            }

	            srand(time(NULL));

	            int aux, digits, size = 1 + 20 + 1024 + 3;
				char *message = new char[size];
				char *message2 = new char[1024];
				double ret = -1;
				unsigned int myseed = key;
				int i = 0;

	            while(i < requests){
					key = rand_r(&myseed);

					switch(argv[1][0]){
		            	case 'p':
							if(ret == -1)
								ret = 0;
	
							digits = 0;
							aux = key;
			
							while(aux > 0){
								aux = aux/10;
								digits++;
							}

							if(key == 0)
								digits++;

							sprintf(message, "p ");

							for(auto i = 0; i < 20-digits; i++)
								message[2+i] = '0';

							sprintf(message+2+(20-digits), "%ld ", key);
			
			            	aux = strlen(message);
                        	for(auto i = aux; i < aux+1024; i++){
				            	message[i] = (rand_r(&myseed)%127);
				            	if(message[i] < 41)
					            	message[i] += 41;
	                    	}

			            	message[aux+1024] = '\0';

			            	send(sock, message, strlen(message), 0);

			            	read(sock, message2, 1024);
                        	ret += atof(message2);
			            	break;

		            	case 'g':
							if(ret == -1)
								ret = 0;

							digits = 0;
							aux = key;
			
							while(aux > 0){
								aux = aux/10;
								digits++;
							}

							if(key == 0)
								digits++;

							sprintf(message, "g ");

							for(auto i = 0; i < 20-digits; i++)
								message[2+i] = '0';

							sprintf(message+2+(20-digits), "%ld ", key);

			            	send(sock, message, strlen(message), 0);

			            	read(sock, message2, 1024);
                        	ret += atof(message2);
			            	break;

		            	default:
			            	break;
	            	}
					i++;
				}

				close(sock);

				free(message);
				free(message2);

				if(ret != -1)
					ret = ret/requests;

                return ret;
		    }, key, requests));
    }

    double total = 0;
	double N = 0;
    double aux;

	for(auto &t : tasks){
        aux = t.get();
        if(aux != -1){
            total += aux;
			N++;
		}
    }

    printf("Tempo médio: %lfms\n", (total/N));

	return 0;
}