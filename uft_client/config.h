#ifndef _CONFIG_H
#define _CONFIG_H

#define PORT 1024 
#define ADDRESS "127.0.0.1"
#define SA struct sockaddr

#define BUFFER_SIZE 50
#define NAME_LEN 128

#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define NOT_ACCEPTABLE 406
#define SERVICE_UNAVAILABLE 503


#define PAYLOAD 1024
#define TX_WINDOW 3
#define TIMER 300000
#define SENDING_TIMER 100000
#define MIN_SENDING_TIMER 0.005
#define LOSS_PROBABILITY 0.15
#define ALPHA 0.125
#define BETA 0.25

#endif