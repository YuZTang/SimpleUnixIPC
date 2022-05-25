#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#define SOCKET_NAME "tmp/DemoSocket"
#define BUFFER_SIZE 128

int main() {
    struct sockaddr_un name;
    int ret;
    int connection_socket;
    int data_socket;
    int result;
    int data;
    char buffer[BUFFER_SIZE];


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
        printf("Waiting on accept() system calls");
        data_socket = accept(connection_socket,NULL,NULL);

        if (data_socket == -1){
            perror("Accept system call");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from clients\n");

        result = 0;
        // Wait for the client to send data
        while(1){
            // Initialize the buffer for receiving data, prepare the buffer
            memset(buffer,0,BUFFER_SIZE);

            //Wait for the next data packet
            //Server is blocked here, waiting for the data to arrive from the client
            printf("Waiting for data from the client...\n");
            ret = read(data_socket, buffer, BUFFER_SIZE);
            if(ret == -1){
                perror("Read");
                exit(EXIT_FAILURE);
            }

            memcpy(&data, buffer, sizeof(int));
            if (data == 0){
                break;
            }
            result+=data;
        }

        //Send result
        memset(buffer,0,BUFFER_SIZE);
        sprintf(buffer,"Result = %d", result);

        printf("Sending the final result to the client");
        ret = write(data_socket,buffer,BUFFER_SIZE);
        if(ret == -1){
            perror("Write");
            exit(EXIT_FAILURE);
        }

        //close data socket
        close(data_socket);
    }
    // close the master socket FD
    close(connection_socket);
    printf("Connection closed..\n");

    //server should release the sources before getting terminated.
    /*unlink the socket*/

    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);

}
