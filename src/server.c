#include "cchat.h"
#include "list.h"

extern int isRunning;

typedef struct {
    int  sock; // socket fd
    char name[CLIENT_NAME_LEN];  // client name
} Client;

List clients;
int listen_d = 0;

pthread_mutex_t p_lock = PTHREAD_MUTEX_INITIALIZER;

int addClient(Client* c)
{
    int result;
    pthread_mutex_lock(&p_lock);
    result = listAdd(&clients, c);
    pthread_mutex_unlock(&p_lock);
    return result;
}

int removeClient(Client* c)
{
    int result;
    pthread_mutex_lock(&p_lock);
    result = listDel(&clients, c);
    pthread_mutex_unlock(&p_lock);
    return result;
}

int clearClient() 
{
    int result;
    pthread_mutex_lock(&p_lock);
    result = listClear(&clients);
    pthread_mutex_unlock(&p_lock);
    return result;
}

Client* getClient(int index)
{
    Client *target;
    pthread_mutex_lock(&p_lock);
    target = (Client *)listGet(&clients, index);
    pthread_mutex_unlock(&p_lock);
    return target;   
}

int clientNum()
{
    return clients.size;
}

int acceptAndCheckClient(Client* cli, char* sendBuf, char* recvBuf)
{
    int ret, used = FALSE, i;
    Client *pc = NULL;

    while ((ret = recvMsg(cli->sock, recvBuf, TCP_BUF_SIZE)) == 0 && isRunning) {
        used = FALSE;

        for (i = 0; i < clients.size; i++) {
            pc = (Client *)listGet(&clients, i);
            if (memcmp(pc->name, recvBuf, strlen(recvBuf)) == 0) {
                used = TRUE;
                break;
            }
        }

        if (used == FALSE) {
            if ((ret = sendMsg(cli->sock, "DONE", 4)) == 0) {
                strcpy(cli->name, recvBuf);
                memset(recvBuf, 0, TCP_BUF_SIZE);
                cclog(NORMAL, "client [%s] joined!\n", cli->name);

                if ((ret = addClient(cli)) == 0) {
                    fprintf(stdout, "Client [%s] entered [%d / %d].\n", cli->name, clientNum(), MAX_CLIENT_NUM);
                    return 0;
                } else {
                    cclog(ERROR, "addClient error\n");
                }             
            } else {
                cclog(ERROR, "sendMsg error.\n");
            }

            break;
        } else {
            cclog(NORMAL, "Client name [%s] is already be used!\n", cli->name);

            if ((ret = sendMsg(cli->sock, RESP0002, strlen(RESP0002)))) {
                cclog(ERROR, "sendMsg error.\n");
                break;
            }

            memset(sendBuf, 0, TCP_BUF_SIZE);
            memset(recvBuf, 0, TCP_BUF_SIZE);
        }
    }

    return -1;
}

void* clientHandler(void *p)
{
    cclog(DEBUG, "thread is running!\n");

    int ret = 0, used = FALSE, i; 
    Client *cli = (Client *)p, *tc = NULL;

    char *sendBuf = malloc(TCP_BUF_SIZE);
    char *recvBuf = malloc(TCP_BUF_SIZE);
    memset(sendBuf, 0, TCP_BUF_SIZE);
    memset(recvBuf, 0, TCP_BUF_SIZE);

    if (acceptAndCheckClient(cli, sendBuf, recvBuf) == 0) {
        while(isRunning) {
            memset(sendBuf, 0, TCP_BUF_SIZE);
            memset(recvBuf, 0, TCP_BUF_SIZE);

            if ((ret = recvMsg(cli->sock, recvBuf, TCP_BUF_SIZE)) == 0) {
                memset(sendBuf, 0, TCP_BUF_SIZE);
                sprintf(sendBuf, "%s:%s", cli->name, recvBuf);
                for (i = 0; i < clientNum(); i++) {
                    if ((tc = (Client *)getClient(i))) {
                        if ((ret = sendMsg(tc->sock, sendBuf, strlen(sendBuf)))) {
                            cclog(ERROR, "sendBuf to sock [%d] error\n", tc->sock);
                            continue;
                        }
                    }
                }
            } else if (ret == -1) {
                cclog(ERROR, "recvMsg error, ret=[%d]\n", ret);
                break;
            } else if (ret == -2) {
                cclog(ERROR, "recvMsg error, ret=[%d]\n", ret);
                break;
            }
        }   
        
        char tCliName[200] = {0};
        strcpy(tCliName, cli->name);
        if ((ret = removeClient(cli))) {
            cclog(ERROR, "removeClient error, %d\n", ret);
        } 
        fprintf(stdout, "Client [%s] has left [%d / %d].\n", tCliName, clientNum(), MAX_CLIENT_NUM);     
    }

    free(sendBuf);
    free(recvBuf);
    close(cli->sock);
    
    pthread_exit(NULL);
}

int runServer(int local_port)
{
    fprintf(stdout, "Chat server is running on port %d :\n", local_port);
    
    int ret, connect_d, listen_d, reuse = -1, c;

    if ((listen_d = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        cclog(ERROR, "socket create error [%s]\n", strerror(listen_d));
        return -1;
    }

    if (setsockopt(listen_d, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int)) == -1) {
        cclog(ERROR, "setsockopt error!\n");
        return -1;
    }

    struct sockaddr_in name;
    name.sin_family = PF_INET;
    name.sin_port = (in_port_t)htons(local_port);
    if ((c = bind(listen_d, (struct sockaddr *)&name, sizeof(name)))) {
        cclog(ERROR, "Tcp bind error [%d], listen_d=[%d], local_port=[%d]\n", c, listen_d, local_port);
        return c;
    }
    if ((ret = listen(listen_d, TCP_LISTEN_LENGTH)) < 0) {
        cclog(ERROR, "Tcp listen error, ret=[%d]\n", ret);
        return ret;
    }

    struct sockaddr_storage client_addr;
    unsigned int address_size = sizeof(client_addr);

    pthread_attr_t thread_attr;
    if ((ret = pthread_attr_init(&thread_attr)) != 0) {
    	cclog(ERROR, "pthread_attr_init error %d \n", ret);
    	return -1;
    }
    if ((ret = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED)) != 0) {
    	cclog(ERROR, "pthread_attr_setdetachstate error %d \n", ret);
    	return -1;
    }
    pthread_t tid;

    isRunning = TRUE;

    while(isRunning) {
        
        connect_d = accept(listen_d, (struct sockaddr*)&client_addr, &address_size);
        if (connect_d < 0) {
            cclog(ERROR, "Tcp accpet failed!\n");
            continue;
        }
        cclog(DEBUG, "Tcp accepted! socket fd = [%d]\n", connect_d);
        cclog(DEBUG, "clientNum = [%d]\n", clientNum());

        if (clientNum() < MAX_CLIENT_NUM) {
            ret = sendMsg(connect_d, "ok", 2);
            if (ret < 0) {
                cclog(ERROR, "sendMsg error!\n");
                close(connect_d);
            } else {
                Client* tc = malloc(sizeof(Client));
                tc->sock = connect_d;
                if ((ret = pthread_create(&tid, &thread_attr, clientHandler, (void*)tc))) {
                    cclog(ERROR, "pthread_create error, ret=[%d]\n", ret);
                    break;
                }                
            }
        } else {
            // No more new connection if room is full.
            ret = sendMsg(connect_d, RESP0001, strlen(RESP0001));
            close(connect_d);
        }
    }

    pthread_attr_destroy(&thread_attr);
    clearClient();

    return 0;
}