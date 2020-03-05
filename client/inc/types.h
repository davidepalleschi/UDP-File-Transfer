#ifndef _TYPES_H
#define _TYPES_H

#include "config.h"

typedef struct _packet_man{
	// sequence number of packet
	int seq;
	
	char payload[PAYLOAD];

	// buffer that contains seq + payload
	char data[BUFFER_SIZE];

	// boolean [to know if packet has been sent]
	int sent;

	// boolean [to know if packet has been received]
	int ack;

	// boolean [to know if ack has been checked]
	int ack_checked;

	//timestamp
	struct timeval initial;
	long time_start;

} packet_man;


#endif