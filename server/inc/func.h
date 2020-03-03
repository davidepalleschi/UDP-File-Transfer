#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include <signal.h>
#include "types.h"
#include<netinet/in.h>

void interrupt_handler(int, siginfo_t *, void *);
void child_death_handler(int, siginfo_t* , void* );

int create_socket(int s_port);
char* dirfile();
char* ispresent(char* file_name);

void cmd_list(int socket_fd, struct sockaddr_in client_addr);
void cmd_corr(char * file_name, int socket_fd, struct sockaddr_in client_addr);
void cmd_corr_put(char* file_size,int socket_fd, struct sockaddr_in client_addr);
//void cmd_send_packets(char* file_name,int socket_fd, struct sockaddr_in client_addr);

#endif