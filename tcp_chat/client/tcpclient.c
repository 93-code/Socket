/* 22:36 2015-04-25 星期六 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, const char *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage : %s <ip> <port> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ret;
    int sockfd;
    char buf[1024];
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd){
        perror("Fail to socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, addrlen);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    ret = connect(sockfd, (struct sockaddr *)&server_addr, addrlen);
    if (-1 == ret){
        perror("Fail to connect");
        exit(EXIT_FAILURE);
    }

    while (1){
        putchar('\r');
        putchar('>');

        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';

        send(sockfd, buf, strlen(buf), 0);
        
        /*recv(sockfd, buf, sizeof(buf), 0);*/
        if (strncmp(buf, "quit", 4) == 0){
            break;
        }
    }
    close(sockfd);

    return 0;
}
