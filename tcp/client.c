/* 16:01 2015-04-23 Thursday */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, const char *argv[])
{
    int ret; 
    int sockfd;
    char packet[1024];
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("Fail to socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);


    ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret)
    {
        perror("Fail to connect");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        putchar('\r');
        putchar('>');

        fgets(packet, sizeof(packet), stdin);
        packet[strlen(packet)-1] = '\0';
        ret = send(sockfd, packet, strlen(packet), 0);
        if (-1 == ret)
        {
            perror("Fail to send");
            break;
        }

        ret = recv(sockfd, packet, sizeof(packet), 0);
        if (-1 == ret)
        {
            perror("Fail to recv");
            break;
        }
        packet[ret] = '\0';

        printf("============================\n");
        printf("recv[%d] : %s\n", ret, packet);
        printf("============================\n");
    }

    close(sockfd);
    
    return 0;

}
