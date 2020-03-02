#ifndef _FOOS_H
#define _FOOS_H
#include <signal.h>
#include "types.h"

void SIGINT_handler(int, siginfo_t *, void *);
void receive_packets(void);
int send_ack(int seq);
int file_to_send(void);
int send_pkt(packet_form *file_form, int seq, int size);
void set_adpt_timeout(double time);
void get_adpt_timeout(double start, double end);
void check_pkt(packet_form *, int offset, int seq);

#endif