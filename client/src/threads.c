#include "../inc/threads.h"
#include "../inc/libraries.h"
#include "../inc/types.h"
#include <semaphore.h>

extern int semaphore;


void *ack_receiver(void *arg){
    
    /* THREAD STACK */

        int ret;
        int len = sizeof(server_addr);

        struct sembuf oper;

        int counter = 0;

        char buffer[BUFFER_SIZE];
        int seq;

        packet_man **pkts = (packet_man **) arg;

    /* HAZARD SECTION */

        while (1){

            oper.sem_num = 0;
            oper.sem_op = -1;
            oper.sem_flg = 0;
semaphore_thread_commit:
            ret = semop(semaphore, &oper, 1);
            if (ret == -1){
                if (errno == EINTR) goto semaphore_thread_commit;
                else{
                    printf("semop() error in thread semaphore operations.\n");
                    printf("Client closing...\n");
                    close(socket_fd);
                    exit(-1);
                }
            }

            ret == recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &len);   
            if (ret == -1){
                printf("recvfrom() error while receiving packet from server.\n");
                printf("Client closing...\n");
                close(socket_fd);
                exit(-1);
            }

            seq = atoi(buffer);

            pkts[seq] -> ack ++; 
            counter ++;
            if (counter == num_pkt) pthread_exit(0);


        }



}