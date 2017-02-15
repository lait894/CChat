#include "CChat.h"
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

void* clientHandler(void *p)
{
    cclog(DEBUG, "thread is running!\n");

    int ret = 0; 
    Client* cli = (Client *)p;

    char* sendBuf = malloc(TCP_BUF_SIZE);
    char* recvBuf = malloc(TCP_BUF_SIZE);

    // Get client name.
    //if (recvMsg(cli->sock, recvBuf, TCP_BUF_SIZE) == 0) {
        memcpy(cli->name, "hahaha", 8);
        cclog(NORMAL, "client [%s] joined!\n", cli->name);

        ret = addClient(cli);
        if (ret) {
            cclog(ERROR, "addClient failed, ret=%d\n", ret);
            close(cli->sock);
            free(sendBuf);
            free(recvBuf);
            free(cli);
            pthread_exit(NULL);
            return NULL;
        }

        fprintf(stdout, "Client [%s] entered [%d / %d].\n", 
                    cli->name,
                    clientNum(),
                    MAX_CLIENT_NUM);

    //}

    while(isRunning) {
        ret = recvMsg(cli->sock, recvBuf, TCP_BUF_SIZE);
        if (ret == 0) {
            memset(sendBuf, 0, TCP_BUF_SIZE);
            sprintf(sendBuf, "%s:%s", cli->name, recvBuf);
            Client *tc = NULL;
            for (int i = 0; i < clientNum(); i++) {
                tc = (Client *)getClient(i);
                if (tc) {
                    ret = sendMsg(tc->sock, sendBuf, strlen(sendBuf));
                    if (ret) {
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

        memset(sendBuf, 0, TCP_BUF_SIZE);
        memset(recvBuf, 0, TCP_BUF_SIZE);
    }

    free(sendBuf);
    free(recvBuf);
    close(cli->sock);
    removeClient(cli);

    pthread_exit(NULL);
}

int runServer(char* local_addr, int local_port)
{
    fprintf(stdout, "Chat server is running on %s:%d\n", local_addr, local_port);
    
    int ret;
    int listen_d = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = -1; 
    if (setsockopt(listen_d, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int)) ==  -1) {
        cclog(ERROR, "setsockopt error!\n");
        return -1;
    }

    struct sockaddr_in name;
    name.sin_family = PF_INET;
    name.sin_port = (in_port_t)htons(local_port);
    int c = bind(listen_d, (struct sockaddr *)&name, sizeof(name));

    ret = listen(listen_d, TCP_LISTEN_LENGTH);
    if (ret < 0) {
        cclog(ERROR, "Tcp listen error, ret=[%d]\n", ret);
        return ret;
    }

    struct sockaddr_storage client_addr;  // 保存客户端详细信息
    unsigned int address_size = sizeof(client_addr);

    int connect_d = 0;

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
            Client* tc = malloc(sizeof(Client));
            tc->sock = connect_d;
            if ((ret = pthread_create(&tid, &thread_attr, clientHandler, (void*)tc))) {
                cclog(ERROR, "pthread_create error, ret=[%d]\n", ret);
                break;
            }
        } else {
            // // no more new connection if room is full.
            // fprintf(stdout, "Client from [%s:%d] is blocked, room is full [%d / %d]!\n", 
            //             inet_ntoa(((struct sockaddr*)&client_addr)->sin_addr), 
            //             ntohs(((struct sockaddr*)&client_addr)->sin_port), 
            //             clientNum,
            //             MAX_CLIENT_NUM);

            ret = sendMsg(connect_d, RESP0001, strlen(RESP0001));
            if (ret < 0) {
                cclog(ERROR, "sendMsg error!\n");
            }
        }
    }

    pthread_attr_destroy(&thread_attr);
    clearClient();

    return 0;
}