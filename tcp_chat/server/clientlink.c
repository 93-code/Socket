/* 11:09 2015-04-26 Sunday */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "clientlink.h"

int clinet_link_init(CLIENT *head){
    head->ip[0] = '\0';
    head->port = 0;
    head->name[0] = '\0';
    head->clientfd = -1;
    head->next = NULL;

    return 0;
}
int client_link_add(CLIENT *head, const char *ip, unsigned short port, const char *name, int clientfd){
    CLIENT **p = &(head->next);

    while (*p != NULL){
        if ((strcmp((*p)->ip, ip) == 0) && ((*p)->port == port)){
            return 1;
        }else{
            p = &((*p)->next);
        }
    }

    CLIENT *new = malloc(sizeof(CLIENT));
    strcpy(new->ip, ip);
    new->port = port;
    new->clientfd = clientfd;
    strcpy(new->name, name);
    new->next = NULL;

    *p = new;
    return 0;
}
int client_link_del(CLIENT *head, const char *ip, unsigned short port){
    CLIENT *t;
    CLIENT **p = &(head->next);

    while (*p != NULL){
        if ((strcmp((*p)->ip, ip) == 0) && ((*p)->port == port)){
            t = *p;
            *p = (*p)->next;
            free(t);
        }else{
            p = &((*p)->next);
        }
    }

    return 0;
}
CLIENT *client_link_get_addr_for_clientfd(CLIENT *head, int clientfd){
    CLIENT *p = head->next;

    while (p != NULL){
        if (p->clientfd == clientfd){
            break;
        }
        p = p->next;
    }
    return p;
}

int client_link_cal(CLIENT *head){
    int n = 0;
    CLIENT *p = head->next;

    while (p != NULL){
        n++;
        p = p->next;
    }

    return n;
}
