#include "CChat.h"

int sendMsg(int sock, char* msg, int msgLen)
{
    if (msg == NULL || sock < 0 || msgLen > TCP_BUF_SIZE - 2) return -1;

    int c = 0, bytesToSend = 0;
    char* buffer = malloc(TCP_BUF_SIZE);
    char* bp = buffer;

    memcpy(buffer + 2, msg, msgLen);

    buffer[0] = (char)(msgLen / 256);
    buffer[1] = (char)(msgLen % 256);

    bytesToSend = msgLen + 2;

    cclog(DEBUG, "buffer[0]=%d buffer[1]=%d\n", buffer[0], (unsigned char)buffer[1]);

    do {
        c = send(sock, bp, bytesToSend, 0); 
        if (c < 0) {
            cclog(ERROR, "Tcp send error, ret=[%d]\n", c);
            free(buffer);
            return -1;
        }
        bp += c;
    } while((bytesToSend -= c) > 0);

    cclog(NORMAL, "Tcp send ok, send[%d][%s]\n", msgLen, msg);
    free(buffer);
    return 0;    
}

int recvMsg(int sock, char* recvBuf, int recvBufLen)
{
    int c = 0, bufSpaceLeft = recvBufLen;
    unsigned int byteToRecv = 0;
    char* bs = recvBuf;

    char slen[2] = {0};

    //memset(recvBuf, 0, recvBufLen);

    c = recv(sock, slen, 2, 0);
    if (c < 0) {
        if (errno > 0 && errno != EAGAIN) {
            cclog(ERROR, "Recv msg len failed, ret=[%d]\n", c);
        }
        return -1;
    } else if (c == 0) {
        cclog(NORMAL, "Tcp close.\n");
        return -2;
    }

    byteToRecv = slen[0] * 256 + (unsigned char)slen[1];

    cclog(DEBUG, "byteToRecv = [%d]\n", byteToRecv);

    if (byteToRecv > recvBufLen) {
        cclog(ERROR, "recv buf too small, must bigger than %d\n", byteToRecv);
        return -3;
    }

    do {
        c = recv(sock, bs, byteToRecv, 0);
        if (c < 0) {
            if (errno > 0) {
                cclog(ERROR, "Tcp recv error, ret=[%d]\n", c);
            }
            return -1;
        } else if (c == 0) {
            cclog(NORMAL, "Tcp close.\n");
            return -2;
        }
        bs += c;
        cclog(NORMAL, "recv %d byte [%s]\n", c, bs);
    } while((byteToRecv -= c) > 0);

    cclog(NORMAL, "Tcp recv ok. buffer[%s]\n", recvBuf);

    return 0;
}