#include "delay.h"

void delay_ms(uint32_t ms)
{
    if (ms <= 0) return;

    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        uint32_t s = ms / 1000;
        ms = ms % 1000;
        struct timespec req = { s, ms * 1000000 };
        struct timespec rem = { s, ms * 1000000 };
        nanosleep(&req, &rem);
    #endif
    #if defined(_WIN32) || defined(_WIN64)
        clock_t start_time = clock();
        while (clock() < start_time + ms);
    #endif
}
