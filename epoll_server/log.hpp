

#ifndef __YUFC_LOG__
#define __YUFC_LOG__

// 日志是有日志级别的
// 不同的级别代表不同的响应方式
#define DEBUG 0
#define NORMAL 1 // 正常的
#define WARNING 2 // 警告
#define ERROR 3 // 错误
#define FATAL 4 // 致命的

#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <time.h>

// 完整的日志功能
// 至少：日志等级、时间、内容、支持用户自定义

const char* gLevelMap[] = {
    "DEBUG",
    "NORMAL",
    "WARNING",
    "ERROR",
    "FATAL"
};

#define __NORMAL_LOG__ "./WebServer.log"
#define __REQUEST_LOG__ "./Requests.log"
#define __LOGBEGIN__ 1
#define __LOGEND__ 0

void InitLogFile(int option) {
    FILE* fp = fopen(__NORMAL_LOG__, "a");
    FILE* fp_r = fopen(__REQUEST_LOG__, "a");
    if (option == __LOGBEGIN__) {
        auto start_time = std::chrono::system_clock::now();
        std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_time);
        std::string prompt = "Process started at: ";
        std::string timeLine = std::ctime(&start_time_t);
        std::string logLine = prompt + timeLine;
        fprintf(fp, "%s\n", logLine.c_str());
        fprintf(fp_r, "%s\n", logLine.c_str());
    } else if (option == __LOGEND__) {
        fprintf(fp, "\n\n");
        fprintf(fp_r, "\n\n");
    } else
        assert(false);
    fflush(fp);
    fflush(fp_r);
    fclose(fp);
    fclose(fp_r);
}

void logMessage(int level, const char* format, ...) {
#ifdef __DEBUG_SHOW
    if (level == DEBUG)
        return;
#else
    // va_list ap; //va_list -> char*
    // va_start(ap, format);
    // while()
    // int x = va_arg(ap, int);
    // va_end(ap);
    char stdBuffer[1024]; // 标准部分
    time_t timestamp = time(nullptr);
    // struct tm *tmlocaltime = localtime(&timestamp);
    // ...

    snprintf(stdBuffer, sizeof stdBuffer, "[%s] [%ld] ", gLevelMap[level], timestamp);

    char logBuffer[1024]; // 自定义部分
    va_list args;
    va_start(args, format); // 初始化一下
    // vprintf(format, args); //格式化到stdout中
    vsnprintf(logBuffer, sizeof logBuffer, format, args); // 可以直接格式化到字符串中
    // 这样v开头的函数有4个，要用的话可以man一下
    // printf("%s", logBuffer);
    va_end(args);

#endif

    FILE* fp = fopen(__NORMAL_LOG__, "a");
    fprintf(stdout, "%s%s\n", stdBuffer, logBuffer);
    fflush(stdout);
    fprintf(fp, "%s%s\n", stdBuffer, logBuffer);
    fflush(fp);
    fclose(fp);
}

void logRequest(const char* request) {
    FILE* fp = fopen(__REQUEST_LOG__, "a");
    fprintf(fp, "%s", request);
    fflush(fp);
    fclose(fp);
}

#endif