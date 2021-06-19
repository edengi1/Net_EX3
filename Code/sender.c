#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 

#define SERVER_PORT          5060
#define SERVER_IP_ADDRESS    "127.0.0.1"
#define FILENAME             "data.txt" 
#define FULL_SIZE 1048576
#define BUFF_SIZE 1500

int main() {

    /* INIT FIELDS
    * --> file_pointer == a pointer to the given file .
    * --> server_address == a structure describing an internet socket address.
    */    
    char buffer[BUFF_SIZE];
    int file_size;
    FILE *file_pointer;
    socklen_t length;

    int number_of_runs = 0;
    int i = 0;
    while(i < 2) {

        int j = 0;
        while(j < 5) {
            /* CREATE CLIENT SOCKET 
            * --> SOCK_STREAM == a TCP protocol type.
            */
            int client_socket = socket(AF_INET, SOCK_STREAM, 0);
            if(client_socket == -1) {
                fprintf(stderr, "Couldn't create the socket : %s\n", strerror(errno));
                exit(EXIT_FAILURE); // failing exit status.
            }

            /* CONGESTION CONTROL PROGRAM
            * --> getsockopt == function manipulates options associated with a socket.
            * --> setsockopt == function shall set the option specified by the option_name argument.
            * --> IPPROTO_TCP == to indicate that an option is interpreted by the TCP.
            * --> TCP_CONGESTION == Congestion control algorithm.
            */
            int get_sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, &length);
            if( get_sock_opt != 0) {
                perror("getsockopt");
                exit(EXIT_FAILURE); // failing exit status.
            }
            if(i == 0) {
                strcpy(buffer,"cubic");
            } else {
                strcpy(buffer,"reno");
            }
            length = sizeof(buffer);
            int set_sock_opt = setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, length);
            if(set_sock_opt !=0 ) {
                perror("setsockopt");
                exit(EXIT_FAILURE); // failing exit status.
            }
            get_sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, &length);
            if( get_sock_opt != 0) {
                perror("getsockopt");
                exit(EXIT_FAILURE); // failing exit status.
            }
            number_of_runs++;
            printf("\n======= (%d) Current CC: %s  ====== \n",number_of_runs, buffer);
            /* construct the server_address struct
            * --> memset == zeroing the server_address struct.
            * --> AF_INET == IPv4 type.
            * --> htons == short, network byte order converter.
            * --> inet_pton == convert the IP address from String type.
            */
            struct sockaddr_in server_address;
            memset(&server_address, 0, sizeof(server_address));
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(SERVER_PORT);
            int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &server_address.sin_addr);
            if(rval <= 0) {
                printf("inet_pton() failed");
                return -1;
            }

            /* CONNECT TO THE SERVER
            * --> make a connection to the server with client_socket.
            */
            int connection = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
            if(connection == -1) {
                fprintf(stderr, "connect() failed with error code --> %s\n", strerror(errno));
                exit(EXIT_FAILURE); // failing exit status.
            } 
            else {
                printf("connected to server!\n");
            }

            /* SEND DATA TO SERVER */
            int numbytes = recv(client_socket, buffer, BUFF_SIZE, 0);
        	if (numbytes == -1) {
            		perror("recv");
            		exit(1);
		    }	

	        buffer[numbytes] = '\0';

        	printf("Received from server: '%s' \n", buffer);

            file_pointer = fopen(FILENAME, "r");
            if(file_pointer == NULL) {
                fprintf(stderr, "Failed to open file 1mb.txt : %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            int data_stream;
            int size = 0;
            while( ( data_stream = fread(buffer,1,sizeof(buffer),file_pointer) ) > 0 ) {
                size += send(client_socket, buffer, data_stream, 0);
            }

            if(size == FULL_SIZE) {
                printf("successfully sent 1MB file: %d\n",size);
            }else {
                printf("sadly sent just %d out of %d\n",size,FULL_SIZE);
            }
            sleep(1);
            close(client_socket);
            j++;
        }
        i++;
    }
    return 0;
}
