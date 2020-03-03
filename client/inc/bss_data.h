#ifndef _BSS_DATA_H
#define _BSS_DATA_H

#include"config.h"
#include<pthread.h>

char cmd[BUFFER_SIZE];
char buffer[BUFFER_SIZE];
char **file_buffer;
char *file_list_buffer;
int filename_len;
char filename[NAME_LEN];
int num_pkt;
int socket_fd, msg_rec, len; 
int server_addr_len;
struct sockaddr_in server_addr;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int adaptive = 0;
int base = 0; //window base index

double estimateRTT;
double devRTT;
double sending_timeout;

#endif