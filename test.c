#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<netdb.h>
#include <fcntl.h> 
#include <unistd.h>
#include <poll.h>

int main(){

    setbuf(stdout, NULL);

    int sockfd, clen, clientfd;
    struct sockaddr_in saddr, caddr;
    unsigned short port = 8784;
    char clientms[1000], svms[1000];

    if ((sockfd =  socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error\n");
        return 1;
    }

    setsockopt(sockfd, SOL_SOCKET,
               SO_REUSEADDR, &(int){1},
               sizeof(int));

    int sockfl = fcntl(sockfd, F_GETFL, 0);
    sockfl |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, sockfl);
 
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0){
        printf("Error\n");  
        return 1;
    }

    if (listen(sockfd, 5) < 0){
        printf("Error\n");  
        return 1;
    }

    struct pollfd pollfd;
    pollfd.fd = fileno(stdin);
    pollfd.events = POLLIN;

    while (1)
    {
        clientfd = accept(sockfd, (struct sockaddr *)&caddr, &clen);
        if (clientfd > 0)
        {
            printf("A client connected\n");
            printf("Message to client: ");

            int clientfl = fcntl(clientfd, F_GETFL, 0);
            clientfl |= O_NONBLOCK;
            fcntl(clientfd, F_SETFL, clientfl);
            int notfinish = 1;

            while (notfinish){

                if (recv(clientfd, clientms, sizeof(clientms), 0) > 0){
                    printf("\rClient: ");
                    printf("%s", clientms);
                    printf("Message: ");
                }
                if (poll(&pollfd, 1, 0) > 0){
                    if (pollfd.revents & POLLIN){
                        fgets(svms, sizeof(svms), stdin);
                        if (strcmp(svms,"/dc\n") == 0) {
                            printf("Disconnect to client\n");
                            break;
                        }
                        send(clientfd, svms, strlen(svms) + 1, 0);
                        printf("Message: ");
                    }
                }
            }
            close(clientfd);
        }
    }
    close(sockfd);
    return 0;
}