/*
 * ʵ��Ŀ��:
 * tftp�Ŀͻ���
 * 
 * ʵ�ֲ���:
 * 1. ��������
 * 2. �������
 * 3. ���ӷ�����
 * 4. ����
 *    4.1 list
 *    4.2 put
 *    4.3 get
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// tftp> Ϊ��ʾ�������������cmd��
// tftp> \0;
// tftp>list\0;
// tftp>put test\0;
// tftp> put test\0;
// tftp>put  test\0;
// tftp>put  test \0;

// tftp>put\0  test \0;
// argv[0] ��p��ʼ
// argv[1] ��t��ʼ
int cmd_parse(char *cmd, char *argv[], int maxargs)
{
	int i = 0;
	int count = 0;
	char *p = cmd;
	
	// ȥ��ǰ���ո�
	// tftp> put test\0;
	while (' ' == *p) p++;
	
	// ������
	// tftp> \0;
	if ('\0' == *p) {
		count = 0;
		goto exit;
	} 
	argv[count] = p;
	
	while (count <= maxargs){
		
		// �ҵ��ո��\0'
		// tftp>list\0;
		// tftp>put  test\0;
		while (*p !=  ' ' && *p != '\0') p++;
		if ('\0' == *p){
			break;
		}
		*p++ = '\0';
		
		// ȥ�������м�Ŀո�
		// tftp>put  test\0;
		// tftp>put  test \0;
		while (' ' == *p) p++;
		if ('\0' == *p){
			break;
		}
		
		// ��һ�������Ŀ�ʼλ��
		count += 1;
		argv[count] = p;
	}
	
	count += 1;
#ifdef	__DEBUG__
	for (i = 0; i < count; i++){
		printf("argv[%d] = %s\n", i, argv[i]);
	}
#endif

exit:
	return count;
}

void cmd_help(void)
{
	printf("------------------------------\n");
	printf("list\n");
	printf("put <file>\n");
	printf("get <file>\n");
	printf("------------------------------\n");
}

int cmd_exe(int sockfd, int argc, char *argv[])
{
	if (strcmp(argv[0], "list") == 0){
		//  4.1 list
		client_exe_list(sockfd);
	} else if (strcmp(argv[0], "put") == 0 && 2 == argc){
		// 4.2 put
		
	} else if (strcmp(argv[0], "get") == 0 && 2 == argc){
		// 4.3 get
	} else {
		return -1;
	}
	
	return 0;
}

// ./client 192.168.0.249 8888
int main(int argc, const char *argv[])
{
	int ret = 0;
	int sockfd;
	char packet[1024];
	char cmd[1024];
	char *args[10];
	int count;
	struct sockaddr_in server_addr;
	
	if (argc < 3){
		fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// ���÷���˵�ַ
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	while (1){
		// ��������socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == sockfd){
			perror("Fail to socket.");
			exit(EXIT_FAILURE);
		}
	
		// 1. ��������
		putchar('\n');
		printf("tftp>");
		fgets(cmd, sizeof(cmd), stdin);
		cmd[strlen(cmd) - 1] = '\0';
		
		// 2. �������
		count = cmd_parse(cmd, args, sizeof(args) / sizeof(args[0]));
		if (0 == count){
			cmd_help();
			continue;
		}
		
		// 3. ���ӷ�����
		ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if (-1 == ret){
			perror("Fail to connect.");
			break;
		}
		
		//  4. ����
		ret = cmd_exe(sockfd, count, args);
		if (-1 == ret){
			cmd_help();
		}

		close(sockfd);
	}
	
	
	return 0;
}