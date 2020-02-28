#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080 
#define SA struct sockaddr 



int main() 
{ 
	int sockfd, msg_rec, len; 
	struct sockaddr_in server_addr; 
	char buffer[20];
	char cmd[20];

	//Create Socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  

	// assign IP, PORT 
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	server_addr.sin_port = htons(PORT); 
   


	// function for chat 
	scanf("%s",cmd);
	sendto(sockfd,cmd,sizeof(cmd),0, (SA*) & server_addr,sizeof(server_addr)); 

	/*
    len = sizeof(server_addr);

	bzero(&buffer,sizeof(buffer));
	msg_rec = recvfrom(sockfd, (char *)buffer, sizeof(buffer),  0 , ( SA*)  &server_addr, &len);  

	printf("%s\n",buffer);
*/
    return 0;
}


