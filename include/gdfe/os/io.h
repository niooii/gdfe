#pragma once

#include <gdfe/def.h>

#define MB_BYTES 1048576
#define KB_BYTES 1024

#define MAX_PATH_LEN 512

typedef enum GDF_IO_RESULT {
    /// A generic failure.
    GDF_IO_RESULT_FAIL = 0,
    GDF_IO_RESULT_BAD_HANDLE = 0,
    GDF_IO_RESULT_SUCCESS = 1,

    GDF_IO_RESULT_ACCESS_DENIED,

    GDF_IO_RESULT_ALREADY_EXISTS,
    GDF_IO_RESULT_PATH_NOT_FOUND,
    GDF_IO_RESULT_PROC_WAIT_TIMEOUT,
} GDF_IO_RESULT;

typedef struct GDF_DirInfoNode {
    char* name;
    char* full_path;
    GDF_BOOL is_directory;
} GDF_DirInfoNode;

typedef struct GDF_DirInfo {
    GDF_DirInfoNode** nodes;
    size_t num_nodes;
} GDF_DirInfo;

typedef struct GDF_Process_T *GDF_Process;

EXTERN_C_BEGIN

void GDF_ShowConsole();

void GDF_HideConsole();

const char* GDF_GetExecutablePath();

void GDF_WriteConsole(const char* msg);

void GDF_GetAbsolutePath(const char* rel_path, char* out_buf);

// gets path of resource in a resources/ directory
// ex: resources/icon.ico can be accessed by GDF_GetResourcePath("icon.ico", buf)
void GDF_GetResourcePath(const char* rel_path, char* out_buf);

// if the path is outside the directory of the executable
// out_buf will be set to NULL.
void GDF_GetRelativePath(const char* abs_path, char* out_buf);

// MUST BE DESTROYED with GDF_FreeDirInfo
// Example:
// GDF_GetDirInfo("worlds") // gets ./worlds folder form executable folder
// Also valid:
// GDF_GetDirInfo("worlds/")
// GDF_GetDirInfo("worlds\\")
// GDF_GetDirInfo("worlds/players")
// ASSUMES THE RELATIVE PATH IS A VALID DIRECTORY
// RETURNS NULL
GDF_DirInfo* GDF_GetDirInfo(const char* rel_path);

GDF_IO_RESULT GDF_MakeFile(const char* rel_path);

GDF_IO_RESULT GDF_MakeDir(const char* rel_path);

GDF_IO_RESULT GDF_MakeDirAbs(const char* abs_path);

// WILL OVERWRITE CONTENTS OF FILE
GDF_IO_RESULT GDF_WriteFileOnce(const char* rel_path, const char* buf, u64 len);

GDF_IO_RESULT GDF_ReadFileOnce(const char* rel_path, char* out_buf, size_t bytes_to_read);

// must be freed with GDF_Free
// returns NULL on error
/// @note read_bytes cannot be NULL.
GDF_IO_RESULT GDF_FileReadAll(const char* rel_path, u64* read_size, u8** read_bytes);

// returns GDF_FALSE if overwrite_existing is GDF_FALSE and a file already exists at dest_path,
// or the src_path or dest_path is invalid.
GDF_IO_RESULT GDF_CopyFile(const char* src_path, const char* dest_path, GDF_BOOL overwrite_existing);

// the size of the file in bytes
GDF_IO_RESULT GDF_GetFileSize(const char* rel_path, u64* out_size);

GDF_IO_RESULT GDF_GetFileSizeAbs(const char* abs_path, u64* out_size);

/* Process API */

/// @brief Creates and starts a new child process.
/// @param command       The executable to run.
/// @param args          Array of comm and-line arguments (NULL-terminated).
/// @param working_dir   Working directory (NULL uses current directory).
/// @param env           Environment variables (NULL-terminated, and NULL inherits parent environment).
/// @return              A process handle, or NULL on failure.
GDF_Process GDF_CreateProcess(
    const char* command,
    const char* const args[],
    const char* working_dir,
    const char* const env[]
);

#define GDF_TIMEOUT_INFINITE 4294967295
/// @brief Waits for a process to complete.
/// @param process       Process to wait for.
/// @param exit_code     Output parameter for exit code (can be NULL).
/// @param timeout_ms    Timeout in milliseconds (or GDF_TIMEOUT_INFINITE).
/// @return              If the waiting was successful.
GDF_IO_RESULT GDF_WaitForProcess(
    GDF_Process process,
    i32* exit_code,
    u32 timeout_ms
);

/// @brief Terminates a running process.
/// @param process       Process to terminate.
/// @return              If the termination was successful.
GDF_IO_RESULT GDF_TerminateProcess(GDF_Process process);

/// @brief Shorthand for terminating a process and then freeing it's handle.
/// @param process       Process handle to free after terminating
/// @return              If the termination and freeing of resources was successful
GDF_IO_RESULT GDF_DestroyProcess(GDF_Process process);

/// @brief Cleans up a handle to a running process.
/// @param process A process handle.
void GDF_FreeProcessHandle(GDF_Process process);

// free resources
void GDF_FreeDirInfo(GDF_DirInfo* dir_info);

EXTERN_C_END