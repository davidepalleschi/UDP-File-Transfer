#ifndef _TYPES_H
#define _TYPES_H


typedef struct _packet_form{
	int counter;
	char buf[BUFFER_SIZE];
	int ack;
} packet_form;

#endif