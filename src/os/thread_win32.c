#include <stdio.h>
#include <gdfe/os/thread.h>

#ifdef OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct GDF_Mutex_T {
    HANDLE mutex_handle;
} GDF_Mutex_T;

typedef struct GDF_Thread_T {
    HANDLE thread_handle;
    u32 id;
} GDF_Thread_T;

typedef struct GDF_Semaphore_T {
    HANDLE sm_handle;
    const char* name;
} GDF_Semaphore_T;

GDF_Thread GDF_CreateThread(unsigned long thread_fn(void*), void* args) {
    GDF_Thread thread = GDF_Malloc(sizeof(GDF_Thread_T), GDF_MEMTAG_APPLICATION);
    u32 creation_flags = 0;
    thread->thread_handle = CreateThread(
        NULL,
        0,
        thread_fn,
        args,
        creation_flags,
        &thread->id
    );

    return thread;
}

void GDF_DestroyThread(GDF_Thread thread) {
    GDF_Free(thread);
}

u32 GDF_GetCurrentThreadId()
{
    return GetCurrentThreadId();
}

void GDF_JoinThread(GDF_Thread thread)
{
    WaitForSingleObject(thread->thread_handle, INFINITE);
}

void GDF_ThreadSleep(u64 ms)
{
    Sleep(ms);
}

GDF_Mutex GDF_CreateMutex()
{
    HANDLE handle = CreateMutex(NULL, FALSE, NULL);
    GDF_Mutex mutex = GDF_Malloc(sizeof(GDF_Mutex_T), GDF_MEMTAG_APPLICATION);
    mutex->mutex_handle = handle;

    return mutex;
}

GDF_BOOL GDF_LockMutex(GDF_Mutex mutex)
{
    WaitForSingleObject(mutex->mutex_handle, INFINITE);
    return GDF_TRUE;
}

GDF_BOOL GDF_ReleaseMutex(GDF_Mutex mutex)
{
    ReleaseMutex(mutex->mutex_handle);
    return GDF_TRUE;
}

void GDF_DestroyMutex(GDF_Mutex mutex)
{
    CloseHandle(mutex->mutex_handle);
    GDF_Free(mutex);
}

static const char* GLOBAL_NAMESPACE_PREFIX = "Global\\";
GDF_Semaphore GDF_CreateSemaphore(const char* name)
{
    char namebuf[strlen(name) + sizeof(GLOBAL_NAMESPACE_PREFIX) + 1];
    GDF_Memzero(namebuf, sizeof(namebuf));

    if (name)
        snprintf(namebuf, sizeof(namebuf), "%s%s", GLOBAL_NAMESPACE_PREFIX, name);

    HANDLE handle = CreateSemaphore(NULL, 0, 1, namebuf);
    if (handle == NULL)
        return NULL;

    GDF_Semaphore sm = GDF_Malloc(sizeof(GDF_Semaphore_T), GDF_MEMTAG_APPLICATION);
    sm->sm_handle = handle;
    sm->name = name;

    return sm;
}

GDF_Semaphore GDF_GetSemaphore(const char* name)
{
    if (!name)
        return NULL;

    char namebuf[strlen(name) + sizeof(GLOBAL_NAMESPACE_PREFIX) + 1];
    GDF_Memzero(namebuf, sizeof(namebuf));

    snprintf(namebuf, sizeof(namebuf), "%s%s", GLOBAL_NAMESPACE_PREFIX, name);

    HANDLE handle = OpenSemaphore(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        namebuf
    );

    if (handle == NULL)
        return NULL;

    GDF_Semaphore sm = GDF_Malloc(sizeof(GDF_Semaphore_T), GDF_MEMTAG_APPLICATION);
    sm->sm_handle = handle;
    sm->name = name;

    return sm;
}

GDF_BOOL GDF_WaitSemaphore(GDF_Semaphore semaphore)
{
    return WaitForSingleObject(semaphore->sm_handle, INFINITE) == WAIT_OBJECT_0;
}

GDF_BOOL GDF_ConsumeSemaphoreSignal(GDF_Semaphore semaphore)
{
    return WaitForSingleObject(semaphore->sm_handle, 0) == WAIT_OBJECT_0;
}

GDF_BOOL GDF_SignalSemaphore(GDF_Semaphore semaphore)
{
    return ReleaseSemaphore(semaphore->sm_handle, 1, NULL) != FALSE;
}

#endif