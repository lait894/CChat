#ifndef CCHAT_H
#define CCHAT_H

#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_WORDS_BYTES 256
#define MAX_CHATS 300

#define DEFAULT_ADDR  "127.0.0.1"
#define DEFAULT_PORT  "3333"
#define LOG_FILE_PATH "./log"
#define LOG_FILE_NAME "CChat"

#define DEBUG  1, __FILE__, __LINE__
#define WARN   2, __FILE__, __LINE__
#define ERROR  3, __FILE__, __LINE__
#define NORMAL 4, __FILE__, __LINE__

#define TCP_BUF_SIZE 2048

#define MAX_CLIENT_NUM    2
#define TCP_LISTEN_LENGTH 20

#define RESP0001 "CCRP0001"
#define RESP0002 "CCRP0002"

#define TRUE  1
#define FALSE 0

#define CLIENT_NAME_LEN 200

//#define DEBUG_LOG

void* runTerm(void* p);
int cclog(int level, char* filename, int line, char* msg, ...);
int sendMsg(int sock, char* msg, int msgLen);
int recvMsg(int sock, char* recvBuf, int recvBufLen);
int runServer(char* local_addr, int local_port);
int runClient(char* remote_addr, int remote_port);

#endif
