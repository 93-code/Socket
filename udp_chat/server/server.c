/* 20:27 2015-04-23 Thursday */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client_link_list.h"
#include "protocol.h"

void broadcast_list(int sockfd, const char *packet, int len, CLIENT *head)
{
    int index = 1;
    int num;
    char ip[16];
    unsigned short port;
    char name[32];
    struct sockaddr_in peer_addr;

    bzero(&peer_addr, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;

    num = client_link_total(head);
    printf("%d client online\n",num);
    while (1){
        client_link_get_for_index(head, index, ip, &port, name);
        peer_addr.sin_addr.s_addr = inet_addr(ip);
        peer_addr.sin_port = htons(port);
        sendto(sockfd, packet, len, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
        /*printf("I had sendto\n");*/
        ++index;

        if (index > num)
            break;
    }
    /*printf("sendto over\n");*/
    return;

}
int main(int argc, const char *argv[])
{

    int ret = 0;
    int sockfd;
    int func;
    int len;
    char name[32];
    char packet[1024];
    char context[1024];
    struct sockaddr_in server_addr;
    struct sockaddr_in peer_addr;
    socklen_t addrlen = sizeof(peer_addr);

    CLIENT head;
    if (argc < 3){
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    client_link_init(&head);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sockfd){
        perror("Fail to socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret){
        perror("Fail to bind");
        exit(EXIT_FAILURE);
    }

    while (1){
        ret = recvfrom(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&peer_addr, &addrlen);
        if (-1 == ret){
            perror("Fail to bind");
            break;
        }
        packet[ret] = '\0';

        printf("========================\n");
        printf("ip      :%s\n", inet_ntoa(peer_addr.sin_addr));
        printf("port    :%d\n", ntohs(peer_addr.sin_port));
        printf("recv(%d):%s\n", ret, packet);
        printf("========================\n");


        func = packet_get_func(packet);

        switch (func){
        case FUNC_LOGIN:
            packet_get_context(packet, name);
            client_link_add(&head, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), name);
            break;
        case FUNC_CHAT:
            client_link_get_name_for_addr(&head, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), name);
            sprintf(context, "%s>", name);
            packet_get_context(packet, context+strlen(context));
            len = packet_chat(packet, context);
            broadcast_list(sockfd, packet, len, &head);
                break;
        case FUNC_QUIT:
            client_link_del(&head, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
            break;
        default:
            break;
        }
    }
    close(sockfd);
    return 0;
}
