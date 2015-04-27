#ifndef	__TFTP_H__
#define __TFTP_H__

#define	LEN_PACKET_LEN		10
#define LEN_FUNC					2
#define	LEN_HEAD					(LEN_PACKET_LEN + LEN_FUNC)

enum {
	FUNC_LIST = 1,
	FUNC_PUT,
	FUNC_GET,
	FUNC_END
};

void packet_set_len(char *packet, int len);
int packet_get_len(const char *packet);
int packet_get_len(const char *packet);
int packet_get_func(const char *packet);
int recv_fix_len(int sockfd, char *buf, int len);
int packet_recv_head(int sockfd, int *func);
int packet_recv_proc(int sockfd);
int client_exe_list(int sockfd);
int server_exe_list(int sockfd);

#endif // __TFTP_H__
