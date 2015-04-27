/*
 * ʵ��Ŀ��:
 * tftp�����
 *
 * ʵ�ֲ���:
 * 1. TCP��������(�Ѿ�ʵ��)
 * 2. tftpЭ��
 *    ��tftp.c���͡�tftp.h��
 *
 * 3. ��ͻ��˲�����Ӧ(���߳�)
 *    3.1 �����̻߳ص�����
 *    3.2 ���������߳�
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "tftp.h"

// 3.1 �����̻߳ص�����
void * handler(void *arg)
{
	int ret = 0;
	int sockfd = (int)(long int)arg;
	
	ret = packet_recv_proc(sockfd);
	close(sockfd);
	
	return (void *)(long int)ret;
}

// ./server 192.168.0.249 8888
int main(int argc, const char *argv[])
{
	int ret;
	int sockfd;
	int clientfd;
	pthread_t tid;
	char packet[1024];
	struct sockaddr_in server_addr;
	struct sockaddr_in peer_addr;
	socklen_t addrlen = sizeof(peer_addr);
	
	
	if (argc < 3){
		fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// ��������socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd){
		perror("Fail to socket.");
		exit(EXIT_FAILURE);
	}
	
	// ��ip��ַ(����)�Ͷ˿�(�Լ��Ķ˿�)������socket
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (-1 == ret){
		perror("Fail to bind.");
		exit(EXIT_FAILURE);
	}
	
	// listen(��socket����Ϊ����ģʽ)
	ret = listen(sockfd, 10);
	if (-1 == ret){
		perror("Fail to listen.");
		exit(EXIT_FAILURE);
	}
	
	while (1){
		// accept(���տͻ�������, �����ͻ���socket)
		clientfd = accept(sockfd, (struct sockaddr *)&peer_addr, &addrlen);
		if (-1 == clientfd){
			perror("Fail to accept.");
			exit(EXIT_FAILURE);
		}
		printf("--------------------------------\n");
		printf("ip   : %s\n", inet_ntoa(peer_addr.sin_addr));
		printf("port : %d\n", ntohs(peer_addr.sin_port));
		printf("--------------------------------\n");
		
		// 3.2 ���������߳�
		pthread_create(&tid, NULL, handler, (void *)(long int)clientfd);
		pthread_detach(tid);
	}
	
	// �رռ����׽���
	close(sockfd);
	
	return 0;
}
