#ifndef _TYPES_H
#define _TYPES_H

#include "config.h"

typedef struct _packet_form{
	int seq;

	char payload[PAYLOAD];

	int recv;

	int checked;

	int ack;
} packet_form;


#endif