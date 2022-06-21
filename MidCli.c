#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>

#define PORT 8784
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    struct sockaddr_in saddr;
    struct hostent *h; 
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error creating socket\n");
        return 1;
    }

    if ((h = gethostbyname("127.0.0.1")) == NULL){
        printf("Unknown host\n");
        return 1;
    }
    else {
        printf("Domain: 127.0.0.1\n");
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    memcpy((char *)&saddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    saddr.sin_port = htons(PORT);

    if(connect(sockfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0){
        printf("Connect failed\n");
        return 1;
    }

    printf("Connected\n");
    
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) < 0) {
        perror("Set socket opt failed\n");
        close(sockfd);
        exit(-1);
    }

    struct pollfd pollfd;
    pollfd.fd = fileno(stdin);
    pollfd.events = POLLIN;

    int fl = fcntl(sockfd, F_GETFL, 0);
    if (fcntl(sockfd, F_SETFL,fl | O_NONBLOCK) < 0){
        printf("Error fcntl");
        return 1;
    }

    char sendbuffer[BUFFER_SIZE];
    char rcvbuffer[BUFFER_SIZE];

    bool condition = true;
    while (condition){
        if (recv(sockfd, rcvbuffer, sizeof(rcvbuffer),0) > 0) {
            char tmp[BUFFER_SIZE];
            int i = 0;
            do {
                tmp[i] = rcvbuffer[i];
            } while (rcvbuffer[i+1] != '\0');
            printf("\nFrom server: %s", tmp);
            bzero(sendbuffer, sizeof(sendbuffer));
        }

        if (poll(&pollfd,1,0) > 0){
            if (pollfd.revents && POLLIN) {
                printf("Enter message: ");
                fgets(sendbuffer, sizeof(sendbuffer), stdin);
                if (strncmp(sendbuffer,"/quit\n", 6) == 0){
                    printf("Disconnected\n");
                    close(sockfd);
                    break;
                }
                send(sockfd, sendbuffer, strlen(sendbuffer),0);
            }
            bzero(sendbuffer, sizeof(sendbuffer));
        }
    }


}