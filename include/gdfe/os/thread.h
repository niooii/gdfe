#pragma once
#include <gdfe/core.h>

typedef struct GDF_Thread_T* GDF_Thread;
typedef struct GDF_Mutex_T* GDF_Mutex;
typedef struct GDF_Semaphore_T* GDF_Semaphore;
// https://learn.microsoft.com/en-us/windows/win32/sync/semaphore-objects

#ifdef __cplusplus
extern "C" {
#endif

// Run the thread_fn on a separate thread of execution. This does NOT make a copy of the arguments passed in. 
GDF_Thread GDF_CreateThread(unsigned long thread_fn(void*), void* args);
void GDF_DestroyThread(GDF_Thread thread);
u32 GDF_GetCurrentThreadId();
void GDF_JoinThread(GDF_Thread thread);
void GDF_ThreadSleep(u64 ms);

// Lock implementation
GDF_Mutex GDF_CreateMutex();
// TODO! Check return value for poisoned mutex, if it returns WAIT_ABANDONED
GDF_BOOL GDF_LockMutex(GDF_Mutex mutex);
GDF_BOOL GDF_ReleaseMutex(GDF_Mutex mutex);
 
// Technically a binary semaphore, used for signaling the completion of code
// across different threads.
GDF_Semaphore GDF_CreateSemaphore();
// When this function returns, the semaphore will be reset to the unsignaled state.
GDF_BOOL GDF_WaitSemaphore(GDF_Semaphore semaphore);
// When called, any thread currently waiting on the semaphore will continue
// execution. If no thread is waiting, the next thread that calls WaitSemaphore
// will not be blocked, and the semaphore will be reset to the unsignaled state.
GDF_BOOL GDF_SignalSemaphore(GDF_Semaphore semaphore);

#ifdef __cplusplus
}
#endif