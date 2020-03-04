#ifndef FUNC_H
#define FUNC_H

#include "../inc/libraries.h"
#include"../inc/config.h"


int create_socket(int s_port, struct sockaddr_in server_addr);
char* ispresent(char* file_name);
void display();
int prob_perdita( int prob);


#endif