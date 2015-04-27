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
    int flags;
    int maxfd;
    int sockfd; 
    int clientfd;
    fd_set clientfds;
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

    //设置监听套接字为非阻塞
    flags = fcntl(sockfd, F_GETCL);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETCL, flags);

    //创建文件描述符集
    FD_ZERO(&clientfds);

    while (1){
        //接受套接字
        clientfd = accept(sockfd, (struct sockaddr *)&peer_addr, &addrlen);
        if (-1 == clientfd && errno != EWOULDBLOCK && errno != EAGAIN){
            perror("Fail to accept");
            break;
        }else if (-1 == clientfd && errno == EWOULDBLOCK && errno == EAGAIN){
            ;
        }else {

            //链接成功
        printf("-----------------------\n");
        printf("ip      :%s\n", inet_ntoa(peer_addr.sin_addr));
        printf("port    :%d\n", ntohs(peer_addr.sin_port));
        printf("-----------------------\n");
        //add to link
        client_link_add(&head, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), "", clientfd);

        // 设置客户端非阻塞
        flags = fcntl(clientfd, F_GETCL);
        flags |= O_NONBLOCK;
        fcntl(clientfd, F_SETCL, flags);

        //添加到clientfds
        FD_SET(clientfd, &clientfds);
        if (maxfd < clientfd) maxfd = clientfd;
        }

    }
    close(sockfd);

    return 0;
}
