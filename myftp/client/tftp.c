/*
 * ʵ��Ŀ��:
 * �ƶ���ʵ��tftpЭ��
 *
 * ʵ�ֲ���:
 * 1. �ƶ�Э��
 *    �ͻ���-->�����
 *    +-------------+------------+-----------------+------+
 *    | ���峤(10B) | ���ܺ�(2B) |      ����       | ���� |
 *    +-------------+------------+-----------------+------+
 *    |             |    "01"    |       ��        | list |
 *    +-------------+------------+-----------------+------+
 *    |             |    "02"    | �ļ��� | ����   | put  |
 *    +-------------+------------+-----------------+------+
 *    |             |    "03"    |     �ļ���      | get  |
 *    +-------------+------------+-----------------+------+
 *    
 *    �����-->�ͻ���
 *    +-------------+------------+-----------------+------+
 *    | ���峤(10B) | ���ܺ�(2B) |      ����       | ���� |
 *    +-------------+------------+-----------------+------+
 *    |             |    "01"    |  �ļ���Ŀ¼�б� | list |
 *    +-------------+------------+-----------------+------+
 *    |             |    "02"    |  �ļ����ս��   | put  |
 *    +-------------+------------+-----------------+------+
 *    |             |    "03"    |       ����      | get  |
 *    +-------------+------------+-----------------+------+
 *    (1) ���峤
 *        �������ַ�����ʾ���峤�ȣ����磺����Ϊ100����"0000000100"
 *
 *    (2) ����
 *        ��Э��Ľ���
 *
 *    (3) �ļ���
 *        ���ȹ̶�Ϊ32���ַ�������31���ַ����ļ����ַ���
 *
 *    (4) �ļ����ս��
 *        '1' - ���ճɹ�
 *        '0' - ����ʧ��
 *
 * 2. ���崦��
 *    2.1 ���/������峤��
 *    2.2 ���/������ܺ�
 *    2.3 ���ս�����ͷ����
 *
 * 3. ���ս������
 * 4. list����
 *    4.1 �ͻ���(��������)
 *    4.2 �����(������Ӧ)
 *    
 * 5. put����
 *    5.1 �ͻ���(��������)
 *    5.2 �����(������Ӧ)
 *
 * 6. get����
 *    6.1 �ͻ���(��������)
 *    6.2 �����(������Ӧ)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <dirent.h>
#include "tftp.h"
#include <sys/stat.h>
#include <fcntl.h>

// 2.1 ���/������峤��
void packet_set_len(char *packet, int len)
{
    char buf[LEN_PACKET_LEN + 1];

    sprintf(buf, "%10d", len);
    memcpy(packet, buf, LEN_PACKET_LEN);
}

int packet_get_len(const char *packet)
{
    char buf[LEN_PACKET_LEN + 1];

    memcpy(buf, packet, LEN_PACKET_LEN);
    buf[LEN_PACKET_LEN] = '\0';

    return atoi(buf);
}

// 2.2 ���/������ܺ�
void packet_set_func(char *packet, int func)
{
    char buf[LEN_FUNC + 1];

    sprintf(buf, "%02d", func);
    memcpy(packet + LEN_PACKET_LEN, buf, LEN_FUNC);
}

int packet_get_func(const char *packet)
{
    char buf[LEN_FUNC + 1];

    memcpy(buf, packet + LEN_PACKET_LEN, LEN_FUNC);
    buf[LEN_FUNC] = '\0';

    return atoi(buf);
}

// 2.3 ���ս�����ͷ����
int recv_fix_len(int sockfd, char *buf, int len)
{
    int ret = 0;
    int recv_len = 0;

    while (recv_len < len){
        ret = recv(sockfd, buf + recv_len, len - recv_len, 0);
        if (-1 == ret){
            perror("Fail to recv");
            goto exit;
        }

        recv_len += ret;
    }
    ret = recv_len;

exit:
    return ret;
}

int send_fix_len(int sockfd, const char *packet, int len)
{
    int ret = 0;
    int send_ret = 0;

    while (send_ret < len){
        ret = send(sockfd, packet + send_ret, len - send_ret, 0);
        if (-1 == ret){
            perror("Fail to send");
            goto exit;
        }
        send_ret += ret;
    }
    ret = send_ret;
exit:
    return ret;
}


int packet_recv_head(int sockfd, int *func)
{
    int ret = 0;
    char buf[LEN_HEAD];

    ret = recv_fix_len(sockfd, buf, LEN_HEAD);
    if (-1 == ret){
        goto exit;
    }
    ret = packet_get_len(buf);
    *func = packet_get_func(buf);
    if (*func < FUNC_LIST || *func >= FUNC_END){
        ret = -1;
    }

exit:
    return ret;
}

// 3. ���ս������
int packet_recv_proc(int sockfd)
{
    int ret = 0;
    int func;
    char packet[1024];

    ret = packet_recv_head(sockfd, &func);
    if (-1 == ret){
        goto exit;
    }

#ifdef	__DEBUG__
    printf("------------------------------\n");
    printf("recv : len = %d, func = %d\n", ret, func);
    printf("------------------------------\n");
#endif
    switch (func){
    case FUNC_LIST:
        ret = server_exe_list(sockfd);
        if (-1 == ret){
            perror("Fail to server_exe_list");
            goto exit;
        }
        break;

    case FUNC_PUT:
        ret = server_exe_put(sockfd, ret);
        break;

    case FUNC_GET:
        ret = server_exe_get(sockfd, ret);
        break;
    }

exit:
    return ret;
}

// 4.1 �ͻ���(��������)
int client_exe_list(int sockfd)
{
    int ret = 0;
    int func;
    char packet[1024];

    // ����list����
    packet_set_len(packet, LEN_HEAD);
    packet_set_func(packet, FUNC_LIST);

    ret = send(sockfd, packet, LEN_HEAD, 0);
    if (-1 == ret){
        perror("Fail to send");
        goto exit;
    }

    // ������ʾ�ļ���Ŀ¼�б�
    ret = packet_recv_head(sockfd, &func);
    if (-1 == ret){
        goto exit;
    }

    ret = recv_fix_len(sockfd, packet, ret);
    if (-1 == ret){
        goto exit;
    }
    packet[ret] = '\0';

    printf("%s\n", packet);

exit:
    return ret;
}

// 4.2 �����(������Ӧ)
int server_exe_list(int sockfd)
{
    int len = 0;
    DIR *pdir;
    struct dirent *pdirent = malloc(sizeof(struct dirent));
    int ret = 0;
    char packet[1024];


    len = LEN_HEAD;
    // ��Ŀ¼
    pdir = opendir(".");
    if (pdir == NULL){
        perror("Fail to opendir");
        ret = -1;
        goto exit;
    }
    // ��ȡĿ¼�µ��ļ�����Ŀ¼�б�
    while ((pdirent = readdir(pdir)) != NULL){
        if (pdirent->d_name[0] == '.'){
            continue;
        }

        sprintf(packet + len, "%s ", pdirent->d_name);
        //+1:' '����
        len += strlen(pdirent->d_name)+1;
    }
    // �������
    printf("%d\n",len);
    //�������
    packet_set_len(packet, len - LEN_HEAD);
    //������ܺ�
    packet_set_func(packet, FUNC_LIST);
    ret = send_fix_len(sockfd, packet, len);
    if (-1 == ret){
        perror("Fail to send");
        goto exit;
    }
exit:
    // �ɹ����͵ĳ���
    return ret;
}

int client_exe_put(int sockfd, const char *name)
{
    int fd;
    int file_len;
    int send_len;
    int ret = 0;
    char packet[1024];

    //open file
    fd = open(name, O_RDONLY);
    if (-1 == fd){
        perror("Fail to open");
        ret = -1;
        goto exit;
    }

    //get file len
    ret = lseek(fd, 0, SEEK_END);
    if (-1 == ret){
        perror("Fail to lseek");
        goto exit;
    }

    file_len = ret;

    //packet head + name
    packet_set_len(packet, LEN_FILE_NAME + file_len);
    packet_set_func(packet, FUNC_PUT);
    strcpy(packet + LEN_HEAD, name);

    ret = send_fix_len(sockfd, packet, LEN_HEAD + LEN_FILE_NAME);
    if (-1 == ret){
        goto exit;
    }

    //send file context;
    send_len = 0;
    lseek(fd, 0, SEEK_SET);

#ifdef __DEBUG__
    printf("send..........\n");
#endif
    while (send_len < file_len){
        ret = read(fd, packet, sizeof(packet));
        if (-1 == ret){
            perror("Fail to read");
            goto exit;
        }

        ret = send_fix_len(sockfd, packet, ret);
        if (-1 == ret){
            goto exit;
        }
        send_len += ret;
    }

#ifdef __DEBUG__
    printf("send. over.........\n");
#endif
    //recv results
    ret = recv_fix_len(sockfd, packet, LEN_HEAD + LEN_PUT_SUCCESS);
    if (-1 == ret){
        goto exit;
    }
    //print results
    if ('1' == packet[LEN_HEAD]){
        printf("put successfully.\n");
    }else{
        printf("put failure.\n");
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}

int server_exe_put(int sockfd, int packet_len)
{
    int fd;
    int ret = 0; 
    int recv_len;
    char packet[1024];
    int file_len = packet_len - LEN_FILE_NAME;
    char name[LEN_FILE_NAME];

    ret = recv_fix_len(sockfd, name, LEN_FILE_NAME);
    if (-1 == ret){
        goto exit;
    }

    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (-1 == fd){
        perror("Fail to open");
        ret = -1;
        goto exit;
    }

    //recv file context;
    recv_len = 0;
    while (recv_len < file_len){
        ret = recv(sockfd, packet, sizeof(packet), 0);
        if (-1 == ret){
            perror("Fail to recv");
            goto exit;
        }
        write(fd, packet, ret);
        recv_len += ret;
    }
    //return results
    packet_set_len(packet, LEN_PUT_SUCCESS);
    packet_set_func(packet, FUNC_PUT);
    packet[LEN_HEAD] = '1';
    send(sockfd, packet, LEN_HEAD + LEN_PUT_SUCCESS, 0);
exit:
    close(fd);
    return ret;
}

int client_exe_get(int sockfd, const char *name)
{
    int fd;
    int ret;
    int func;
    int recv_len;
    int file_len;
    char packet[1024];

    packet_set_len(packet, LEN_FILE_NAME);
    packet_set_func(packet, FUNC_GET);
    strcpy(packet + LEN_HEAD, name);
    //send file name
    ret = send_fix_len(sockfd, packet, LEN_HEAD + LEN_FILE_NAME);
    if (-1 == ret){
        perror("Fail to send");
        goto exit;
    }

    //open file
    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (-1 == fd){
        perror("Fail to open");
        ret = -1;
        goto exit;
    }
    //recv head
    /*recv_fix_len(sockfd, packet, LEN_HEAD);*/
    ret = packet_recv_head(sockfd, &func);
    if (-1 == ret){
        goto exit;
    }
    //recv file
    file_len = ret;
    recv_len = 0;
    while (recv_len < file_len){
        ret = recv(sockfd, packet, sizeof(packet), 0);
        if (-1 == ret){
            perror("Fail to 111 recv");
            goto exit;
        }
        write(fd, packet, ret);
        recv_len += ret;
    }

exit:
    close(fd);
    return ret;
}

int server_exe_get(int sockfd, int packet_len)
{
    int fd;
    int ret;
    int file_len;
    int send_len;
    char name[LEN_FILE_NAME];
    char packet[1024];

    ret = recv_fix_len(sockfd, name, LEN_FILE_NAME);
    if (-1 == ret){
        goto exit;
    }

    //open file
    fd = open(name, O_RDONLY);
    if (-1 == fd){
        goto exit;
    }

    //get file len
    ret = lseek(fd, 0, SEEK_END);
    if (-1 == ret){
        perror("Fail to lseek");
        goto exit;
    }
    file_len = ret;

    //send head
    packet_set_len(packet, file_len);
    packet_set_func(packet, FUNC_GET);
    ret = send_fix_len(sockfd, packet, LEN_HEAD);
    if (-1 == ret){
        goto exit;
    }
    
    //send file context
    send_len = 0;
    lseek(fd, 0, SEEK_SET);

    while (send_len < file_len){
        ret = read(fd, packet, sizeof(packet));
        if (-1 == ret){
            perror("Fail to read");
            goto exit;
        }

        ret = send_fix_len(sockfd, packet, ret);
        if (-1 == ret){
            goto exit;
        }
        send_len += ret;
    }

exit:
    close(fd);
    return ret;
}
