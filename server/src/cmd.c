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
    char buffer[BUFFER_SIZE];   // @@@ no 50 si BUFFER_SIZE
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
    if (fd == -1){
        printf("Server couldn't open %s.\n", buffer);
    }
    int num_pkt=ceilf((float)(atoi(file_size)/(float)PAYLOAD)); //@@@ no ceil si ceilf e aggiunto casting a float
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
    if(num_pkt<window){
        window=num_pkt;
    }
    while(counter < num_pkt){
        
            rec:
            bzero(buffer,sizeof(buffer));
            printf("Sto aspettando il pacchetto...\n");

            recvfrom(socket_fd, buffer, sizeof(buffer), 0, (SA *) &client_addr, &len);

            strncpy(seq,buffer,64);
            printf("Il pacchetto %s è arrivato\n",seq);
            if(prob(PROBABILITY_LOSS) || (atoi(seq) > window + base)){
                printf("Il pacchetto %s è stato perso\n",strtok(buffer," "));
                goto rec;
            } 
            if(packet[atoi(seq)].recv == 0 ){
                counter++;
                packet[atoi(seq)].recv ++;
            }  
            
            //strcpy(packet[i].payload, buffer+64);                             //@@@ no
            memcpy(packet[atoi(seq)].payload, buffer + 64, PAYLOAD);            //@@@ si
       // printf("\n\n\n"); fflush(stdout); write(STDOUT_FILENO, packet[i].payload, BUFFER_SIZE); printf("\n\n\n");
            sendto(socket_fd, seq, sizeof(seq) , 0, (SA *) &client_addr, len);

            for(int i=base; i<base + window; i++){
                if (packet[i].recv){
                    if (packet[i].seq == base){
                        // se bisogna togliere la printf qui sotto va lasciata l'istruzione di write
                        printf("@@@ byte scritti dal pkt %d al file %ld\n", i, write(fd, packet[i].payload, PAYLOAD));
                        base ++;
                    }
                }
            }

            /*if(packet[atoi(seq)].recv == 1){
                if(atoi(seq)==base){
                    // se bisogna togliere la printf qui sotto va lasciata l'istruzione di write
                    printf("@@@byte scritti dal pkt %d al file %ld\n", atoi(seq), write(fd,packet[atoi(seq)].payload,PAYLOAD));
                    //if (base < num_pkt - window) base ++;
                    base++;
                }
            }*/
        
        printf("---------------------------------------------------\n");
    }
    printf("File received\n");
    close(fd);
}

