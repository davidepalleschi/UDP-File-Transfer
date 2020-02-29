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

/* defines */
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
#define TX_WINDOW 3
#define TIMER 300000
#define LOSS_PROBABILITY 0.15


#define fflush(stdin) while(getchar()!='\n'){}

/* nuovi tipi */
typedef struct _packet_form{
	int counter;
	char buf[BUFFER_SIZE];
	int ack;
} packet_form;

/* variabili globali */
char buffer[BUFFER_SIZE];
char **file_buffer;
int num_pkt;
int socket_fd, msg_rec, len; 
int server_addr_len;
struct sockaddr_in server_addr;

/* funzioni */
void receive_packets(void);
int send_ack(int seq);


int main() {  
	char cmd[BUFFER_SIZE];
	int ret;	//variabile per il controllo dei return values
	int filename_len;
	char filename[NAME_LEN];
	int file_len;
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
sen:
			// invio la richiesta di download del file
			ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				if (errno == EINTR || errno == EAGAIN) goto sen;
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
			
			receive_packets();
			printf("Packets received succesfully.\nCreating file...");
			for (int i=0; i<num_pkt; i++){
				ret = write(fd, file_buffer[i], PAYLOAD);
				if (ret == -1){
					printf("write() error on creating file from packets.\n");
					exit(-1);
				}
			}
			printf("File written succesfully.\n");
			printf("Updating file list...\n");
			//write file list


			


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


void receive_packets(){

	int ret; //variabile per il controllo dei return values
	int pkts = 0;
	int counter = 0;
	int seq = 0;
	int window = TX_WINDOW;
	int offset = num_pkt%window;
	packet_form packets[num_pkt];

	struct timeval timer;
	timer.tv_sec = 0;
	timer.tv_usec = TIMER;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));
	// in ricezione finché non sono ottenuti tutti i pacchetti
	while (pkts < num_pkt || counter <= num_pkt - 2){
		if (TX_WINDOW % 2 == 1) window = offset;
		else window = offset + 1;


		// pacchetti ricevuti "window" alla volta
		char pkt[BUFFER_SIZE];
		char *RX_pkt;
		for (int i=0; i<window; i++){
	receive:
			printf("");
			RX_pkt = (char *) malloc(PAYLOAD * sizeof(char));
			if (RX_pkt == NULL){
				printf("malloc error.\n");
				exit(-1);
			}
			bzero(pkt, BUFFER_SIZE);
			bzero(RX_pkt, PAYLOAD);
			ret = recvfrom(socket_fd, pkt, BUFFER_SIZE, 0, (SA *) &server_addr, &server_addr_len);
			if (ret == -1){
				if (errno == EAGAIN) goto catchemall;
				else{
					printf("recvfrom() error while receiving packets.\n");
					exit(-1);
				}
			}
			printf("");

			// apprendimento della sequenza del pacchetto
			seq = atoi(strtok(pkt, " "));
			if (seq >= pkts) pkts = seq + 1;
			printf("");
			// ricezione del payload
			char *c_index;
			sprintf(c_index, "%d", seq);
			int starting = strlen(c_index) + 1;
			char *beginning = &pkt[starting];
			char *end = &pkt[BUFFER_SIZE];
			char *payload = (char *) malloc((end - beginning + 1) * sizeof(char)); // malloc((BUFFER_SIZE*seq - starting + 1) * sizeof(char));
			if (payload == NULL){
				printf("malloc error.\n");
				exit(-1);
			}
			memcpy(payload, beginning, end - beginning);
			RX_pkt = payload;
			///SENDACK
			if (send_ack(seq)){
				if (strcpy(packets[seq].buf, RX_pkt) == NULL){
					printf("strcpy error while receiving packet %s.\n", RX_pkt);
					exit(-1);
				}
				if (strcpy(file_buffer[seq], packets[seq].buf) == NULL){
					printf("strcpy error while receiving packet (%s, %s).\n", file_buffer[seq], packets[seq].buf);
					exit(-1);
				}
			}
	catchemall:
			printf("");
		}
	}
	return;
}

int send_ack(int seq){

	int ret;
	float loss_prob;
	bzero(buffer, BUFFER_SIZE);
	sprintf(buffer, "%d", seq);
	
	if (seq > num_pkt - TX_WINDOW - 1) loss_prob = 0;
	else loss_prob = LOSS_PROBABILITY;

	float rndm = (float) random() / RAND_MAX;
	if (rndm < (1 - loss_prob)){
		ret = sendto(socket_fd, buffer, 32, 0, (SA *) &server_addr, server_addr_len);
		if (ret == -1){
			printf("sendto() error while sending ack.\n");
			exit(-1);
		}
		return 1;
	}else return 0;
}


