/* 10:57 2015-04-26 Sunday */
#ifndef __CLIENTLINK_H__
#define __CLIENTLINK_H__

typedef struct client{
    char ip[32];
    unsigned short port;
    char name[32];
    int clientfd;//通过一个唯一标示的socket，来从链表中获得此节点的信息，以便del指定节点
    
    struct client *next;
}CLIENT;

int clinet_link_init(CLIENT *head);
int client_link_add(CLIENT *head, const char *ip, unsigned short port, const char *name, int clientfd);
int client_link_del(CLIENT *head, const char *ip, unsigned short port);
CLIENT *client_link_get_addr_for_clientfd(CLIENT *head, int clientfd);
int client_link_cal(CLIENT *head);

#endif
