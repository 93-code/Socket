/* 19:25 2015-04-23 Thursday */
//#ifndef __CLIENT_LINK_LIST__
//#define __CLIENT_LINK_LIST__

typedef struct client_t
{
    char ip[16];
    unsigned short port;
    char name[32];

    struct client_t *next;
}CLIENT;

void client_link_init(CLIENT *head);
void client_link_add(CLIENT *head, const char *ip, unsigned short port, const char *name);
void client_link_del(CLIENT *head, const char *ip, unsigned short port);
int client_link_total(CLIENT *head);
int client_link_get_for_index(CLIENT *head, int index, char *ip, unsigned short *port, char *name);
void client_link_get_name_for_addr(CLIENT *head, char *ip, unsigned short *port, char *name);
//#endif
