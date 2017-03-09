#include "cchat.h"

int mode;
int isRunning = 1;

void on_cc_exit(char* msg)
{
    if (msg == NULL) {
        fprintf(stdout, "Bye bye ~\n");
    } else {
        fprintf(stdout, "%s\n", msg);
    }
    
    isRunning = 0;
}

void handle_exit(int e)
{
    on_cc_exit("Terminated by user.");
}

int catch_signal(int sig, void (* handler)(int))
{
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction(sig, &action, NULL);
}

int init()
{
    int ret; 

    ret = catch_signal(SIGTERM, handle_exit);
    if (ret) {
        cclog(ERROR, "catch_signal for SIGTERM failed, ret=[%d]\n", ret);
        return -1;
    }

    ret = catch_signal(SIGINT, handle_exit);
    if (ret) {
        cclog(ERROR, "catch_signal for SIGTERM failed, ret=[%d]\n", ret);
        return -1;
    }
    
    return 0;
}

int main(int argc, char* argv[])
{
    char addr[50] = {0};
    char port[6] = {0};
    char ch;

    int ret = 0;

    while((ch = getopt(argc, argv, "csh:p:")) != EOF) {
        switch(ch) {
            case 'c':
                mode = 0;
                break;

            case 's':
                mode = 1;
                break;

            case 'h':
                if (strlen(optarg) > sizeof(addr)) {
                    on_cc_exit("Address too long");
                    return -1;
                } else {
                    strcpy(addr, optarg);
                }
                break;
            case 'p':
                if (strlen(optarg) > sizeof(port)) {
                    on_cc_exit("Port too long");
                    return -1;
                } else {
                    strcpy(port, optarg);
                }
                break;
        }
    }

    if (strlen(port) == 0 || (mode != 1 && strlen(addr) == 0)) {
        cclog(ERROR, "Input error\n");
        on_cc_exit("Input error");
        return -1;
    }

    ret = init();
    if (ret) {
        fprintf(stdout, "init() error, ret=[%d]\n", ret);
        return ret;
    }

    cclog(DEBUG, "mode=[%i] host[%s] port[%s]\n", mode, addr, port);

    if (mode) {
        runServer(atoi(port));
    } else {
        runClient(addr, atoi(port));
    }

    on_cc_exit(NULL);
    return 0;
}
