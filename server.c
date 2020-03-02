/*
 * (1) Memoria condivisa con i processi figli
 * (2) Gestione segnali
 *      - SIGCHLD per la terminazione di un processo figlio
 *      - SIGUSR1 provola il decremento di num_client
 * (3) Creazione processo per richiesta di terminazione server
 * 
 *  
*/

#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/time.h>
#include<signal.h>
#include<math.h>
#include<sys/mman.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<pthread.h>
#include<errno.h>
#include<dirent.h>

/* defines */
#define DIRECTORY "./file_server/"
#define PORT 1024
#define SA struct sockaddr
#define MAX_CLIENTS 20
#define BUFFER_SIZE 50
#define NOT_FOUND 404
#define BAD_REQUEST 400
#define NOT_ACCEPTABLE 406
#define OK 200
#define SERVICE_UNAVAILABLE 503


/* nuovi tipi */


/* variabili globali */
int socket_fd;
int num_client = 0;

/* dichiarazione funzioni */
void interrupt_handler(int, siginfo_t *, void *);
void child_death_handler(int, siginfo_t* , void* );

int create_socket(int s_port);
char* dirfile();
char* ispresent(char* file_name);

void cmd_list(int socket_fd, struct sockaddr_in client_addr);
void cmd_corr(char * file_name, int socket_fd, struct sockaddr_in client_addr);
void cmd_corr_put(char* file_size,int socket_fd, struct sockaddr_in client_addr);
//void cmd_send_packets(char* file_name,int socket_fd, struct sockaddr_in client_addr);





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
                
                
                while (1)
                {
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

                    if(strcmp("get", token) == 0){
						printf("Sto processando la richiesta di download del client collegato alla porta: %d.\n", client_port);
                        token=strtok(NULL,"");
                        cmd_corr(token,socket_fd_child,client_addr);
                        //cmd_send_packets(token,socket_fd_child,client_addr);
                        }
                    	
	                if(strcmp("put", token) == 0){
						printf("Sto processando la richiesta di upload del client collegato alla porta: %d.\n", client_port);
                        token=strtok(NULL," ");
                        sizep=strtok(NULL,"");
                        cmd_corr_put(sizep,socket_fd_child,client_addr);
                        //cmd_recv_packets(token,socket_fd_child,client_addr);
					}

                    if(strcmp("exit", token) == 0){
                        printf("Client su porta %d uscito\n\n",client_port);
                        close(socket_fd_child);
                        fflush(stdout);
                        exit(0);
                    }
                }
                

            }


        }

    }
    close(socket_fd);
    
    

    return 0;
}

void interrupt_handler(int signo, siginfo_t *a, void *b){

    close(socket_fd);
    exit(0);

}

void child_death_handler(int signo, siginfo_t *a, void *b){
    num_client--;
}

int create_socket(int s_port){
    struct sockaddr_in server_addr;
    //creazione socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd == -1){
        printf("\nSocket %d creation failed.\n",s_port);
    }else {
        printf("\nSocket %d created succesfully.\n",s_port);
    }
    //flush della memoria per la struttura della socket
    bzero(&server_addr, sizeof(server_addr));

    //assegnazione di indirizzo IP e porta della socket lato server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(s_port);

    //binding
    if (bind(socket_fd, (SA *) (&server_addr), sizeof(server_addr)) == -1){
        printf("Binding failed.\n");
    }else{
        printf("Binding success.√\n");
    }
    return socket_fd;
}

char* dirfile(){
    char buffer[50];
    bzero(buffer,sizeof(buffer));
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (DIRECTORY)) != NULL) {
    while ((ent = readdir (dir)) != NULL) {
        if(strcmp(ent->d_name,".") != 0 + strcmp(ent->d_name,"..") != 0 == 0){
            strcat(buffer,ent->d_name);
            strcat(buffer,"\n");
        }
    }
    return strcat(buffer,"");
    closedir (dir);
}   else {
    /* could not open directory */
    return strcat(buffer,"ERROR");
}
}


char* ispresent(char* file_name){
    char file[30]= DIRECTORY;
    char ok[50]="200 ";
    char sizestr[25];

    strcat(file,file_name);

    int fd=open(file,O_RDONLY, 0666);
    int size=lseek(fd,0,SEEK_END);
    
    bzero(sizestr,sizeof(sizestr));
    sprintf(sizestr,"%d",size);
    if(fd==-1){
        return "404";
    }
    else{
        return strcat(ok  , sizestr);
    }
    close(fd);
}

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
