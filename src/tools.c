#include "cchat.h"
#include <stdarg.h>

FILE* logFile = NULL;
pthread_mutex_t f_lock = PTHREAD_MUTEX_INITIALIZER;

extern int mode;

int cclog(int level, char* filename, int line, char* msg, ...)
{
    char logMsg[4096] = {0};
    char logFileName[100] = {0};
    struct tm *tm_ptr;
    time_t cur_time;

    char tmp[10];
    
    int ret = 0;

    if (mode) {
        strcpy(tmp, "server");
    } else {
        strcpy(tmp, "client");
    }

    va_list va;
    va_start(va, msg);

    pthread_mutex_lock(&f_lock);

    time(&cur_time);
    tm_ptr = localtime(&cur_time);

    sprintf(logFileName, "%s/%s.%s.%d%02d%02d.log", LOG_FILE_PATH, LOG_FILE_NAME, tmp, tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday);

    logFile = fopen(logFileName, "a+");
    if (logFile == NULL) {
        fprintf(stderr, "fopen %s error!\n", logFileName);
        return -1;
    } 

    vsprintf(logMsg, msg, va);  // do not use sprintf or printf with va, strange thing happens.
    fprintf(logFile, "%d%02d%02d%2d%2d%2d {%s}{%i} - %s", tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec, filename, line, logMsg);

    if (fclose(logFile)) {
        fprintf(stderr, "fclose error, ret=[%d]\n", ret);
        return -1;
    }
    logFile = NULL;
    
    pthread_mutex_unlock(&f_lock);

    va_end(va);

    return 0;
}