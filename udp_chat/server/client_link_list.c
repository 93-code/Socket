/* 19:31 2015-04-23 Thursday */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "client_link_list.h"

void client_link_init(CLIENT *head)
{
    head->ip[0] = '\0';
    head->port  = 0;
    head->name[0] = '\0';

    head->next = NULL;
    return;
}

void client_link_add(CLIENT *head, const char *ip, unsigned short port, const char *name)
{
    CLIENT **p = &(head->next);   

    while (*p != NULL)
    {
        if ((strcmp(ip,*p->ip) == 0) && (port == *p->port)){
            return;
        }
        else{
            p = &(*p->next);
        }
    }
    
    CLIENT *new = malloc(sizeof(CLIENT));
    strcpy(new->ip,ip);
    strcpy(new->name,name);
    new->port = port;

    *p->next = new;

    return; 
}

void client_link_del(CLIENT *head, const char *ip, unsigned short port);
{
    CLIENT *t;
    CLIENT **p = &(head->next); 

    while (*p != NULL)
    {
        if ((strcmp(ip,*p->ip) == 0) && (port == *p->port))
        {
           t = *p; 
           p = &(*p->next);
           free(t);
        }
        else
        {
            p = &(*p->next);
        }
    }
    return;
}

int client_link_get_for_index(CLIENT *head, int index, char *ip, unsigned short *port, char *name)
{
    CLIENT *p = head->next;
    int i = 1;    
    for (i = 1; i <= index; i++)
    {
       p = p->next; 
    }

    strcpy(ip,p->ip);
    strcpy(name,p->name);
    *port = p->port;

    return 0;
}
