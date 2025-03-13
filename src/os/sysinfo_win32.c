#include <gdfe/os/sysinfo.h>

#ifdef OS_WINDOWS
#include <Windows.h>

static f64 clock_freq;
static LARGE_INTEGER start_time;

GDF_BOOL GDF_InitSysinfo()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    clock_freq = 1.0 / (f64)freq.QuadPart;
    QueryPerformanceCounter(&start_time);
    return GDF_TRUE;
}

f64 GDF_GetAbsoluteTime()
{
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_freq;
}

void GDF_GetSystemTime(GDF_DateTime* datetime)
{
    SYSTEMTIME time;
    GetLocalTime(&time);
    datetime->year = time.wYear; 
    datetime->month = time.wMonth; 
    datetime->day_of_week = time.wDayOfWeek; 
    datetime->day = time.wDay; 
    datetime->hour = time.wHour;
    datetime->minute = time.wMinute; 
    datetime->second = time.wSecond; 
    datetime->milli = time.wMilliseconds; 
}

void GDF_GetDisplayInfo(GDF_DisplayInfo* display_info)
{
    display_info->screen_height = GetSystemMetrics(SM_CYSCREEN);
    display_info->screen_width= GetSystemMetrics(SM_CXSCREEN);
}

#endif