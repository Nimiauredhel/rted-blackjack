#ifndef DELAY_H
#define DELAY_H

    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define _GNU_SOURCE
    #endif
    #if defined(_WIN32) || defined(_WIN64)
    #endif

#include <stdint.h>
#include <time.h>

// waits for specified number of milliseconds
void delay_ms(uint32_t ms);

#endif
