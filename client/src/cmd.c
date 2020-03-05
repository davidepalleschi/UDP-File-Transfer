#include "../inc/cmd.h"
#include "../inc/threads.h"

#include <time.h>
#include <sys/time.h>
#include <pthread.h>

int num_pkt;

void cmd_list(int socket_fd, char *buffer, struct sockaddr_in server_addr, int len){

    // checking values variable
    int ret;

    // status code from server
    char str_response[4]; int response;

    // string of server available files
    char file_list[BUFFER_SIZE];

    // buffer sending
    ret = sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, len);
    // check 
    if (ret == -1){
        printf("sendto() error while sending list command.\n");
        close(socket_fd);
        exit(-1);
    }

    // buffer flushing
    bzero(buffer, BUFFER_SIZE);
    // buffer receiving : receiving list of available files on server
    //          |
    //          |----> EXPECTED FORMAT OK:                           "200 file_list"
    //          |----> UNEXPECTED FORMAT BAD REQUEST:                "400"
    //          |----> UNEXPECTED FORMAT SERVICE UNAVAILABLE:        "503"
    ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &len);
    // check
    if (ret == -1){
        printf("recvfrom() error while receiving file list.\n");
        close(socket_fd);
        exit(-1);
    }

    // getting status code from buffer
    strcpy(str_response, strtok(buffer, " "));

    // getting file_list from buffer
    strcpy(file_list, strtok(NULL, "\0"));

    // response = OK || SERVICE_UNAVAILABLE
    response = atoi(buffer);
    if (response == SERVICE_UNAVAILABLE){
        printf("Server has shutdown.\n");
        close(socket_fd);
        exit(-1);
    }else if (response == BAD_REQUEST){
        printf("Unknown command.\n");
        return;
    }

    printf("\nLIST OF SERVER AVAILABLE FILES:\n%s\n\n", file_list);

}

int semaphore;

void cmd_put(int socket_fd, char *buffer, struct sockaddr_in server_addr, int len){

    // checking values variable
    int ret;

    // infos on uploading file
    char filename[128];
    int file_len; char str_file_len[17]; // 10 cifre circa 1 GB
    char path[200] = DIRECTORY;
    int temp_fd;

    // code status from server variables
    char str_response[4]; int response;

    // packets variables
    packet_man **pkts;

    // threads
    pthread_t ack_controller_tid, ack_receiver_tid;

    // semaphore buffer
    struct sembuf oper;

    strtok(buffer, " ");
    // get filename from user input
	strcpy(filename, strtok(NULL, "\0"));
    // get pathname
    printf("§§§§§§§ %s\n", filename);
    snprintf(path, 200, "%s%s", DIRECTORY, filename);
	//strcat(path, filename);
    printf("§§§§§§§ %s\n", filename);

printf("@@@ Path is %s\n", path);

    // open file to get length
    temp_fd = open(path, O_RDONLY, 0666);
    // check
    if (temp_fd == -1){
        printf("open() error while trying to open %s for get its length.\n"
                "Client closing...\n", path);
        close(socket_fd);
        exit(-1);
    }
    // get file length
    file_len = lseek(temp_fd, 0, SEEK_END);
    close(temp_fd);
    // str_file_len flushing
    bzero(str_file_len, sizeof(str_file_len));
    sprintf(str_file_len, "%d", file_len);

    // buffer rebuilt, actual value = "put"
    strcat(buffer, " ");
    // buffer rebuilt, actual value = "put " 
    strcat(buffer, filename);
    // buffer rebuilt, actual value = "put file_name"
    strcat(buffer, " ");
    // buffer rebuilt, actual value = "put file_name "
    strcat(buffer, str_file_len);
    // buffer rebuilt, actual value = "put file_name file_len"

printf("@@@ buffer contains %s: \n", buffer);

    // buffer sending
    ret = sendto(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, len);
    // check
    if (ret == -1){
        printf("sendto() error while sending put command.\n"
                "Client closing...\n");
        close(socket_fd);
        exit(-1);
    }

    // buffer flushing
    bzero(buffer, BUFFER_SIZE);
    // buffer receiving : receiving put permission
    //          |-------------> EXPECTED FORMAT OK:                     "200"
    //          |-------------> UNEXPECTED FORMAT BAD REQUEST:          "400"
    //          |-------------> UNEXPECTED FORMAT NOT ACCEPTABLE:       "406"
    //          |-------------> UNEXPECTED FORMAT SERVICE UNAVAILABLE:  "503"
    ret = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (SA *) &server_addr, &len);
    // check
    if (ret == -1){
        printf("recvfrom() error while receiving put permission.\n"
                "Client closing...\n");
        close(socket_fd);
        exit(-1);
    }

    // status code value
    response = atoi(buffer);
    // checking...
    if (response != OK){
        if (response == BAD_REQUEST){
            printf("Unknown command.\n");
            return;
        }else if (response == NOT_ACCEPTABLE){
            printf("You can not upload your file.\n");
            return;
        }else if (response == SERVICE_UNAVAILABLE){
            printf("Server has shutdown.\n"
                    "Client closing...\n");
            close(socket_fd);
        }
    }

    // calculate number of packets needed
    num_pkt = ceilf((float)file_len / (float)PAYLOAD);
    printf("@@@ file_len = %d\n    payload = %d\n    num_pkt = ceil(%d/%d) = %d\n", file_len, PAYLOAD, file_len, PAYLOAD, num_pkt);

    // open file to copy bytes in payload
    temp_fd = open(path, O_RDONLY, 0666);
    if (temp_fd == -1){
        printf("open error while opening file for packets building.\n");
        printf("Closing client...\n");
        close(socket_fd);
        exit(-1);
    }

    // memory allocation for packets structs
    pkts = (packet_man **) malloc(num_pkt * sizeof(packet_man *));
    if (pkts == NULL){
        printf("malloc error while allocating management structs for packets.\n");
        printf("Client closing.\n");
        close(socket_fd);
        exit(-1);
    }
    for (int i=0; i<num_pkt; i++){
        pkts[i] = (packet_man *) malloc(sizeof(packet_man));
        if (pkts[i] == NULL){
            printf("malloc error while allocating management struct %d.\n", i);
            printf("Client closing.\n");
            close(socket_fd);
            exit(-1);
        }
        // packet struct building
        pkts[i] -> seq = i;
        read(temp_fd, pkts[i] -> payload, PAYLOAD);    
        pkts[i] -> ack = 0;
        pkts[i] -> ack_checked = 0;

        // copying in data field the real pkt to send (seq + payload)
        /*
        bzero(pkt_buffer, BUFFER_SIZE);
        sprintf(pkt_buffer, "%d", pkts[i] -> seq);
        strcat(pkt_buffer, " ");
        strcat(pkt_buffer, pkts[i] -> payload);
        bzero(pkts[i] -> data, BUFFER_SIZE);
        strcpy(pkts[i] -> data, pkt_buffer);
        */

        bzero(pkts[i] -> data, BUFFER_SIZE);
        snprintf(pkts[i] -> data, 64, "%d", pkts[i] -> seq);
        //snprintf((pkts[i] -> data) + 64, PAYLOAD, "%s", pkts[i] -> payload);
        memcpy((pkts[i] -> data) + 64, pkts[i] -> payload, PAYLOAD);
        //write(STDOUT_FILENO, pkts[i] -> payload, BUFFER_SIZE);printf("\n");

//printf("\n\n\n@@@@@@"); fflush(stdout); write(STDOUT_FILENO, pkts[i] -> data, BUFFER_SIZE); printf("\n\n\n");
    }
    // closing I/O temporarily session
    close(temp_fd);

printf("@@@ All packets (%d) have been built and file is closed.\n", num_pkt);

    // initialize semaphore for ack_receiver thread
    semaphore = semget(IPC_PRIVATE, 1, 0666);
    ret = semctl(semaphore, 0, SETVAL, 0);

    // main_thread routin : sending packets
        
        pthread_create(&ack_receiver_tid, NULL, ack_receiver, (void *) pkts);

        // base number of transmission window
        int base = 0;
        int counterACK = 0;

        int window = TX_WINDOW;
        if (num_pkt < window) window = num_pkt;

        struct timeval final;
        long time_stamp;


        //                                   base          (num_pkt-TX_WINDOW)    num_pkt
        while (counterACK < num_pkt){ //      |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|

//printf("@@@ Number of ACK received %d.\n", counterACK);

            for(int i=base; i<base+window; i++){ //   base    i       base+TX_WINDOW
                                                    //    |_|_|_|_|_|_|_|_|_|_|
//if (counterACK < num_pkt){
                
                if (!(pkts[i] -> sent)){
                    sendto(socket_fd, pkts[i] -> data, BUFFER_SIZE, 0, (SA *) &server_addr, len);
                    
                    pkts[i] -> sent = 1;
                    printf("@@@ Pkt %d transmitted.\n", i);

                    // setting pkt starting time    (measure unit is microsec)
                    gettimeofday(&(pkts[i] -> initial), NULL);
                    pkts[i] -> time_start = (((long)((pkts[i] -> initial).tv_sec))*1000000) +                          
                                                (long)((pkts[i] -> initial).tv_usec);
                     
                    oper.sem_num = 0;
                    oper.sem_op = 1;
                    oper.sem_flg = 0;
semaphore_commit:
                    ret = semop(semaphore, &oper, 1);
                    if ( ret == -1 ){
                        if (errno == EINTR) goto semaphore_commit;
                        else{
                            printf("semop() error in semaphore operations.\n");
                            printf("Client closing...\n");
                            close(socket_fd);
                            exit(-1);
                        }
                    }

                }

                // if main_thread receives ACK of base window packet then transmission window will slide
                if (pkts[i] -> ack){
                    if (!(pkts[i] -> ack_checked)){
                        printf("@@@ ACK %d checked.\n", i);
                        pkts[i] -> ack_checked ++;
                        counterACK ++;
                    }
                    if ((pkts[i] -> seq == base) && (base < num_pkt - window)){
                        base ++;
                    }
                } else {// timemout interval check

                    //actual timestamp
                    gettimeofday(&final, NULL);
                    time_stamp = ((long)(final.tv_sec) * 1000000) + (long)(final.tv_usec);
 
                  //if (initial timestamp) + timeout > (actual timestamp)
                    long diff = time_stamp - (pkts[i] -> time_start);
                /*    printf("\nPkt %d\n"
                            "tempo di invio: %ld\ntempo attuale: %ld\n"
                            "tempo trascorso: %ld\ntimeout interval: %ld\n", 
                            i, pkts[i] -> time_start, time_stamp, diff, (long) TIMEOUT_INTERVAL);*/
                    if (diff > (long) TIMEOUT_INTERVAL){  

                        // RETRANSMISSION 
                        sendto(socket_fd, pkts[i] -> data, BUFFER_SIZE, 0, (SA *) &server_addr, len);
                        printf("@@@ Pkt %d retransmitted.\n", i);
                        
                        
                        // setting pkt starting time    (measure unit is microsec)
                        gettimeofday(&(pkts[i] -> initial), NULL);
                        pkts[i] -> time_start = (((double)((pkts[i] -> initial).tv_sec))*1000000) +                          
                                                (double)((pkts[i] -> initial).tv_usec);

                    }
                 
                }
                
                /* PER PROVARE SENZA SERVER FACCIO FINTA CHE CON PROBABILITA 20% NON RICEVO L'ACK
                if (!(pkts[i] -> ack) && (prob_perdita(20))){
                    printf("@@@ ACK %d received.\n", i);
                    pkts[i] -> ack = 1;
                }
                */
//} 
            }            
        }
    
}








/*
void cmd_corr(char * file_name, int socket_fd, struct sockaddr_in client_addr){
    int len=sizeof(client_addr);
    sendto(socket_fd, ispresent(file_name), 50 , 0, (SA *) &client_addr, len);
}

void cmd_corr_put(char* file_size,int socket_fd, struct sockaddr_in client_addr){
    int len=sizeof(client_addr);
    if (atoi(file_size)>20000000){
        sendto(socket_fd, "406", 3 , 0, (SA *) &client_addr, len);
    }
    else{
        sendto(socket_fd, "200", 3 , 0, (SA *) &client_addr, len);
    }
}

void cmd_recv_packets(char* file_name,int socket_fd, struct sockaddr_in client_addr){
    int len=sizeof(client_addr);
    
}
*/