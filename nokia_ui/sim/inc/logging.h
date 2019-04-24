#ifndef _LOGGING_H_
#define _LOGGING_H_

#ifdef SIM_SOCKET

    #include <stdio.h>

    #define TRACE_MIN       0
    #define TRACE_ERROR     1
    #define TRACE_INFO      2
    #define TRACE_DEBUG     3
    #define TRACE_MAX       TRACE_DEBUG

    #ifndef TRACE_LEVEL
    #define TRACE_LEVEL TRACE_MAX
    #endif

    #if (TRACE_LEVEL <= 0)
    #define MSG(...)   (void)0
    #else
    #define MSG(...)   printf(__VA_ARGS__)
    #endif

    #if (TRACE_LEVEL < TRACE_ERROR)
    #define EMSG(...)   (void)0
    #else
    #define EMSG(...)   printf(__VA_ARGS__)
    #endif

    #if (TRACE_LEVEL < TRACE_INFO)
    #define IMSG(...)   (void)0
    #else
    #define IMSG(...)   printf(__VA_ARGS__)
    #endif

    #if (TRACE_LEVEL < TRACE_DEBUG)
    #define DMSG(...)   (void)0
    #else
    #define DMSG(...)   printf(__VA_ARGS__)
    #endif

#elif defined(SIM_TRUSTZONE)

    #include <trace.h>

#endif

#endif
