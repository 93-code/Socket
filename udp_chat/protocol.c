/* 21:24 2015-04-24 星期五 */
#include <stdio.h>
#include <string.h>
#include "protocol.h"


int packet_login(char *packet, const char *name){
    int len = 0;

    sprintf(packet, "%02d", FUNC_LOGIN);
    len += LEN_FUNC;

    strcpy(packet + LEN_FUNC, name);
    len += strlen(name);

    //add the '\0' len
    return len + 1;
}
int packet_chat(char *packet, const char *context){
    int len = 0;

    sprintf(packet, "%02d", FUNC_CHAT);
    len += LEN_FUNC;

    strcpy(packet + LEN_FUNC, context);
    len += strlen(context);

    return len + 1;
}
int packet_quit(char *packet){
    int len = 0;
    
    sprintf(packet, "%02d", FUNC_QUIT);
    len += LEN_FUNC;

    return len + 1;
}
int packet_get_func(char *packet){
    char buf[16];

    strncpy(buf, packet, LEN_FUNC);
    buf[LEN_FUNC] = '\0';

    return atoi(buf);
}
void packet_get_context(char *packet, char *context){
    strcpy(context, packet + LEN_FUNC);
    return;
}
