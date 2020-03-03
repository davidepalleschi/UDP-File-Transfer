#include "../inc/cmd.h"

void cmd_list(int socket_fd, struct sockaddr_in client_addr){
    char buffer[50];
    int len=sizeof(client_addr);
    bzero(buffer,BUFFER_SIZE);
    sendto(socket_fd, dirfile() , BUFFER_SIZE , 0, (SA *) &client_addr, len);
}

void cmd_corr(char * file_name, int socket_fd, struct sockaddr_in client_addr){
    int len=sizeof(client_addr);
    sendto(socket_fd, ispresent(file_name), 50 , 0, (SA *) &client_addr, len);
}

void cmd_corr_put(char* file_size,int socket_fd, struct sockaddr_in client_addr){
    int len=sizeof(client_addr);
    if (atoi(file_size)>20000000){
        sendto(socket_fd, "406", 3 , 0, (SA *) &client_addr, len);
    }
    else{
        sendto(socket_fd, "200", 3 , 0, (SA *) &client_addr, len);
    }
}

void cmd_recv_packets(char* file_name,int socket_fd, struct sockaddr_in client_addr){
    int len=sizeof(client_addr);
    
}

int cmd_send_port(int socket_fd, struct sockaddr_in client_addr, int free_port[]){
    char buffer[BUFFER_SIZE];
    int client_port=0;
    int len = sizeof(client_addr);
    for(int i=0;i<MAX_CLIENTS;i++){
        if (free_port[i]==0){
            free_port[i]=1;
            client_port=PORT+i+1;
            break;
        }
        }
        if(client_port==0){
            printf("Numero limite di clients superato.\n");
            bzero(buffer, BUFFER_SIZE);
            sprintf(buffer, "%d", SERVICE_UNAVAILABLE);
            if (sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &client_addr, len) == -1){
                printf("sendto() error %d: %s", SERVICE_UNAVAILABLE, strerror(errno)); 
                }
            return -1;
        }

        bzero(buffer,BUFFER_SIZE);
        sprintf(buffer,"%d",client_port);
        sendto(socket_fd,buffer,BUFFER_SIZE,0,(SA*) &client_addr,len);
        return client_port;
}