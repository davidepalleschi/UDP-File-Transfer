/*
 * (1) Memoria condivisa con i processi figli
 * (2) Gestione segnali
 *      - SIGCHLD per la terminazione di un processo figlio
 *      - SIGUSR1 provola il decremento di num_client
 * (3) Creazione processo per richiesta di terminazione server
 * 
 *  
*/
#include "../inc/libraries.h"
#include"../inc/config.h"
#include "../inc/func.h"

int socket_fd;
int num_client = 0;



int main(int argc, char **argv){
    

    /* MAIN PROCESS SERVER STACK */


    pid_t child_pids[MAX_CLIENTS];
    pid_t parent_pid = getpid();
    struct sockaddr_in  client_addr;
    int msgrcv, len;
    char buffer[BUFFER_SIZE];
    struct sigaction act_int;
    struct sigaction act_chld;
    sigset_t set_int;
    sigset_t set_chld;
    int port = 0,client_port;

    /* gestione segnali */

    sigfillset(&set_int);
    act_int.sa_sigaction = interrupt_handler;
    act_int.sa_mask = set_int;
    act_int.sa_flags = 0;
    sigaction(SIGINT, &act_int, NULL);

    sigfillset(&set_chld);
    act_chld.sa_sigaction = child_death_handler;
    act_chld.sa_mask = set_chld;
    act_chld.sa_flags = 0;
    sigaction(SIGCHLD, &act_chld, NULL);
    /*bzero(buffer,BUFFER_SIZE);
    dirfile(buffer);
    printf("%s\n",buffer);*/

    /* Inizializzazione socket */
    printf("Start Server\n");

    socket_fd=create_socket(PORT);


    while (1) {
        //signal(SIGCHLD,SIG_IGN);
        //preparo il buffer del messaggio da ricevere
        bzero(buffer, BUFFER_SIZE);
                
        //preparo la ricezione dell'indirizzo del client
        rec:
        bzero(&client_addr, sizeof(client_addr));
        len = sizeof(client_addr);

        //ricevo il messaggio dal client

        msgrcv = recvfrom(socket_fd, (char *) buffer, sizeof(buffer), 0, (SA *) &client_addr, &len);

        if (msgrcv == -1){
            if (errno == EINTR) goto rec;
            printf("rcvfrom() error, error: %s .\n", strerror(errno));
            bzero(buffer, BUFFER_SIZE);
        }


        if(num_client >= MAX_CLIENTS){
            printf("Numero limite di clients superato.\n");
            bzero(buffer, BUFFER_SIZE);
            sprintf(buffer, "%d", SERVICE_UNAVAILABLE);
            if (sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &client_addr, len) == -1)
                printf("sendto() error %d: %s", SERVICE_UNAVAILABLE, strerror(errno));
        }
        else{
            //calcolo porta per il client
            num_client++;
            port++;
            client_port=PORT + port;
            bzero(buffer,BUFFER_SIZE);
            sprintf(buffer,"%d",client_port);
            sendto(socket_fd,buffer,BUFFER_SIZE,0,(SA*) &client_addr,len);
            pid_t pid=fork();
            if (pid==-1){
                printf("Fork error while creating process for new client");
            }
            if(pid==0){
                //gestione segnali TODO
                signal(SIGCHLD,SIG_IGN);
                int socket_fd_child=create_socket(client_port);
                printf("Numero Clienti: %d\n\n",num_client);
                
                
                while (1){

                    bzero(buffer,BUFFER_SIZE);
                    recvfrom(socket_fd_child, buffer, sizeof(buffer), 0, (SA *) &client_addr, &len);
                    char tok[50];
                    char* token=tok;
                    char size[50];
                    char* sizep=size;
                    token=strtok(buffer," ");


                    if(strcmp("list", token) == 0){
                        printf("Sto processando la richiesta di list del client collegato alla porta: %d.\n", client_port);
                        cmd_list(socket_fd_child,client_addr);
                    }

                    else if(strcmp("get", token) == 0){
						printf("Sto processando la richiesta di download del client collegato alla porta: %d.\n", client_port);
                        token=strtok(NULL,"");
                        cmd_corr(token,socket_fd_child,client_addr);
                        //cmd_send_packets(token,socket_fd_child,client_addr);
                        }
                    	
	                else if(strcmp("put", token) == 0){
						printf("Sto processando la richiesta di upload del client collegato alla porta: %d.\n", client_port);
                        token=strtok(NULL," ");
                        sizep=strtok(NULL,"");
                        cmd_corr_put(sizep,socket_fd_child,client_addr);
                        //cmd_recv_packets(token,socket_fd_child,client_addr);
					}

                    else if(strcmp("exit", token) == 0){
                        printf("Client su porta %d uscito\n\n",client_port);
                        close(socket_fd_child);
                        fflush(stdout);
                        exit(0);
                    }
                    else{
                        printf("Comando del cliente su porta: %d nullo\n\n",client_port);
                    }
                }
            }
        }
    }
    close(socket_fd);
    return 0;
}