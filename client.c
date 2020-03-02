#include"uft_client/includes.h"

#define fflush(stdin) while(getchar()!='\n'){}

// use argument adpt to use adaptive timeout
int main(int argc, char **argv) { 

	/* variabili locali */

	int ret;	//variabile per il controllo dei return values

	int file_len;
	int fd;
	struct sigaction act;
	sigset_t set;

	/* controllo argomento */

	if (argc == 2){
		if (strcmp(argv[1], "adpt") == 0){
			adaptive = 1;
		}
	}

	/* gestione SIGINT */

	sigfillset(&set);
	act.sa_sigaction = SIGINT_handler;
	act.sa_mask = set;
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);

	//creazione socket per il benvenuto
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
	strcpy(buffer, "hello");
	ret = sendto(socket_fd, buffer, sizeof(buffer), 0, (SA *)&server_addr, server_addr_len);
	if (ret == -1){
		printf("sendto() error while sending welcome packet.\n");
		exit(-1);
	}
	
	display();

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

	int response = atoi(buffer); //nuovo numero di porta
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
command:
		//preaparo l'area di comando
		bzero(cmd, BUFFER_SIZE);

		//ottengo la stringa dell'utente clien
		
		scanf("%[^\n]",cmd);
		fflush(stdin);

		/* EXIT COMMAND */
		if (strcmp("exit", cmd) == 0){
			printf("Client is closing connection...\n");
			// comunico al server l'uscita di questo client
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

			printf("\nList of available files:\n%s\n", buffer);
			display();
		}else 

		/* GET COMMAND */

		if (strncmp("get", cmd, 3) == 0){
			printf("sending get command to server.\n");
			ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				printf("sendto() error while sending get command to server.\n");
				exit(-1);
			}

		strtok(cmd, " ");
		strcpy(filename, strtok(NULL, ""));

///*
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

			char *string_code;
			string_code = strtok(buffer, " ");
			int code = atoi(string_code);

			if(code != OK){
				if (code == SERVICE_UNAVAILABLE){
					printf("Server has shutdown.\n");
					exit(-1);
				}else if (code == NOT_FOUND){
					printf("\nSTATUS CODE: %d FILE NOT FOUND\n\n", NOT_FOUND);
					display();
					goto command;
				}
				exit(-1);
			}

/*

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
	//		printf("controllo correttezza nome file %s\n", buffer);

			if (atoi(buffer) == NOT_FOUND) goto get_insert;
*/
			//apprendimento lunghezza file

			char *file_len_string;
			file_len_string = strtok(NULL, "");
			file_len = atoi(file_len_string);

			// allocazione dinamica per la ricezione del file
			// verra allocata tanta memoria quanta indicata dalla lunghezza del file

			printf("\n"
					"STATUS CODE : %d OK\n"
					"FILE LENGTH: %d\n"
					"NOTE: packet receiving...\n\n", code, file_len);

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
			FILE *file_list;
			file_list = fopen("file_list.txt", "a");
			if (file_list == NULL){
				printf("fopen() error while updating file_list.txt");
				exit(-1);
			}else{
				pthread_mutex_lock(&m);
				fprintf(file_list, "\n,%s", filename);
				pthread_mutex_unlock(&m);
			}
			fclose(file_list);
			printf("File list updated succesfully.\n");
			display();
		}else	

		/* PUT COMMAND */
		if (strncmp("put", cmd, 3) == 0){
			/*ret == sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				printf("sendto() error while sending put command to server.\n");
				exit(-1);
			}*/

			strtok(cmd, " ");
			strcpy(filename, strtok(NULL, ""));
			char file[30]= DIRECTORY;
			strcat(file,filename);
		

			//apro il file per saperne la lunghezza
			int fdtemp = open(file, O_RDONLY, 0666);
			if (fdtemp == -1){
				printf("open() error while checking file length.\n");
				exit(-1);
			}
			file_len = lseek(fdtemp, 0, SEEK_END);
			close(fdtemp);

			char file_len_str[17]; //16 cifre circa 2^54 bytes
			bzero(file_len_str, sizeof(file_len_str));
			sprintf(file_len_str, "%d", file_len);
		/**/printf("controllo correttezza file len %s\n", file_len_str);

			strcat(cmd, " ");
			strcat(cmd, filename);
			strcat(cmd, " ");
			strcat(cmd, file_len_str);

		/**/printf("controllo correttezza stringa inviata %s\n", cmd);

			ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				printf("sendto() error while sending put command to server.\n");
				exit(-1);
			}

	///*
put_insert:
			//ricezione del permesso di put
			bzero(buffer, BUFFER_SIZE);
			ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &server_addr_len);
			if (ret == -1){
				if (errno == EAGAIN) goto put_insert;
				else{
					printf("recvfrom() error while receiving put permission.\n");
					exit(-1);
				}
			}

//		printf("controllo correttezza risposta server = %s\n", buffer);

			int code = atoi(buffer);

			if (code!= OK){
				//server non in funzione
				if (code == SERVICE_UNAVAILABLE){
					printf("Server is out of service.\n");
					kill(getpid(), SIGINT);
				}else if (code == NOT_ACCEPTABLE){
					printf("\nServer response:\n\n"
							"STATUS CODE: %d NOT ACCEPTABLE FILE\n"
							"NOTE: Server cannot accpet file.\n\n", code);
					display();
					goto command;
				}else exit(-1);
			}
			/*
			// stampo la lista dei filename presente nel file_list del client
			fdtemp = open("file_list.txt", O_RDONLY, 0666);
			if (fdtemp == -1){
				printf("open() error while trying to open file_list.txt");
				kill(getpid(), SIGINT);
			}*//*

			int no_bytes_read = lseek(fd, 0, SEEK_END);

			file_list_buffer = (char *) malloc(no_bytes_read * sizeof(char));
			if (file_list_buffer == NULL){
				printf("malloc error.\n");
				exit(-1);
			}
			lseek(fdtemp, 0, 0);
			ret = read(fdtemp, file_list_buffer, no_bytes_read);
			if (ret == -1){
				printf("read() error while reading server file list.\n");
				exit(-1);
			}
			printf("List of available files in client:\n%s\n", file_list_buffer);
			//close(fdtemp);
			printf("Please insert file to be updated...\n"); */
			
			//fd = file_to_send(); //????????

			num_pkt = ceil(file_len/PAYLOAD);
			
			printf("Starting file upload...\n");

			
			//start_sending_pckt(fd);

			// invio dei pacchetti
			packet_form file_form[num_pkt];
			for(int i=0; i<num_pkt; i++){
				bzero(file_form[i].buf, BUFFER_SIZE);
			}

			int counter = 0;
			char *temp_payload = (char *) malloc(PAYLOAD * sizeof(char));
			if (temp_payload == NULL){
				printf("malloc error for temporary payload.\n");
				exit(-1);
			}

			fd = open(filename, O_RDONLY, 0666);{
				if (fd == -1){
					printf("open error while reading file.\n");
					exit(-1);
				}
			}
			/* LOGICA DEI PACCHETTI */
			for(int i=0; i<num_pkt; i++){
				bzero(temp_payload, PAYLOAD);
				ret = read(fd, temp_payload, PAYLOAD);
				if (ret == -1){
					printf("read() error while reading from file.\n");
					exit(-1);
				}

				char pkt[BUFFER_SIZE];
				if (sprintf(pkt, "%d", i) < 0){
					printf("sprintf() error while forming packets.\n");
					exit(-1);
				}

				if (strcat(pkt, temp_payload) == NULL){
					printf("strcat() error while forming packets.\n");
					exit(-1);
				}

/**/			printf("controllo correttezza PACKET = %s\n", file_form[i].buf);
				file_form[i].counter = i;
			}

			int seq = 0;
			struct timeval timer;
			timer.tv_sec = 0;
			timer.tv_usec = SENDING_TIMER;
			setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));
			int offset = num_pkt % TX_WINDOW;
			if (offset > 0){ //more pkts than tx_window size
				if (num_pkt - seq >= offset){
					while( seq < num_pkt - offset){
						seq = send_pkt(file_form, seq, TX_WINDOW);
					}
				}
				seq = send_pkt(file_form, seq, offset);
			}else {
				while(seq < num_pkt){
					seq = send_pkt(file_form, seq, TX_WINDOW);
				}
			}

			/* FINE LOGICA DEI PACCHETTI */

			display();
		}


	}













    
	
	
	
	
	
	return 0;
/*
	if (0){
		 //*** FUNZIONE PER CHAT *** 
	prova_davide:
		scanf("%s",cmd);
		sendto(socket_fd,cmd,sizeof(cmd),0, (SA*) & server_addr,server_addr_len); 
		recvfrom(socket_fd, (char *) buffer, sizeof(buffer), 0, (SA *) &server_addr, &server_addr_len);
		printf("%s",buffer);
		close(socket_fd);
		return 0;
		 //************************* 
	}
	*/
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
/**/		printf("controllo correttezza pacchetto ricevuto %s\n", pkt);
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
/**/		printf("controllo correttezza payload del pacchetto %s.\n", RX_pkt);
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
/**/ 	printf("controllo correttezza sending ACK %s.\n", buffer);
		ret = sendto(socket_fd, buffer, 32, 0, (SA *) &server_addr, server_addr_len);
		if (ret == -1){
			printf("sendto() error while sending ack.\n");
			exit(-1);
		}
		return 1;
	}else return 0;
}

void SIGINT_handler(int signo, siginfo_t *a, void *b){
	int ret;
	printf("Client is closing connection...\n");
	bzero(cmd, sizeof(cmd));
	strcpy(cmd, "exit");
	ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
	if (ret == -1){
		printf("sendto() error while trying to exit connection.\n");
		exit(-1);
	}
	close(socket_fd);
	printf("Client disconnected.\n");
	exit(0);
}

int file_to_send(){
	int ret;
	int size;
name_insert:
	bzero(cmd, BUFFER_SIZE);
	scanf("%s", cmd);
	fflush(stdin);
	int filename_len = strlen(cmd);
	bzero(filename, NAME_LEN);
	strcpy(filename, cmd);
	printf("Opening file...\n");
	int fd = open(filename, O_RDONLY, 0666);
	if (fd == -1){
		printf("open error: file doesn't exist.\n");
		goto name_insert;
	}
	ret = sendto(socket_fd, cmd, sizeof(cmd), 0, (SA *) &server_addr, server_addr_len);
	if (ret == -1){
		printf("sendto() error in file_to_send().\n");
		exit(-1);
	}
/**/ printf("controllo correttezza: voglio aggiornare file %s\n", cmd);
	size = lseek(fd, 0, SEEK_END);
	num_pkt = ceil(size/PAYLOAD) + 1;
/**/ printf("controllo correttezza il numero di pacchetti da caricare è %d.\n", num_pkt);
	lseek(fd, 0, 0);

	//invio lunghezza
	bzero(buffer, BUFFER_SIZE);
	sprintf(buffer, "%d", size);
	ret = sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, server_addr_len);
	if (ret == -1){
		printf("sendto error while sending filename length.\n");
		exit(-1);
	}
/**/ printf("controllo correttezza invio la lunghezza del file %s.\n", buffer);

	return fd;
}

// returns actual sequence number
// if (argv[1] == adpt) ==> adaptive timeout
int send_pkt(packet_form *file_form, int seq, int size){
	int ret;
	int lock;
	double send_timer = 0.1;
	int startRTT = 0, endRTT = 0;
	struct timeval time;
	for(int i=0; i<size; i++){
		lock = 0;
		file_form[seq].ack = 0;
		if (adaptive){
			set_adpt_timeout(send_timer);
			int ret = gettimeofday(&time, NULL);
			if (ret == -1){
				printf("gettimeofday() error while sending packet.\n");
				exit(-1);
			}		
			startRTT = time.tv_sec + (time.tv_usec / 1000000.0);
		} else {
			//setting timeout value
			struct timeval timer;
			timer.tv_sec = 0;
			timer.tv_usec = TIMER;
			setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));
		}
		ret = sendto(socket_fd, file_form[seq].buf, BUFFER_SIZE, 0, (SA *) &server_addr, server_addr_len);
		if (ret == -1){
			fprintf(stderr, "sendto() error while sending pkt %d.\n", seq);
			exit(-1);
		}

		bzero(buffer, BUFFER_SIZE);
		ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &server_addr_len);
		if (ret == -1){
			if (errno == EAGAIN) {
				lock = 1;
/**/			printf("controllo correttezza. Pacchetto perso: ack %d non ricevuto.\n", seq);
			} else {
				printf("recvfrom error while server is sending packet.\n");
				exit(-1);
			}
		}

		//setting variable adaptive timeout interval
		if (adaptive){
			int ret;
			ret = gettimeofday(&time, NULL);
			if (ret == -1){
				printf("gettimeofday error while server is sending packet.\n");
				exit(-1);
			}
			endRTT = time.tv_sec + (time.tv_usec / 1000000.0);
			//todo get_adpt_timeout [get_adaptive_timeout]
			get_adpt_timeout(startRTT, endRTT);
		}
		if (!lock){
			int check;
			check = atoi(buffer);
			if (check >= 0){
				file_form[check].ack = 1;
			}
		}
		seq ++;
	}
	//check dell'arrivo di tutti i pacchetti
	//todo check_pkt [check_packet_sendend_of_window]
	check_pkt(file_form, size, seq);
	return seq;
}

//time amount we want to set waiting time socket
void set_adpt_timeout(double time){
	if (time < MIN_SENDING_TIMER) time = MIN_SENDING_TIMER;
	struct timeval timer;
	timer.tv_sec = 0;
	timer.tv_usec = time*100000;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));
}

void get_adpt_timeout(double start, double end){
	double sampleRTT = start - end;
	estimateRTT = ((1 - ALPHA) * estimateRTT) + (ALPHA * (sampleRTT));
	devRTT = ((1 - BETA) * (devRTT + BETA)) * (fabs(((sampleRTT) - estimateRTT)));
	sending_timeout = estimateRTT + (4 * devRTT);
}

//
void check_pkt(packet_form *file_form, int offset, int seq){
	int ret;
	for (int i= base; i<(base + offset); i++){
		if (file_form[i].ack != 1){
retry:
			ret = sendto(socket_fd, file_form[i].buf, BUFFER_SIZE, 0, (SA *) &server_addr, server_addr_len);
			if (ret == -1){
				fprintf(stderr, "sendto() error while sending packet no %d.\n", i);
				exit(-1);
			}

/**/ 		printf("controllo correttezza: sending packet no %d.\n", i);

			bzero(buffer, BUFFER_SIZE);
			ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &server_addr_len);
			if (ret == -1){
				if (errno == EAGAIN){
					if (i == num_pkt -1){
						goto done;
					}
					usleep(50);
					goto retry;
				} else {
					printf("recvfrom() error while checking packet.\n");
					exit(-1);
				}
			}
done:
			printf("");
			int check = atoi(buffer);
			check = atoi(buffer);
			if (check > 0){
				file_form[check].ack = 1;
			}
		}
	}
}

