//
// Created by TANGSMALL on 2022/5/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>


#define SOCKET_NAME "tmp/DemoSocket"
#define BUFFER_SIZE 128


int main(){
    int data_socket;
    int data;
    int ret;
    char buffer[BUFFER_SIZE];
    struct sockaddr_un addr;


    /*create a data socket*/
    data_socket = socket(AF_UNIX,SOCK_STREAM,0);
    if (data_socket == -1){
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    /*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure.
     * */
    memset(&addr, 0 , sizeof(struct sockaddr_un));

    /* connect socket to socket address*/
    addr.sun_family=AF_UNIX;
    strncpy(addr.sun_path,SOCKET_NAME, sizeof(addr.sun_path)-1);

    ret = connect(data_socket,(struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (ret == -1){
        fprintf(stderr, "The server is down.\n");
        exit(EXIT_FAILURE);
    }

    /* Send arguments*/
    do{
        printf("Please Enter a number to send to the server: ");
        scanf("%d", &data);
        ret = write(data_socket,&data, sizeof(int));
        if(ret == -1){
            perror("Write");
            break;
        }
        printf("No of bytes sent = %d, data sent = %d\n", ret, data);
    }while(data);

    /*request results*/
//    memset(buffer, 0, BUFFER_SIZE);
//    strncpy (buffer, "RES", strlen("RES"));
//    buffer[strlen(buffer)] = '\0';
//    printf("buffer = %s\n", buffer);
//
//    ret = write(data_socket, buffer, strlen(buffer));
//    if (ret == -1) {
//        perror("write");
//        exit(EXIT_FAILURE);
//    }


    /* Receive results*/
    memset(buffer,0,BUFFER_SIZE);
    ret = read(data_socket,buffer, BUFFER_SIZE);
    if (ret == -1){
        perror("Read");
        exit(EXIT_FAILURE);
    }
    
    /* Ensure buffer is 0-terminated. */

    buffer[BUFFER_SIZE - 1] = 0;
    printf("Result = %s\n", buffer);

    /* Close socket. */

    close(data_socket);
    exit(EXIT_SUCCESS);

}
