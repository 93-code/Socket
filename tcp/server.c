/* 15:39 2015-04-23 Thursday */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, const char *argv[])
{
    int ret; 
    int sockfd;
    int clinetfd;
    char packet[1024];
    struct sockaddr_in server_addr;
    struct sockaddr_in peer_addr;
    socklen_t addrlen = sizeof(peer_addr);

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
    server_addr.sin_port   = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret)
    {
        perror("Fail to bind");
        exit(EXIT_FAILURE);
    }

    ret = listen(sockfd, 10);
    if (-1 == ret)
    {
        perror("Fail to listen");
        exit(EXIT_FAILURE);
    }

    clinetfd = accept(sockfd, (struct sockaddr *)&peer_addr, &addrlen);
    if (-1 == clinetfd)
    {
        perror("Fail to accept");
        exit(EXIT_FAILURE);
    }

    printf("============================\n");
    printf("ip  : %s\n",inet_ntoa(peer_addr.sin_addr));
    printf("port: %d\n",ntohs(peer_addr.sin_port));
    printf("============================\n");

    while (1)
    {
        ret = recv(clinetfd, packet, sizeof(packet), 0);
        if (-1 == ret)
        {
            perror("Fail to recv");
            break;
        }
        packet[ret] = '\0';

        printf("============================\n");
        printf("recv[%d] : %s\n", ret, packet);
        printf("============================\n");

        ret = send(clinetfd, packet, ret, 0);
        if (-1 == ret)
        {
            perror("Fail to send");
            break;
        }

        if (strncmp(packet,"quit",4) == 0)
            break;
    }

    close(sockfd);
    close(clinetfd);

    return 0;

}
