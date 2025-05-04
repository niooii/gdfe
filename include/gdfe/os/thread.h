#pragma once
#include <gdfe/prelude.h>

typedef struct GDF_Thread_T*    GDF_Thread;
typedef struct GDF_Mutex_T*     GDF_Mutex;
typedef struct GDF_Semaphore_T* GDF_Semaphore;
// https://learn.microsoft.com/en-us/windows/win32/sync/semaphore-objects

EXTERN_C_BEGIN

// Run the thread_fn on a separate thread of execution. This does NOT make a copy of the arguments
// passed in.
GDF_Thread GDF_CreateThread(unsigned long thread_fn(void*), void* args);
void       GDF_DestroyThread(GDF_Thread thread);

u32 GDF_GetCurrentThreadId();

void GDF_JoinThread(GDF_Thread thread);

void GDF_ThreadSleep(u64 ms);

// Lock implementation
GDF_Mutex GDF_CreateMutex();

// TODO! Check return value for poisoned mutex, if it returns WAIT_ABANDONED
GDF_BOOL GDF_LockMutex(GDF_Mutex mutex);

GDF_BOOL GDF_ReleaseMutex(GDF_Mutex mutex);

void GDF_DestroyMutex(GDF_Mutex mutex);

// Technically a binary semaphore, used for signaling the completion of code
// across different threads/processes.
// If name is not NULL, then a globally scoped semaphore is created
GDF_Semaphore GDF_CreateSemaphore(const char* name);

// Returns the global semaphore with the provided name, or NULL
// if it cannot be acquired for any reason
GDF_Semaphore GDF_GetSemaphore(const char* name);

// When this function returns, the semaphore will be reset to the unsignaled state.
GDF_BOOL GDF_WaitSemaphore(GDF_Semaphore semaphore);

// Returns GDF_TRUE if the signal was successfully consumed,
// or GDF_FALSE if there was no signal.
// Effectively a non blocking poll on the semaphore
GDF_BOOL GDF_ConsumeSemaphoreSignal(GDF_Semaphore semaphore);

// When called, any thread currently waiting on the semaphore will continue
// execution. If no thread is waiting, the next thread that calls WaitSemaphore
// will not be blocked, and the semaphore will be reset to the unsignaled state.
GDF_BOOL GDF_SignalSemaphore(GDF_Semaphore semaphore);

EXTERN_C_END
