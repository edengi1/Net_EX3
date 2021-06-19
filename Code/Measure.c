#include <stdio.h>
#include <unistd.h> // for sleep()
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>    // for clock_t, clock()
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define SERVER_PORT 5060  //The port that the server listens.
#define BILLION  1000000000.0
#define BUFF_SIZE 1500

int main() {
    // on linux to prevent crash on closing socket.
    signal(SIGPIPE, SIG_IGN);
    char buffer[BUFF_SIZE];

    // create a socket lisener.
    int socket_listener = -1;
    if((socket_listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Couldn't create a socket listener : %d",errno);
    }

	// setsockopt = used to control the options of this socket.
    // Reuse the port if the server socket on was closed
	// and remains for 45 seconds in TIME-WAIT state till the final removal.
    int enable_reuse = 1;
    if(setsockopt(socket_listener, SOL_SOCKET, SO_REUSEADDR,&enable_reuse, sizeof(int)) < 0) {
        printf("setsockopt() failed with error code: %d", errno);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);  //network order

    // connect the server to a port which can read and write on.
    if(bind(socket_listener, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        printf("Bind failed with error code : %d" , errno);
        return -1;
    }

    printf("Bind() success!\n\n");

    // Make the socket listening; actually mother of all client sockets.
    // 500 is a Maximum size of queue connection requests
    // number of concurrent connections 
    if(listen(socket_listener, 500) == -1) {
        printf("listen() failed with error code : %d",errno);
        return -1;
    }

    //Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");

    struct sockaddr_in client_address; 
    socklen_t client_address_length = sizeof(client_address);
    int i = 0;

    while(i < 2){
        int j = 0;
        double sum_for_average = 0.0;
        while(j < 5) {
            memset(&client_address, 0, sizeof(client_address));

            // updates the length in each iteration.
            client_address_length = sizeof(client_address);

            // accept() gets the connection and return the socket of this connection.
            int client_socket = accept(socket_listener, (struct sockaddr *)&client_address, &client_address_length);
            if(client_socket == -1) {
            printf("listen failed with error code : %d",errno);
            close(socket_listener);
            return -1;
            } else {
                printf("A new client connection accepted\n");
            }

            //Reply to client
            char message[] = "welcome";
            int messageLen = strlen(message) + 1;
            // this method returns the size of bytes that succesfully sent.
            int bytes_sent = send(client_socket, message, messageLen, 0);
            if(bytes_sent == -1) {
                printf("send() failed with error code : %d", errno); 
            }
            else if(bytes_sent == 0) {
                printf("peer has closed the TCP connection prior to send().\n");
            }
            else if (messageLen > bytes_sent) {
            printf("sent only %d bytes from the required %d.\n", messageLen, bytes_sent);
            }
            else {
            printf("'welcome' successfully sent.\n");            
            }

            struct timespec start, end;
            clock_gettime(CLOCK_REALTIME, &start);

            int bytes = -1;
            while(bytes != 0) {
                bytes = recv(client_socket,buffer, BUFF_SIZE,0);
            }
            clock_gettime(CLOCK_REALTIME, &end);
            // time_spent = end - start
            double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
            printf("Time elpased is %f seconds\n\n", time_spent);
            sum_for_average += time_spent;
            sleep(1);
            j++;
        }
        char cc_type[20];
        if(i == 0) {
            strcpy(cc_type,"cubic");
        } else {
            strcpy(cc_type,"reno");
        }
        printf("\nAverage time for CC %s is %f .\n\n",cc_type,sum_for_average/5);
        i++;
    }
    
    close(socket_listener);
    return 0;
}
