/* 19:22 2015-04-22 Wednesday */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ./server 127.0.0.1 8888
int main(int argc,  const char *argv[])
{
    int ret = 0;
    int sockfd;
    char packer[1024];
    struct sockaddr_in server_addr;
    struct sockaddr_in peer_addr;

    socklen_t addrlen = sizeof(peer_addr);
    
    if (argc < 3){
        fprintf(stderr, "Usage: %s <ip> <port>\n",argv[0]);
        exit(EXIT_FAILURE);
        }

    //create and open socket;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sockfd){
        perror("Fail to socket.");
        exit(EXIT_FAILURE);
    }

    //init
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    //bind ip and port;
    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (-1 == ret){
        perror("Fail to bind");
        exit(EXIT_FAILURE);
    }

    //recv and send packet;
    while (1)
    {
        //recv
        ret = recvfrom(sockfd, packer, sizeof(packer), 0, (struct sockaddr *)&peer_addr, &addrlen);
        if (-1 == ret)
        {
            perror("Fail to recvfrom");
            break;
        }
        packer[ret] = '\0';


        printf("---------------------------\n");
        printf("ip      :%s\n", inet_ntoa(peer_addr.sin_addr));
        printf("prot    :%d\n", ntohs(peer_addr.sin_port));
        printf("recv[%d]:%s\n", ret, packer);
        printf("---------------------------\n");

        //send
        ret = sendto(sockfd, packer , ret, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
        if (-1 == ret){
            perror("Fail to sendto");
            exit(EXIT_FAILURE);
        }
        if (strncmp(packer,"quit",4) == 0)
        {
            printf("%s:%d quit\n",inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
        }
    }
    close(sockfd);

    return 0;
}
