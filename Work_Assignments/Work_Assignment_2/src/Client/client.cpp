#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define PORT 8080

int main(int argc, char const *argv[]){

	if((argc < 2) ||
	   (argv[1][0] != 'g' && argv[1][0] != 'p') ||
	   (argv[1][0] == 'g' && argc < 3))
		return 0;

	int sock = 0;
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

	long key;

	int aux, digits, size = 1 + 20 + 1024 + 3;
	char *message = new char[size];
	char *message2 = new char[1024];

	switch(argv[1][0]){
		case 'p':
			if(argc >= 3)
				key = atoi(argv[2]);
			else
				key = rand();
			
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
				message[i] = (rand()%127);
				if(message[i] < 41)
					message[i] += 41;
			}
			message[aux+1024] = '\0';

			send(sock, message, strlen(message), 0);

			read(sock, message2, 1024);
			printf("%s\n", message2);
			break;

		case 'g':
			key = atol(argv[2]);
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
			printf("%s\n", message2);
			break;

		default:
			break;
	}

	close(sock);

	free(message);
	free(message2);

	return 0;
}