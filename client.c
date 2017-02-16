#include "CChat.h"
#include <curses.h>

#include "list.h"

#define MAX_LINE_NUM 300

extern int isRunning;

char clientName[20] = {0};
List lines;
int screenTopIndex = 0;
int screenBottomIndex = 0;

int printChats()
{
    int y = 0;
    char *line = NULL;

    if (lines.size > 0) {
        for (int i = screenTopIndex; i <= screenBottomIndex; i++) {
            move(y, 0);
            line = listGet(&lines, i);
            if (!line) {
                cclog(ERROR, "line %d is NULL!\n");
                return -1;
            }
            printw("%s", (char *)line);
            y++;
        }        
    }

    return 0;
}

void updateTerm()
{
    clear();

    printChats();

    move(LINES - 1, 0);
    printw("%s", "YOU:");
    refresh();
}

int term_getChat(char* buffer, int bSize)
{
    int curPos = 1, inputLen = 0;
    int key;
    int y, x;
    while(inputLen < bSize) {
        key = getch();
        cclog(DEBUG, "key=[%d] pressed\n", key);
        if (key == ERR) {
            break;
        } else {
            if (key >= 32 && key <= 126) {
                if (curPos > inputLen) {
                    buffer[inputLen++] = key;
                    addch(key);    
                    curPos++;
                } else {
                    for (int i = ++inputLen; i > curPos; i--) {
                        buffer[i] = buffer[i - 1];
                    }
                    buffer[curPos++] = key;
                    insch(key);
                }
            } else {
                if (key == KEY_ENTER || key == 10) {
                    cclog(DEBUG, "KEY_ENTER pressed!\n");
                    cclog(DEBUG, "inputLen=[%d] buffer=[%s]\n", inputLen, buffer);
                    refresh();
                    updateTerm();
                    break;
                } else if (key == KEY_BACKSPACE || key == 127) {
                    cclog(DEBUG, "KEY_BACKSPACE pressed!\n");
                    if (curPos > 1) {
                        for (int i = curPos - 2; i < inputLen - 1; i++) {
                            buffer[i] = buffer[i + 1]; 
                        }
                        buffer[--inputLen] = '\0';
                        curPos--;
                        getyx(stdscr, y, x);
                        move(y, x - 1);
                        delch();
                        refresh();
                    }
                } else if (key == KEY_LEFT || key == 260) {
                    cclog(DEBUG, "KEY_LEFT pressed!\n");
                    if (curPos > 1) {
                        curPos--;
                        getyx(stdscr, y, x);
                        move(y, x - 1);
                        refresh();
                    }
                } else if (key == KEY_RIGHT || key == 261) {
                    cclog(DEBUG, "KEY_RIGHT pressed!\n");
                    if (curPos <= inputLen) {
                        curPos++;
                        getyx(stdscr, y, x);
                        move(y, x + 1);
                        refresh();                        
                    }
                }        
            }
        }
    }

    return inputLen;
}

int term_addChat(char* chat, int cLen)
{
    if (chat == NULL || cLen == 0) return -1;

    int lineCount = 0;
    char *pLine = NULL;
    char *pCur  = chat;

    lineCount = (cLen / COLS) + (cLen % COLS > 0 ? 1 : 0);
    cclog(DEBUG, "lineCount=[%d]\n", lineCount);
    while(lineCount > 0) {
        pLine = malloc(COLS + 1);

        if (lineCount == 1) {
            memcpy(pLine, pCur, strlen(pCur));
        } else {
            memcpy(pLine, pCur, COLS);
            pCur += COLS;
        }
        
        listAdd(&lines, pLine);
        if (lines.size > MAX_LINE_NUM) {
            cclog(DEBUG, "line.size > MAX_LINE_NUM\n");
            listDelHead(&lines);
        }

        cclog(DEBUG, "lines.size=[%d]\n", lines.size);
        screenBottomIndex = lines.size - 1;
        if (lines.size > LINES - 1) {
            screenTopIndex = screenBottomIndex - (LINES - 2);
        } else {
            screenTopIndex = 0;
        }

        cclog(DEBUG, "screenTopIndex=[%d]\n", screenTopIndex);
        cclog(DEBUG, "screenBottomIndex=[%d]\n", screenBottomIndex);

        lineCount--;
    }

    updateTerm();
    return 0;
}

int runClient(char* remote_addr, int remote_port)
{
    puts("Chat client is running.");
    
    int ret = 0;
    char sport[6] = {0};
    char ch;
    int  ic;
    sprintf(sport, "%d", remote_port);
    char* sendBuf = malloc(TCP_BUF_SIZE);
    char* recvBuf = malloc(TCP_BUF_SIZE);

    struct addrinfo *info = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    ret = getaddrinfo(remote_addr, sport, &hints, &info);
    if (ret) {
        cclog(ERROR, "getaddrinfo error:%s", gai_strerror(ret));
        return -1;  
    }

    int sock = socket(info->ai_family, info->ai_socktype, 0);
    if (sock < 0) {
        cclog(ERROR, "socket create error [%s]\n", strerror(sock));
        return -1;  
    }
    
    ret = connect(sock, info->ai_addr, info->ai_addrlen);
    if (ret) {
        cclog(ERROR, "socket connect error [%d][%s]\n", ret, strerror(ret));
        return -1;
    }

    int done = FALSE;
    char cliName[200] = {0};
    fprintf(stdout, "Please enter you name:");
    while (!done && isRunning) {
        // Very dangerous indeed. fix it later
        memset(cliName, 0, sizeof(cliName));
        if (fscanf(stdin, "%s", cliName) != EOF) {
            if ((ret = sendMsg(sock, cliName, strlen(cliName)))) {
                cclog(ERROR, "sendMsg error, ret=[%d]\n", ret);
                return -2;
            } else {
                cclog(NORMAL, "Send client name OK\n");

                if ((ret = recvMsg(sock, recvBuf, TCP_BUF_SIZE))) {
                    cclog(ERROR, "Recv error\n");
                    return -2;
                } else {
                    cclog(NORMAL, "Recv ok\n");

                    if (memcmp(recvBuf, RESP0002, strlen(RESP0001)) == 0) {
                        fprintf(stdout, "Name [%s] is already be used. Please try another one:", cliName);
                    } else {
                        done = TRUE;
                    }
                }
            }
        } else {
            return -2;
        }
    }

    cclog(DEBUG, "Waiting for client or internet \n");

    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    updateTerm();
    timeout(300000000000);

    cclog(NORMAL, "Screen [%d %d]\n", LINES, COLS);

    fd_set rSet;
    struct timeval tv;
    int sRet = 0;
    int maxFd = 0;

    while(isRunning) {
        FD_ZERO(&rSet);

        FD_SET(STDIN_FILENO, &rSet);
        FD_SET(sock, &rSet);

        maxFd = (STDIN_FILENO > sock ? STDIN_FILENO : sock) + 1;            
        sRet = select(maxFd, &rSet, NULL, NULL, &tv);
        if (sRet == -1) {
            cclog(ERROR, "select error, errno=[%d][%s]\n", errno, strerror(errno));
            break;
        } else if (sRet == 0) {
            continue;
        } else {
            if (FD_ISSET(STDIN_FILENO, &rSet)) {
                ret = term_getChat(sendBuf, TCP_BUF_SIZE);
                if (ret <= 0) {
                    cclog(ERROR, "term_getChat error, ret=[%d]\n", ret);
                    break;
                } else {
                    if ((ret = sendMsg(sock, sendBuf, strlen(sendBuf)))) {
                        cclog(ERROR, "sendMsg error, ret=[%d]\n", ret);
                        break;
                    } else {
                        cclog(NORMAL, "send ok\n");
                    }
                }
            } 

            if (FD_ISSET(sock, &rSet)) {
                if ((ret = recvMsg(sock, recvBuf, TCP_BUF_SIZE))) {
                    cclog(ERROR, "recvMsg error\n");
                    break;
                } else {
                    cclog(NORMAL, "recvMsg ok\n");

                    if (memcmp(recvBuf, RESP0001, strlen(RESP0001)) == 0) {
                        fprintf(stdout, "Room is full, please try again later.");
                        break;
                    }

                    term_addChat(recvBuf, strlen(recvBuf));
                }
            }            
        }
        memset(sendBuf, 0, TCP_BUF_SIZE);
        memset(recvBuf, 0, TCP_BUF_SIZE);
    }

    free(sendBuf);
    free(recvBuf);
    listClear(&lines);

    endwin();
    
    return 0;
}