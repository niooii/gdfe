#include <gdfe/os/thread.h>
#include <stdio.h>

#ifdef OS_LINUX
    #include <fcntl.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <string.h>
    #include <unistd.h>

typedef struct GDF_Mutex_T {
    pthread_mutex_t mutex;
} GDF_Mutex_T;

typedef struct GDF_Thread_T {
    pthread_t thread;
    u32       id;
} GDF_Thread_T;

typedef struct GDF_Semaphore_T {
    sem_t*      sem;
    const char* name;
    GDF_BOOL    is_named;
} GDF_Semaphore_T;

GDF_Thread GDF_CreateThread(unsigned long thread_fn(void*), void* args)
{
    GDF_Thread thread = GDF_Malloc(sizeof(GDF_Thread_T), GDF_MEMTAG_APPLICATION);

    int result = pthread_create(&thread->thread, NULL, (void* (*)(void*))thread_fn, args);
    if (result != 0)
    {
        GDF_Free(thread);
        return NULL;
    }

    thread->id = (u32)thread->thread;
    return thread;
}

void GDF_DestroyThread(GDF_Thread thread)
{
    if (thread)
    {
        pthread_detach(thread->thread);
        GDF_Free(thread);
    }
}

u32 GDF_GetCurrentThreadId() { return (u32)pthread_self(); }

void GDF_JoinThread(GDF_Thread thread) { pthread_join(thread->thread, NULL); }

void GDF_ThreadSleep(u64 ms) { usleep(ms * 1000); }

GDF_Mutex GDF_CreateMutex()
{
    GDF_Mutex mutex = GDF_Malloc(sizeof(GDF_Mutex_T), GDF_MEMTAG_APPLICATION);

    if (pthread_mutex_init(&mutex->mutex, NULL) != 0)
    {
        GDF_Free(mutex);
        return NULL;
    }

    return mutex;
}

GDF_BOOL GDF_LockMutex(GDF_Mutex mutex) { return pthread_mutex_lock(&mutex->mutex) == 0; }

GDF_BOOL GDF_ReleaseMutex(GDF_Mutex mutex) { return pthread_mutex_unlock(&mutex->mutex) == 0; }

void GDF_DestroyMutex(GDF_Mutex mutex)
{
    if (mutex)
    {
        pthread_mutex_destroy(&mutex->mutex);
        GDF_Free(mutex);
    }
}

GDF_Semaphore GDF_CreateSemaphore(const char* name)
{
    GDF_Semaphore sm = GDF_Malloc(sizeof(GDF_Semaphore_T), GDF_MEMTAG_APPLICATION);

    if (name)
    {
        // named semaphore
        char namebuf[256];
        snprintf(namebuf, sizeof(namebuf), "/%s", name);

        sm->sem = sem_open(namebuf, O_CREAT, 0644, 0);
        if (sm->sem == SEM_FAILED)
        {
            GDF_Free(sm);
            return NULL;
        }
        sm->is_named = GDF_TRUE;
        sm->name     = name;
    }
    else
    {
        // snnamed semaphore
        sm->sem = GDF_Malloc(sizeof(sem_t), GDF_MEMTAG_APPLICATION);
        if (sem_init(sm->sem, 0, 0) != 0)
        {
            GDF_Free(sm->sem);
            GDF_Free(sm);
            return NULL;
        }
        sm->is_named = GDF_FALSE;
        sm->name     = NULL;
    }

    return sm;
}

GDF_Semaphore GDF_GetSemaphore(const char* name)
{
    if (!name)
        return NULL;

    char namebuf[256];
    snprintf(namebuf, sizeof(namebuf), "/%s", name);

    sem_t* sem = sem_open(namebuf, 0);
    if (sem == SEM_FAILED)
        return NULL;

    GDF_Semaphore sm = GDF_Malloc(sizeof(GDF_Semaphore_T), GDF_MEMTAG_APPLICATION);
    sm->sem          = sem;
    sm->name         = name;
    sm->is_named     = GDF_TRUE;

    return sm;
}

GDF_BOOL GDF_WaitSemaphore(GDF_Semaphore semaphore) { return sem_wait(semaphore->sem) == 0; }

GDF_BOOL GDF_ConsumeSemaphoreSignal(GDF_Semaphore semaphore)
{
    return sem_trywait(semaphore->sem) == 0;
}

GDF_BOOL GDF_SignalSemaphore(GDF_Semaphore semaphore) { return sem_post(semaphore->sem) == 0; }

void GDF_DestroySemaphore(GDF_Semaphore semaphore)
{
    if (semaphore)
    {
        if (semaphore->is_named)
            sem_close(semaphore->sem);
        else
        {
            sem_destroy(semaphore->sem);
            GDF_Free(semaphore->sem);
        }
        GDF_Free(semaphore);
    }
}

#endif
