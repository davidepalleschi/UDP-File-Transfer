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
#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define NOT_ACCEPTABLE 406
#define SERVICE_UNAVAILABLE 503
#define ADDRESS "127.0.0.1"


int main() 
{ 
	int socket_fd, msg_rec, len; 
	struct sockaddr_in server_addr; 
	char buffer[BUFFER_SIZE];
	char cmd[BUFFER_SIZE];
	int server_addr_len;
	int ret;	//variabile per il controllo dei return values

	//Create Socket
	socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_fd == -1){
		printf("Error: cannot create socket. %s\n", strerror(errno));
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
   
//goto prova_davide;

	





    
	return 0;

	if (0){
		/* *** FUNZIONE PER CHAT *** */ 
	prova_davide:
		scanf("%s",cmd);
		sendto(socket_fd,cmd,sizeof(cmd),0, (SA*) & server_addr,server_addr_len); 
		recvfrom(socket_fd, (char *) buffer, sizeof(buffer), 0, (SA *) &server_addr, &server_addr_len);
		printf("%s",buffer);
		return 0;
		/* ************************* */
	}
	
}


