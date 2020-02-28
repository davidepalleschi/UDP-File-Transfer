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

/* defines */
#define PORT 8080
#define SA struct sockaddr
#define MAX_CLIENTS 3
#define BUFFER_SIZE 50
#define NOT_FOUND 404
#define BAD_REQUEST 400
#define NOT_ACCEPTABLE 406
#define OK 200
#define SERVICE_UNAVAILABLE 503


/* nuovi tipi */


/* variabili globali */
int socket_fd;

/* dichiarazione funzioni */
void interrupt_handler(int, siginfo_t *, void *);


int main(int argc, char **argv){

    /* MAIN PROCESS SERVER STACK */

    pid_t child_pids[MAX_CLIENTS];
    pid_t parent_pid = getpid();
    struct sockaddr_in server_addr, client_addr;
    int msgrcv, len;
    char buffer[BUFFER_SIZE];
    struct sigaction act;
    sigset_t set;
    int num_client = 0;

    /* gestione segnali */

    sigfillset(&set);
    act.sa_sigaction = interrupt_handler;
    act.sa_mask = set;
    act.sa_flags = 0;

    /* Inizializzazione socket */

    //creazione socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd == -1){
        printf("Socket creation failed.\n");
    }else {
        printf("Socket created succesfully.\n");
    }
    //flush della memoria per la struttura della socket
    bzero(&server_addr, sizeof(server_addr));

    //assegnazione di indirizzo IP e porta della socket lato server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    //binding
    if (bind(socket_fd, (SA *) (&server_addr), sizeof(server_addr)) == -1){
        printf("Binding failed.\n");
    }else{
        printf("Binding success.√\n");
    }

    while (1) {
        //preparo il buffer del messaggio da ricevere
        bzero(buffer, BUFFER_SIZE);
                
        //preparo la ricezione dell'indirizzo del client
        bzero(&client_addr, sizeof(client_addr));
        len = sizeof(client_addr);

        //ricevo il messaggio dal client
        msgrcv = recvfrom(socket_fd, (char *) buffer, sizeof(buffer), 0, (SA *) &client_addr, &len);
        if (msgrcv == -1){
            printf("rcvfrom() error .\n");
            bzero(buffer, BUFFER_SIZE);
        }

        //ciò che manda il client
        //printf("%s\n", buffer);

printf("number of clients = %d", num_client);
        if(num_client >= MAX_CLIENTS){
            num_client --;
            printf("Numero limite di clients superato.\n");
            bzero(buffer, BUFFER_SIZE);
            sprintf(buffer, "%d", SERVICE_UNAVAILABLE);
            if (sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &client_addr, len) == -1)
                printf("sendto() error %d: %s", SERVICE_UNAVAILABLE, strerror(errno));
        }

    }
    close(socket_fd);
    
    

    return 0;
}

void interrupt_handler(int signo, siginfo_t *a, void *b){

    close(socket_fd);
    exit(0);

}