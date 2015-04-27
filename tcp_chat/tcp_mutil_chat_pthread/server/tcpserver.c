/* 20:13 2015-04-25 星期六 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "clientlink.h"

CLIENT head; 
void broadcast_msg(CLIENT *head, char *buf, int len){
    CLIENT *p = head->next;
    
    printf("len = %d\n", len);
    while (p != NULL){
        send(p->clientfd, buf, len, 0);
        p = p->next;
    }
    return;
}

void *handler(void *arg){
    CLIENT *p;
    int clientfd = (int)(long int)arg;
    char buf[1024];
    int bsize;

    /*printf("This is a child pthread\n");*/
    while (1){
    bsize = recv(clientfd, buf, sizeof(buf), 0);
    buf[bsize] = '\0';

    printf("recv: %s\n", buf);
    if (strncmp(buf, "quit", 4) == 0){
        //返回的节点地址
        p = client_link_get_addr_for_clientfd(&head, clientfd);
        //del client
        client_link_del(&head, p->ip, p->port);
        break;
    }

    broadcast_msg(&head, buf, strlen(buf));
    }
    close(clientfd);
    return;
}

int main(int argc, const char *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage : %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ret;
    int sockfd; 
    int clientfd;
    pthread_t tid;
    struct sockaddr_in server_addr;
    struct sockaddr_in peer_addr;
    socklen_t addrlen = sizeof(peer_addr); 
    

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd){
        perror("Fail to create socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret){
        perror("Fail to bind");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 11);

    clinet_link_init(&head);

    while (1){
        clientfd = accept(sockfd, (struct sockaddr *)&peer_addr, &addrlen);
        if (-1 == clientfd){
            perror("Fail to accept");
            break;
        }

        printf("-----------------------\n");
        printf("ip      :%s\n", inet_ntoa(peer_addr.sin_addr));
        printf("port    :%d\n", ntohs(peer_addr.sin_port));
        printf("-----------------------\n");
        //add to link
        client_link_add(&head, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), "", clientfd);

        printf("pthread_create....\n");
        ret = pthread_create(&tid, NULL, handler, (void *)(long int)clientfd);
        if (-1 == ret){
            perror("Fail to pthread_create");
            break;
        }

        pthread_detach(tid);

    }
    close(sockfd);

    return 0;
}
