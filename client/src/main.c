/* INCLUDES */
    #include "../inc/libraries.h"
    #include"../inc/config.h"
    #include "../inc/func.h"
    #include "../inc/interrupt.h"
    #include "../inc/cmd.h"
    #include "../inc/types.h"

/* DEFINES */

    #define fflush(stdin) while ( getchar() != '\n') { }

/* CLIENT BSS AND DATA */

    // file descriptor for socket
    int socket_fd;
    // server address
    struct sockaddr_in server_addr;
    // server address length
    int len = sizeof(server_addr);



int main(int argc, char **argv){
    

    /* CLIENT STACK */

        // checking values variable 
        int ret;

        // sending/receiving buffer
        char buffer[BUFFER_SIZE];

        // integer response from server variable
        int response;

        // signal management variables
        struct sigaction SIGINT_act;
        sigset_t SIGINT_set;

    /* SIGNAL MANAGEMENT */

        sigfillset(&SIGINT_set);
        SIGINT_act.sa_sigaction = SIGINT_handler;
        SIGINT_act.sa_mask = SIGINT_set;
        SIGINT_act.sa_flags = 0;
        sigaction(SIGINT, &SIGINT_act, NULL);

    /* WELCOMING SOCKET CREATION */

        // socket initializzation
        socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_fd == -1){
            printf("socket() error while creating socket.\n"
                    "Error: %s\n", strerror(errno));
            exit(-1);
        }
	    // server_addr flushing
	    bzero(&server_addr, len);
        // server_addr setting
	    server_addr.sin_family = AF_INET; 
	    server_addr.sin_addr.s_addr = inet_addr(ADDRESS); 
	    server_addr.sin_port = htons(PORT);

    /* SENDING HELLO REQUEST */ retry_send_hello:

        // buffer flushing
        bzero(buffer, BUFFER_SIZE);
        // buffer setting
        strcpy(buffer, "hello");
        // buffer sending
        ret = sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, len);
        // check
        if (ret == -1){
            if (errno == EINTR) goto retry_send_hello;
            else {
                printf("sendto() error while sending 'hello' request.\n"
                        "Client closing...\n");
                close(socket_fd);
                exit(-1);
            }
        }

    // display commands
    display();

    /* GET PERMANENT CONNECTION */ retry_receive_port_number:

        // buffer flushing
        bzero(buffer, BUFFER_SIZE);
        // buffer receiving : receiving new port number
        ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &len);
        // check
        if (ret == -1){
            if (errno == EINTR) goto retry_receive_port_number;
            else {
                printf("recvfrom() error while receiving new port number.\n"
                        "Client closing...\n");
                close(socket_fd);
                exit(-1);
            }
        }
        // response = new port number
        response = atoi(buffer);
        // check
        if (response == SERVICE_UNAVAILABLE){
            printf("Server is out of service...\n"
                    "Closing client...\n");
            close(socket_fd);
            exit(-1);
        }
        // close welcoming connection
        close(socket_fd);
    
    /* PERMANENT SOCKET CREATION */

        // socket initialization
        socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        // check
        if (socket_fd == -1){
            printf("Cannot create the socket.\n");
            exit(-1);
        }

        // server_addr setting
        server_addr.sin_port = htons(response);

    /* CLIENT ROUTINE */

        while (1) {

            // buffer flushing
            bzero(buffer, BUFFER_SIZE);

            // buffer setting from stdin
            scanf("%[^\n]", buffer); fflush(stdin);
            
            /* LIST REQUEST */
                
                // input check
                if (strcmp(buffer, "list") == 0){

                    // send request and print server available files
                    cmd_list(socket_fd, buffer, server_addr, len);

                    // display available commands
                    display();

                }
else
            /* GET REQUEST */

                // input check
                if (strncmp(buffer, "get", 3) == 0){

                }
                
else
            /* PUT REQUEST */

                // input check
                if (strncmp(buffer, "put", 3) == 0){
                

                    cmd_put(socket_fd, buffer, server_addr, len);

                    // display available commands
                    display();
                }
else
            /* EXIT TRANSMIT */

                // input check
                if (strcmp(buffer, "exit") == 0){

                    // buffer sending
                    ret = sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, len);
                    // socket closing
                    close(socket_fd);
                    // check
                    if (ret == -1){
                        printf("sendto() error while sending exit command.\n"
                                "Client closing...\n");
                        close(socket_fd);
                        exit(-1);
                    }

                    exit(0);
                }

        
        }
    
    return 0;
}