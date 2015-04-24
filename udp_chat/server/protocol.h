/* 21:01 2015-04-24 星期五 */
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define LEN_FUNC 2

enum {
    FUNC_LOGIN = 1,
    FUNC_CHAT,
    FUNC_QUIT,
    FUNC_END
};
int packet_login(char *packet, const char *name);
int packet_chat(char *packet, const char *context);
int packet_quit(char *packet);
int packet_get_func(char *packet);
void packet_get_context(char *packet, char *context);

#endif
