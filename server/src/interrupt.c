#include "../inc/interrupt.h"

extern int socket_fd;
extern int num_client;

void interrupt_handler(int signo, siginfo_t *a, void *b){

    close(socket_fd);
    exit(0);

}

void child_death_handler(int signo, siginfo_t *a, void *b){
    num_client--;
}
