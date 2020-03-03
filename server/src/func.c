#include "../inc/func.h"
#include "../inc/libraries.h"
#include"../inc/config.h"

extern int socket_fd;
extern int num_client;




void interrupt_handler(int signo, siginfo_t *a, void *b){

    close(socket_fd);
    exit(0);

}

void child_death_handler(int signo, siginfo_t *a, void *b){
    num_client--;
}

int create_socket(int s_port){
    struct sockaddr_in server_addr;
    //creazione socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd == -1){
        printf("\nSocket %d creation failed.\n",s_port);
    }else {
        printf("\nSocket %d created succesfully.\n",s_port);
    }
    //flush della memoria per la struttura della socket
    bzero(&server_addr, sizeof(server_addr));

    //assegnazione di indirizzo IP e porta della socket lato server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(s_port);

    //binding
    if (bind(socket_fd, (SA *) (&server_addr), sizeof(server_addr)) == -1){
        printf("Binding failed.\n");
    }else{
        printf("Binding success.√\n");
    }
    return socket_fd;
}

char* dirfile(){
    char buffer[50];
    bzero(buffer,sizeof(buffer));
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (DIRECTORY)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(strcmp(ent->d_name,".") != 0 + strcmp(ent->d_name,"..") != 0 == 0){
                strcat(buffer,ent->d_name);
                strcat(buffer,"\n");
            }
        }
        return strcat(buffer,"");
        closedir (dir);
        }   
    else {
    /* could not open directory */
        return strcat(buffer,"406");
    }
}


char* ispresent(char* file_name){
    char file[30]= DIRECTORY;
    char ok[50]="200 ";
    char sizestr[25];

    strcat(file,file_name);

    int fd=open(file,O_RDONLY, 0666);
    int size=lseek(fd,0,SEEK_END);
    
    bzero(sizestr,sizeof(sizestr));
    sprintf(sizestr,"%d",size);
    if(fd==-1){
        return "404";
    }
    else{
        return strcat(ok  , sizestr);
    }
    close(fd);
}

void cmd_list(int socket_fd, struct sockaddr_in client_addr){
    char buffer[50];
    int len=sizeof(client_addr);
    bzero(buffer,BUFFER_SIZE);
    sendto(socket_fd, dirfile() , BUFFER_SIZE , 0, (SA *) &client_addr, len);
}

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
