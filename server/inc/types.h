#ifndef _TYPES_H
#define _TYPES_H

#include "config.h"

typedef struct _packet_form{
	int counter;
	char payload[PAYLOAD];
	int ack;
} packet_form;


#endif