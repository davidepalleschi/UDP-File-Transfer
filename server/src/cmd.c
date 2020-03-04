#include "../inc/cmd.h"
#include "../inc/types.h"
#include "../inc/func.h"
#include "../inc/libraries.h"
#include "../inc/config.h"

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
    char buffer[BUFFER_SIZE];
    bzero(buffer,sizeof(buffer));
    char seq[64];
    bzero(seq,sizeof(seq));
    int len=sizeof(client_addr);
    strcpy(buffer,DIRECTORY);
    strcat(buffer,file_name);
    int fd = open(buffer, O_CREAT | O_RDWR | O_TRUNC, 0666);
    int num_pkt=ceil((atoi(file_size)/PAYLOAD));
    int base=0;
    int counter=0;
    packet_form packet[num_pkt];
    for(int i=0; i<num_pkt;i++){
        packet[i].seq=0;
        packet[i].recv=0;
        packet[i].checked=0;
    }
    printf("numero pacchetti: %d\n",num_pkt);
    printf("pacchetto %d: %d\n",num_pkt-1,packet[num_pkt-1].seq);
    int window=WINDOW;
    if(num_pky<win){
        window=num_pkt;
    }
    while(counter < num_pkt){
        for(int i=base ;i<WINDOW+base ; i++){
            rec:
            bzero(buffer,sizeof(buffer));
            printf("Sto aspettando il pacchetto...\n");

            recvfrom(socket_fd, buffer, sizeof(buffer), 0, (SA *) &client_addr, &len);

            strncpy(seq,buffer,64);
            printf("Il pacchetto %s è arrivato\n",seq);
            if(prob_perdita(100)){
                printf("Il pacchetto %s è stato perso\n",strtok(buffer," "));
                goto rec;
                }
            packet[i].recv=1;
            if(packet[i].checked==0 ){
                counter++;
                packet[i].checked=1;
            }
            
            packet[i].seq=atoi(seq);
            strcpy(packet[i].payload, buffer+64);
            fflush(stdout);
            write(1,packet[i].payload);
            sendto(socket_fd, seq, sizeof(seq) , 0, (SA *) &client_addr, len);
            if(packet[i].recv==1){
                if(packet[i].seq==base){
                    write(fd,packet[i].payload,PAYLOAD);
                
                    if(!(WINDOW+base==num_pkt)){
                        base++;
                    }
                }
            }
            printf("num pak: %d recv: %d checked: %d\n",packet[i].seq, packet[i].recv,packet[i].checked);
            printf("counter: %d\n",counter);
        }
        printf("Fine for ----------------------------------\n");
    }
    printf("fine\n");
    close(fd);
}

