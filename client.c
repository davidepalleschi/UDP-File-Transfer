#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<netdb.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
#include<math.h>
#include<sys/mman.h>
#include<sys/time.h>
#include<errno.h>
#include<pthread.h>

#define PORT 1024 
#define SA struct sockaddr
#define BUFFER_SIZE 50
#define NAME_LEN 128
#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define NOT_ACCEPTABLE 406
#define SERVICE_UNAVAILABLE 503
#define ADDRESS "127.0.0.1"
#define PAYLOAD 1024

#define fflush(stdin) while(getchar()!='\n'){}


int main() 
{ 
	int socket_fd, msg_rec, len; 
	struct sockaddr_in server_addr; 
	char buffer[BUFFER_SIZE];
	char cmd[BUFFER_SIZE];
	int server_addr_len;
	int ret;	//variabile per il controllo dei return values
	int filename_len;
	char filename[NAME_LEN];
	int file_len;
	char **file_buffer;
	int num_pkt;
	int fd;

	//Create Socket
	socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_fd == -1){
		printf("Error: cannot create socket. %s\n", strerror(errno));
		exit(-1);
	}
	else{
		printf("Socket created succesfully.\n");
	}

	server_addr_len = sizeof(server_addr);

	// assign IP, PORT

	bzero(&server_addr, server_addr_len);
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr(ADDRESS); 
	server_addr.sin_port = htons(PORT);
   
// per provare la funzione chat scommenta l'istruzione successiva
//goto prova_davide;

	//pulizia buffer di messaggio
	bzero(buffer, BUFFER_SIZE);
	ret = sendto(socket_fd, buffer, sizeof(buffer), 0, (SA *)&server_addr, server_addr_len);
	if (ret == -1){
		printf("sendto() error while sending welcome packet.\n");
		exit(-1);
	}
	printf("INSERT A COMMAND:\n"
			"- exit\n"
			"- list\n"
			"- get\n"
			"- put\n");

	//preparo il buffer
	bzero(buffer, BUFFER_SIZE);
	//ricevo la nuova porta per la connessione dal server
	ret = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (SA *) &server_addr, &server_addr_len);
	if (ret == -1){
		printf("recvfrom() error in waiting for new port from server.\n");
		exit(-1);
	}
	// chiudo la connessione di accettazione tramite la porta di benvenuto
	close(socket_fd);

	int response = atoi(buffer);
	if (response == SERVICE_UNAVAILABLE){
		printf("Server is out of service.\n");
		exit(-1);
	}

	/* creazione della nuova socket */

	socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	server_addr_len = sizeof(server_addr);
	if (socket_fd == -1){
		printf("Cannot create the socket: %s", strerror(errno));
		exit(-1);
	}else {
		printf("Socket created succesfully.\n");
	}

	bzero(&server_addr, server_addr_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(response);

	/* corpo del client */

	while (1) {

		//preaparo l'area di comando
		bzero(cmd, BUFFER_SIZE);

		//ottengo la stringa dell'utente clien
		
		scanf("%s",cmd);

		/* EXIT COMMAND */
		if (strcmp("exit", cmd) == 0){
			printf("Client is closing connection...\n");
			ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				printf("sendto() error while trying to exit connection.\n");
				exit(-1);
			}
			close(socket_fd);
			printf("Client disconnected.\n");
			exit(0);
		} else		

		/* LIST COMMAND */

		if (strcmp("list", cmd) == 0){
			ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				printf("sendto() error while sending list command.\n");
				exit(-1);
			}
list:		// preparo il buffer in ricezione
			bzero(buffer, BUFFER_SIZE);
			ret = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (SA *) &server_addr, &server_addr_len);
			if (ret == -1){
				if (errno == EAGAIN) goto list;
				else {
					printf("recvfrom() error while receiving list from server.\n");
					exit(-1);
				}
			}
			if (atoi(buffer) == SERVICE_UNAVAILABLE){
				printf("Server has shutdown.\n");
				exit(-1);
			}

			printf("List of available files:\n%s\n", buffer);
		}else 

		/* GET COMMAND */

		if (strcmp("get", cmd) == 0){
			printf("sending get command to server.\n");
			ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				printf("sendto() error while sending get command to server.\n");
				exit(-1);
			}
download:
			//preparo il buffer in ricezione
			bzero(buffer, BUFFER_SIZE);
			//ricevo la lista dei file scaricabili
			ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &server_addr_len);
			if (ret == -1){
				if (errno == EAGAIN) goto download;
				else {
					printf("recvfrom() error while receiving download informations from server.\n");
					exit(-1);
				}
			}

			if (atoi(buffer) == SERVICE_UNAVAILABLE){
				printf("Server has shutdown.\n");
				exit(-1);
			}

			printf("Downloading is allowed.\n");
			//stampa della lista di file disponibili
			printf("Available files:\n%s\n", buffer);

get_insert:	// preparo il buffer in trasmissione
			bzero(cmd, BUFFER_SIZE);
			printf("Insert the name of file to be downloaded...\n");
			scanf("%s", cmd);

			filename_len = strlen(cmd);
			if (cmd[filename_len - 1] == '\n') cmd[filename_len - 1] = '\0';
			strcpy(filename, cmd);

			// invio la richiesta di download del file
			ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				printf("sendto() error while sending filename to be downloaded.\n");
				exit(-1);
			}

			// ricevo un feedback sulla correttezza del nome del file da scaricare
			bzero(buffer, BUFFER_SIZE);
			ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &server_addr_len);
			if (ret == -1){
				printf("recvfrom() error while receiving file name feedback.\n");
				exit(-1);
			}

			if (atoi(buffer) == NOT_FOUND) goto get_insert;

			//ricezione della lunghezza del file
			bzero(buffer, BUFFER_SIZE);
			ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &server_addr_len);
			if (ret == -1){
				printf("recvfrom() error while receiving file length");
				exit(-1);
			}

			// allocazione dinamica per la ricezione del file
			// verra allocata tanta memoria quanta indicata dalla lunghezza del file

			file_len = atoi(buffer);
			file_buffer = (char **) malloc(file_len * sizeof(char *));
			if (file_buffer == NULL){
				printf("Out of memory. (malloc error)\n");
				exit(-1);
			}

			//calcolo num_packet
			num_pkt = ceil((file_len/PAYLOAD)) + 1;
			for (int i = 0; i<num_pkt; i++){
				file_buffer[i] = (char *) malloc(PAYLOAD * sizeof(char));
				if (file_buffer[i] == NULL){
					printf("Out of memory. (malloc error)\n");
					exit(-1);
				}
			}

			//creazione file
			printf("Creating file %s...\n", filename);
			fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
			if (fd == -1){
				printf("open() error while creating file\n");
				exit(-1);
			}
			
			//SONO ARRIVATO QUI 18:35 28/02/2020

			


		}

		/* PUT COMMAND */
		


	}













    
	
	
	
	
	
	return 0;

	if (0){
		/* *** FUNZIONE PER CHAT *** */ 
	prova_davide:
		scanf("%s",cmd);
		sendto(socket_fd,cmd,sizeof(cmd),0, (SA*) & server_addr,server_addr_len); 
		recvfrom(socket_fd, (char *) buffer, sizeof(buffer), 0, (SA *) &server_addr, &server_addr_len);
		printf("%s",buffer);
		close(socket_fd);
		return 0;
		/* ************************* */
	}
	
}


