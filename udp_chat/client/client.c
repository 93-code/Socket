/* 19:22 2015-04-22 Wednesday */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "protocol.h"

// ./server 127.0.0.1 8888
int main(int argc,  const char *argv[])
{
    int ret = 0;
    int sockfd;
    int len;
    char name[32];
    char packet[1024];
    char context[1024];
    pid_t pid;
    struct sockaddr_in server_addr;

    socklen_t addrlen = sizeof(server_addr);

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

    //login:
    write(1, "login:", 7);
    fgets(name, sizeof(name), stdin);
    name[strlen(name) - 1] = '\0';

    len = packet_login(packet, name);
    ret = sendto(sockfd, packet, len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret){
        perror("Fail to sendto");
        exit(EXIT_FAILURE);
    }
    
   //chat: 
    pid = fork();
    if (pid < 0)
    {
        perror("Fail to fork");
        exit(EXIT_FAILURE);
    }

    //send
    if (pid > 0){
        while (1){
            putchar('\r');
            putchar('>');
            fgets(packet,sizeof(context),stdin);

            if (strncmp(context, "quit", 4) == 0){
                len = packet_quit(packet);
            }else{
                len = packet_chat(packet, context);
            }
            ret = sendto(sockfd, packet, len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (-1 == ret){
                perror("Fail to sendto");
                exit(EXIT_FAILURE);
            }

            if (strncmp(context, "quit", 4) == 0){
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
                break;
            }
        }
    }
    if (pid == 0){
        while (1)
        {
            ret = recvfrom(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, &addrlen);
            printf("I have recv\n");
            if (-1 == ret)
            {
                perror("Fail to recvfrom");
                break;
            }
            packet[ret] = '\0';

            packet_get_context(packet, context);

            printf("---------------------------\n");
            printf("ip      :%s\n", inet_ntoa(server_addr.sin_addr));
            printf("prot    :%d\n", ntohs(server_addr.sin_port));
            printf("recv[%d]:%s\n", (int)strlen(context), context);
            printf("---------------------------\n");
            if (strncmp(packet,"quit",4) == 0)
            {
                exit(EXIT_SUCCESS);
            }
        }

    }

    close(sockfd);

    return 0;
}
