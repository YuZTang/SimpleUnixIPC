#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

#define MAX_CLIENT_SUPPORTED    32

/*An array of File descriptors which the server process
 * is maintaining in order to talk with the connected clients.
 * Master skt FD is also a member of this array*/
int monitored_fd_set[MAX_CLIENT_SUPPORTED];

/*Each connected client's intermediate result is
 * maintained in this client array.*/
int client_result[MAX_CLIENT_SUPPORTED] = {0};


/*Remove all the FDs, if any, from the the array*/
static void
intitiaze_monitor_fd_set(){
    for (int i = 0; i<MAX_CLIENT_SUPPORTED ; i++){
        monitored_fd_set[i]=-1;
    }
}

/*Add a new FD to the monitored_fd_set array*/
static void
add_to_monitored_fd_set(int skt_fd){
    for(int i=0; i<MAX_CLIENT_SUPPORTED ;i++){
        if(monitored_fd_set[i]!=-1){
            continue;
        }
        monitored_fd_set[i]=skt_fd;
        break;
    }

}

/*Remove the FD from monitored_fd_set array*/
static void
remove_from_monitored_fd_set(int skt_fd){
    for (int i=0 ; i<MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i]!=skt_fd){
            continue;
        }
        monitored_fd_set[i]=-1;
        break;
    }

}

/* Clone all the FDs in monitored_fd_set array into
 * fd_set Data structure*/
static void
refresh_fd_set(fd_set *fd_set_ptr){
    FD_ZERO(fd_set_ptr);
    for (int i=0; i<MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i]!=-1){
            FD_SET(monitored_fd_set[i],fd_set_ptr);
        }
    }
}

/*Get the numerical max value among all FDs which server
 * is monitoring*/

static int
get_max_fd(){
    int max = -1;
    for (int i=0; i<MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i]>max){
            max = monitored_fd_set[i];
        }
    }
    return max;
}




int main() {
    struct sockaddr_un name;
    int ret;
    int connection_socket;
    int data_socket;
    int result;
    int data;
    char buffer[BUFFER_SIZE];

    fd_set readfds;
    int comm_socket_fd,i;
    intitiaze_monitor_fd_set();
    add_to_monitored_fd_set(0);


    //In case the other programs exited inadvertently on the last run, remove the socket
    unlink(SOCKET_NAME);

    //Create the master socket file descriptor
    connection_socket = socket(AF_UNIX,SOCK_STREAM,0);
    if (connection_socket==-1){
        perror("Master socket file descriptor");
        exit(EXIT_FAILURE);
    }
    printf("Master socket file descriptor created successfully\n");


    //Initialize the sockaddr_un variable
    memset(&name,0,sizeof(struct sockaddr_un) );
    name.sun_family=AF_UNIX;
    strncpy(name.sun_path,SOCKET_NAME, sizeof(name.sun_path)-1 );

    //Binding the socket to the address(addressing)
    ret = bind(connection_socket, (const struct sockaddr*)&name, sizeof(struct sockaddr_un));
    if (ret == -1){
        perror("Binding");
        exit(EXIT_FAILURE);
    }
    printf("Binding (Addressing) successfully\n");


    // Set the socket to listening status with queueing up to 20 clients
    ret = listen(connection_socket,20);
    if (ret == -1){
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    printf("Listening successfully!\n");


    // Main loop that server handling connections
    while(1){

        refresh_fd_set(&readfds);

        printf("Waiting on select() system calls\n");

        select(get_max_fd()+1,&readfds, NULL, NULL, NULL);


        if(FD_ISSET(connection_socket,&readfds)){

            printf("New connection recieved recvd, accept the connection\n");
            data_socket = accept(connection_socket,NULL,NULL);

            if (data_socket == -1){
                perror("Accept system call");
                exit(EXIT_FAILURE);
            }
            printf("Connection accepted from clients\n");
            add_to_monitored_fd_set(data_socket);
        }

        else if(FD_ISSET(0,&readfds)){
            memset(buffer,0,BUFFER_SIZE);
            ret = read(0,buffer,BUFFER_SIZE);
            printf("Input read from console : %s\n", buffer);
        }
        else{
            comm_socket_fd = -1;
            for(i=0 ;i<MAX_CLIENT_SUPPORTED ;i++){
                if(FD_ISSET(monitored_fd_set[i],&readfds)){
                    comm_socket_fd = monitored_fd_set[i];

                    /*Prepare the buffer to recv the data*/
                    memset(buffer, 0, BUFFER_SIZE);

                    /* Wait for next data packet. */
                    /*Server is blocked here. Waiting for the data to arrive from client
                     * 'read' is a blocking system call*/
                    printf("Waiting for data from the client\n");
                    ret = read(comm_socket_fd, buffer, BUFFER_SIZE);
                    if(ret == -1){
                        perror("Read");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(&data, buffer, sizeof(int));
                    if (data == 0) {
                        memset(buffer,0,BUFFER_SIZE);
                        sprintf(buffer, "Result = %d", client_result[i]);

                        printf("sending final result back to client\n");
                        ret = write(comm_socket_fd, buffer, BUFFER_SIZE);
                        if (ret == -1){
                            perror("Write");
                            exit(EXIT_FAILURE);
                        }
                        /* Close socket. */
                        close(comm_socket_fd);
                        client_result[i] = 0;
                        remove_from_monitored_fd_set(comm_socket_fd);
                        continue; /*go to select() and block*/
                    }
                    client_result[i]+=data;

                }
            }

        }

    }

    // close the master socket FD
    close(connection_socket);
    remove_from_monitored_fd_set(connection_socket);
    printf("Connection closed..\n");

    //server should release the sources before getting terminated.
    /*unlink the socket*/

    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);

}
