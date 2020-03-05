#ifndef _CONFIG_H
#define _CONFIG_H

#define DIRECTORY "./file_client/"

#define ADDRESS "127.0.0.1"
#define PORT 1024

#define TX_WINDOW 8
#define PAYLOAD 1024


#define TIMEOUT_INTERVAL 1000000    // 1000000 microsec = 1 sec

#define SA struct sockaddr

#define MAX_CLIENTS 20
#define BUFFER_SIZE 1088

#define NOT_FOUND 404
#define BAD_REQUEST 400
#define NOT_ACCEPTABLE 406
#define OK 200
#define SERVICE_UNAVAILABLE 503


#endif