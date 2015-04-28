/*
 * 实现目标:
 * 制定并实现tftp协议
 *
 * 实现步骤:
 * 1. 制定协议
 *    客户端-->服务端
 *    +-------------+------------+-----------------+------+
 *    | 包体长(10B) | 功能号(2B) |      包体       | 功能 |
 *    +-------------+------------+-----------------+------+
 *    |             |    "01"    |       无        | list |
 *    +-------------+------------+-----------------+------+
 *    |             |    "02"    | 文件名 | 内容   | put  |
 *    +-------------+------------+-----------------+------+
 *    |             |    "03"    |     文件名      | get  |
 *    +-------------+------------+-----------------+------+
 *    
 *    服务端-->客户端
 *    +-------------+------------+-----------------+------+
 *    | 包体长(10B) | 功能号(2B) |      包体       | 功能 |
 *    +-------------+------------+-----------------+------+
 *    |             |    "01"    |  文件和目录列表 | list |
 *    +-------------+------------+-----------------+------+
 *    |             |    "02"    |  文件接收结果   | put  |
 *    +-------------+------------+-----------------+------+
 *    |             |    "03"    |       内容      | get  |
 *    +-------------+------------+-----------------+------+
 *    (1) 包体长
 *        用数字字符串表示包体长度，比如：长度为100，用"0000000100"
 *
 *    (2) 功能
 *        对协议的解释
 *
 *    (3) 文件名
 *        长度固定为32个字符，存放最长31个字符的文件名字符串
 *
 *    (4) 文件接收结果
 *        '1' - 接收成功
 *        '0' - 接收失败
 *
 * 2. 包体处理
 *    2.1 打包/解包包体长度
 *    2.2 打包/解包功能号
 *    2.3 接收解析包头解析
 *
 * 3. 接收解析框架
 * 4. list命令
 *    4.1 客户端(命令请求)
 *    4.2 服务端(命令响应)
 *    
 * 5. put命令
 *    5.1 客户端(命令请求)
 *    5.2 服务端(命令响应)
 *
 * 6. get命令
 *    6.1 客户端(命令请求)
 *    6.2 服务端(命令响应)
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

// 2.1 打包/解包包体长度
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

// 2.2 打包/解包功能号
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

// 2.3 接收解析包头解析
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

// 3. 接收解析框架
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

// 4.1 客户端(命令请求)
int client_exe_list(int sockfd)
{
    int ret = 0;
    int func;
    char packet[1024];

    // 发送list请求
    packet_set_len(packet, LEN_HEAD);
    packet_set_func(packet, FUNC_LIST);

    ret = send(sockfd, packet, LEN_HEAD, 0);
    if (-1 == ret){
        perror("Fail to send");
        goto exit;
    }

    // 接收显示文件和目录列表
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

// 4.2 服务端(命令响应)
int server_exe_list(int sockfd)
{
    int len = 0;
    DIR *pdir;
    struct dirent *pdirent = malloc(sizeof(struct dirent));
    int ret = 0;
    char packet[1024];


    len = LEN_HEAD;
    // 打开目录
    pdir = opendir(".");
    if (pdir == NULL){
        perror("Fail to opendir");
        ret = -1;
        goto exit;
    }
    // 获取目录下的文件名和目录列表
    while ((pdirent = readdir(pdir)) != NULL){
        if (pdirent->d_name[0] == '.'){
            continue;
        }

        sprintf(packet + len, "%s ", pdirent->d_name);
        //+1:' '隔开
        len += strlen(pdirent->d_name)+1;
    }
    // 打包发送
    printf("%d\n",len);
    //打包包长
    packet_set_len(packet, len - LEN_HEAD);
    //打包功能号
    packet_set_func(packet, FUNC_LIST);
    ret = send_fix_len(sockfd, packet, len);
    if (-1 == ret){
        perror("Fail to send");
        goto exit;
    }
exit:
    // 成功发送的长度
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
