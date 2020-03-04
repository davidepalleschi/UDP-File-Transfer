#include "../inc/cmd.h"
#include "../inc/types.h"
#include "../inc/func.h"

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

void cmd_list(int socket_fd, struct sockaddr_in client_addr){
    char buffer[50];
    int len=sizeof(client_addr);
    bzero(buffer,BUFFER_SIZE);
    strcpy(buffer,"200 ");
    sendto(socket_fd, strcat(buffer,dirfile()) , BUFFER_SIZE , 0, (SA *) &client_addr, len);
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

void cmd_recv_packets(char* file_name,char* file_size ,int socket_fd, struct sockaddr_in client_addr){
    srand(time(NULL));
    int len=sizeof(client_addr);
    int fd = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0666);
    int base=0;
    int num_pkt=ceil((atoi(file_size)/PAYLOAD));
    int counter=0;
    packet_form packet[num_pkt];
    printf("numero pacchetti: %d\n",num_pkt);
    printf("pacchetto %d: %d\n",num_pkt-1,packet[num_pkt-1].seq);
    while(counter<num_pkt){
        for(int i=base;i<WINDOW+base;i++){
            if(prob_perdita(20)){
                packet[i].seq=i;
                packet[i].recv=1;
                }
            printf("iteratore: %d\n",i);
            printf("num pak: %d\n",packet[i].seq);
            printf("counter %d\n",counter);
            if(packet[i].recv==1 && packet[i].checked==0 ){
                counter++;
                packet[i].checked=1;
            }
            //recvfrom(socket_fd_child, buffer, sizeof(buffer), 0, (SA *) &client_addr, &len);
            //strcpy(packet[i].seq,atoi(strtok(buffer," ")));
            //strcpy(packet[i].payload,strtok(NULL,""));
            //sendto(socket_fd, sfprintf("%d",packet.counter), sizeof(sfprintf("%d",packet.counter)) , 0, (SA *) &client_addr, len);
            if(packet[i].seq==base){
                //write(fd,packet[i].payload,PAYLOAD);
                
                if(!(WINDOW+base==num_pkt)){
                    base++;
                }
            }
        }
        printf("Fine for ----------------------------------\n");
    }
    printf("fine\n");
}

