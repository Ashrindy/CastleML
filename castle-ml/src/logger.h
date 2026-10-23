#pragma once
#include <cstdio>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

static HANDLE stdoutHandle = nullptr;
static HANDLE GetStdoutHandle() {
    if (!stdoutHandle) {
        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    return stdoutHandle;
}

static void info(const char* format, va_list args) {
    printf("[CML] [INFO]  ");
    vprintf(format, args);
    printf("\n");
}

static void info(const char* format, ...) {
    va_list list;
    va_start(list, format);
    info(format, list);
    va_end(list);
}

static void error(const char* format, va_list args) {
    SetConsoleTextAttribute(GetStdoutHandle(), FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("[CML] [ERROR]  ");
    vprintf(format, args);
    printf("\n");
    SetConsoleTextAttribute(GetStdoutHandle(), FOREGROUND_WHITE);
}

static void error(const char* format, ...) {
    va_list list;
    va_start(list, format);
    error(format, list);
    va_end(list);
}
