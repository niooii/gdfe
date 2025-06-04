#include <gdfe/os/misc.h>

#ifdef OS_LINUX
#define _GNU_SOURCE
#define _DEFAULT_SOURCE 1
#define _POSIX_C_SOURCE 200809L
#include <time.h>

static struct timespec start_time;

void gdfe_misc_init()
{
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

void gdfe_misc_shutdown() {}

f64 GDF_GetAbsoluteTime()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    f64 elapsed_sec = now.tv_sec - start_time.tv_sec;
    f64 elapsed_nsec = now.tv_nsec - start_time.tv_nsec;

    return elapsed_sec + elapsed_nsec * 1e-9;
}

void GDF_GetSystemTime(GDF_DateTime* datetime)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm *local_tm = localtime(&ts.tv_sec);

    datetime->year = local_tm->tm_year + 1900;
    datetime->month = local_tm->tm_mon + 1;
    datetime->day_of_week = local_tm->tm_wday;
    datetime->day = local_tm->tm_mday;
    datetime->hour = local_tm->tm_hour;
    datetime->minute = local_tm->tm_min;
    datetime->second = local_tm->tm_sec;
    datetime->milli = ts.tv_nsec / 1000000;
}

#endif
