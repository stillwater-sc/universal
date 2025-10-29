// console_utf8.hpp
#pragma once

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN  // Also reduces other pollution
#include <windows.h>
#endif

class ConsoleUTF8 {
public:
    ConsoleUTF8() {
#ifdef _WIN32
        old_cp_ = GetConsoleOutputCP();
        SetConsoleOutputCP(CP_UTF8);
        // Enable ANSI escape sequences if you use them
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hConsole, &mode);
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
    }
    
    ~ConsoleUTF8() {
#ifdef _WIN32
        SetConsoleOutputCP(old_cp_);
#endif
    }
    
    // Delete copy/move to avoid double-restore
    ConsoleUTF8(const ConsoleUTF8&) = delete;
    ConsoleUTF8& operator=(const ConsoleUTF8&) = delete;

private:
#ifdef _WIN32
    UINT old_cp_;
#endif
};
