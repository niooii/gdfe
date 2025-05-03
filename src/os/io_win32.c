#include <gdfe/os/io.h>

#ifdef OS_WINDOWS
#include <gdfe/strutils.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <direct.h>
#include <errno.h>
#include <strsafe.h>

/* TODO! ALL THIS IS SUPER UNSAFE */

static HWND console_window = NULL;
static char* EXECUTABLE_PATH;
static HANDLE stdout_;

void gdfe_io_init()
{
    EXECUTABLE_PATH = GDF_Malloc(MAX_PATH_LEN, GDF_MEMTAG_STRING);
    GetModuleFileName(NULL, EXECUTABLE_PATH, MAX_PATH_LEN);

    // Find the last occurrence of '\'
    char* lastBackslash = strrchr(EXECUTABLE_PATH, '\\');
    if (lastBackslash != NULL) {
        *(lastBackslash+1) = 0; // Null-terminate the string at the last '\'
    }
    stdout_ = GetStdHandle(STD_OUTPUT_HANDLE);
}

void gdfe_io_shutdown()
{
    GDF_Free(EXECUTABLE_PATH);
    TODO("cleanup io subsys resources")
}

void GDF_ShowConsole()
{
    if (console_window == NULL)
        console_window = GetConsoleWindow();

    ShowWindow(console_window, SW_SHOW);
}

void GDF_HideConsole()
{
    if (console_window == NULL)
        console_window = GetConsoleWindow();

    ShowWindow(console_window, SW_HIDE);
}

const char* GDF_GetExecutablePath()
{
    return EXECUTABLE_PATH;
}

void GDF_WriteConsole(const char* msg)
{
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    // static u8 levels[6] = {64, 4, 6, 2, 1, 8};

    // OutputDebugStringA(msg);
    
    // TODO! this strlen is probably expensive at the scales im using it at
    u64 len = strlen(msg);
    WriteConsoleA(stdout_, msg, (DWORD)len, 0, 0);  
}

void GDF_GetAbsolutePath(const char* rel_path, char* out_buf)
{
    // TODO! 
    StringCchCopy(out_buf, MAX_PATH_LEN, EXECUTABLE_PATH);
    StringCchCat(out_buf, MAX_PATH_LEN, rel_path);

    GDF_ReplaceCharWith(out_buf, '/', '\\');
}

void GDF_GetRelativePath(const char* abs_path, char* out_buf)
{
    size_t exec_path_len = strlen(EXECUTABLE_PATH);
    if (strlen(abs_path) <= exec_path_len)
    {
        out_buf = NULL;
        return;
    }
    
    strcpy(out_buf, &abs_path[exec_path_len]);
}

GDF_DirInfo* GDF_GetDirInfo(const char* rel_path) 
{
    HANDLE hFind;
    WIN32_FIND_DATA FindData;
    char tmp_dir[MAX_PATH_LEN];
    if (strcmp(rel_path, ".") == 0)
    {
        strcpy(tmp_dir, EXECUTABLE_PATH);
    }
    else
    {
        GDF_GetAbsolutePath(rel_path, tmp_dir);
    }
    TCHAR dir[MAX_PATH_LEN];
    StringCchCopy(dir, MAX_PATH_LEN, tmp_dir);
    // Find the last occurrence of '\'
    GDF_BOOL last_char_is_backslash = rel_path[strlen(rel_path) - 1] == '\\';
    GDF_BOOL last_char_is_slash = rel_path[strlen(rel_path) - 1] == '/'; 
    if (!last_char_is_backslash && !last_char_is_slash) 
    {
        size_t strlength = strlen(dir);
        *(dir + strlength) = '\\'; // add one ourself '\'
        *(dir + strlength + 1) = '\0'; // null terminate
    }
    else if (last_char_is_slash)
    {
        size_t strlength = strlen(dir);
        *(dir + strlength - 1) = (char)'\\'; // replace with '\'
    }
    TCHAR search_path[MAX_PATH_LEN];
    StringCchCopy(search_path, MAX_PATH_LEN, dir);
    StringCchCat(search_path, MAX_PATH_LEN, TEXT("*"));

    hFind = FindFirstFile(search_path, &FindData);
    GDF_DirInfo* dir_info = GDF_Malloc(sizeof(*dir_info), GDF_MEMTAG_TEMP_RESOURCE);
    dir_info->nodes = GDF_Malloc(sizeof(*dir_info->nodes), GDF_MEMTAG_TEMP_RESOURCE);
    // for some reason this never triggers. it always works.
    // even when given a random ass path it somehow manages
    // to return a valid handle and quite frankly, i am
    // very pressed about it. according to the docs, this 
    // is SUPPOSED to work. but of course, why would anything
    // work as documented, right? i am quite distressed about
    // the lack of error handling in the FindFirstFileW function
    // and will pray that this functionality will remain the same
    // in future versions of windows. Because honestly, 
    // the windows api has to be the most documented api
    // i've ever come across. and even so, why is it STILL
    // so DIFFICULT to do ANYTHING in here? I just want simple error handling.
    // I guess thats okay though. We'll just never know
    // when something went wrong/. It's just unnecessary right?
    // Who needs an api to work as documented? Why would anyone
    // expect it to work the way it was WRITTEN to work?
    // if (FindData.cFileName == INVALID_HANDLE_VALUE)
    // {
    //     LOG_WARN("Could not search directory %s", search_path);
    //     return NULL;
    // }
    int found_files = 0;
    while(FindNextFile(hFind, &FindData))
    {       
        // y no work
        if (strcmp(FindData.cFileName, ".") == 0 || strcmp(FindData.cFileName, "..") == 0) 
        {
            found_files++;
            continue;
        }
        // add node
        dir_info->nodes[dir_info->num_nodes] = GDF_Malloc(sizeof(GDF_DirInfoNode*), GDF_MEMTAG_TEMP_RESOURCE);
        GDF_DirInfoNode* node = dir_info->nodes[dir_info->num_nodes];
        node = GDF_Malloc(sizeof(GDF_DirInfoNode), GDF_MEMTAG_TEMP_RESOURCE);
        dir_info->num_nodes++;
        node->name = GDF_Malloc(strlen(FindData.cFileName) + 1, GDF_MEMTAG_STRING);
        strcpy(node->name, FindData.cFileName);
        LOG_DEBUG("found file %s", FindData.cFileName);
        node->full_path = GDF_StrcatNoOverwrite(dir, FindData.cFileName);
        node->is_directory = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        LOG_DEBUG("at path %s", node->full_path);
    }
    FindClose(hFind);

    if (found_files == 0)
    {
        // we should have found the .. directory and
        // found_files wouldve been 1, so this
        // can only mean the directory is invalid.
        LOG_WARN("Could not search directory: %s", dir);
        return NULL;
    }

    return dir_info;
}

GDF_IO_RESULT GDF_MakeFile(const char* rel_path) {
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    HANDLE h = CreateFile(path, 0, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
    GDF_BOOL success = h != INVALID_HANDLE_VALUE;
    if (success)
        return GDF_IO_RESULT_SUCCESS;

    switch(GetLastError()) {
    case ERROR_FILE_EXISTS:
        return GDF_IO_RESULT_ALREADY_EXISTS;
    case ERROR_FILE_NOT_FOUND:
        return GDF_IO_RESULT_PATH_NOT_FOUND;
    default:
        return GDF_IO_RESULT_FAIL;
    }
}

GDF_IO_RESULT GDF_MakeDir(const char* rel_path) {
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    GDF_BOOL success = CreateDirectoryA(path, NULL);

    if (success)
        return GDF_IO_RESULT_SUCCESS;

    switch (GetLastError()) {
    case ERROR_ALREADY_EXISTS:
        return GDF_IO_RESULT_ALREADY_EXISTS;
    case ERROR_PATH_NOT_FOUND:
        return GDF_IO_RESULT_PATH_NOT_FOUND;
    default:
        return GDF_IO_RESULT_FAIL;
    }
}

GDF_IO_RESULT GDF_MakeDirAbs(const char* abs_path)
{
    int res = _mkdir(abs_path);
    if (res == 0)
        return GDF_IO_RESULT_SUCCESS;

    switch (errno) {
    case EEXIST:
        return GDF_IO_RESULT_ALREADY_EXISTS;
    case ENOENT:
        return GDF_IO_RESULT_PATH_NOT_FOUND;
    default:
        return GDF_IO_RESULT_FAIL;
    }
}


GDF_IO_RESULT GDF_WriteFile(const char* rel_path, const char* data) {
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    HANDLE h = CreateFile(path, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, 0, 0);

    if (h == INVALID_HANDLE_VALUE) {
        switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:
            return GDF_IO_RESULT_PATH_NOT_FOUND;
        default:
            return GDF_IO_RESULT_FAIL;
        }
    }

    GDF_BOOL success = WriteFile(h, data, strlen(data), NULL, NULL);
    CloseHandle(h);

    if (!success) {
        LOG_ERR("Unknown error (%d) writing to file: %s", GetLastError(), path);
        return GDF_IO_RESULT_FAIL;
    }

    return GDF_IO_RESULT_SUCCESS;
}

GDF_IO_RESULT GDF_ReadFile(const char* rel_path, char* out_buf, size_t bytes_to_read) {
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    HANDLE h = CreateFile(path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

    if (h == INVALID_HANDLE_VALUE) {
        switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:
            return GDF_IO_RESULT_PATH_NOT_FOUND;
        default:
            return GDF_IO_RESULT_FAIL;
        }
    }

    DWORD bytes_read = 0;
    GDF_BOOL success = ReadFile(h, (LPVOID)out_buf, bytes_to_read, &bytes_read, NULL);
    CloseHandle(h);

    if (!success) {
        LOG_ERR("Unknown error (%d) reading file: %s", GetLastError(), path);
        return GDF_IO_RESULT_FAIL;
    }

    // LOG_INFO("Read file: %s", path);
    return GDF_IO_RESULT_SUCCESS;
}

char* GDF_ReadFileExactLen(const char* rel_path)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    // If i dont add a bit more, it reads some garbage characters after the end
    // of the file. please dont ask me why this happens, i will not know.
    u64 size = GDF_GetFileSizeAbs(path) + 25;
    char* out_buf = GDF_Malloc(size, GDF_MEMTAG_STRING);
    HANDLE h = CreateFile(path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    GDF_BOOL success = h != INVALID_HANDLE_VALUE;
    if (!success)
    {   
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            LOG_ERR("Could not read non-existent file: %s", path);
        }
        else
        {
            LOG_WARN("Unknown error opening read handle to file: %s", path);
        }
        return NULL;
    }
    DWORD bytes_read = 0;
    GDF_BOOL w_success = ReadFile(h, (LPVOID)out_buf, size, &bytes_read, NULL);
    if (w_success)
    {
        // LOG_INFO("Read fil %s", path);
    }
    else
    {
        LOG_ERR("Unknown error (%d) reading file: %s", GetLastError(), path);
    }
    LOG_DEBUG("%02X", out_buf);
    CloseHandle(h);
    return out_buf;
}

u8* GDF_ReadBytesExactLen(const char* rel_path, u64* bytes_read)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    FILE* file = fopen(path, "rb");

    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    u64 size = ftell(file);
    rewind(file);
    LOG_DEBUG("File size: %u", size);

    u8* out_bytes = GDF_Malloc(size, GDF_MEMTAG_STRING);
    *bytes_read = fread(out_bytes, 1, size, file);
    LOG_DEBUG("bytes read: %u", bytes_read);
    fclose(file);    
    if (*bytes_read != size)
    {
        *bytes_read = 0;
        LOG_WARN("Reading binary from file failed...");
        return NULL;
    }
    // FILE* file1 = fopen(path, "wb");
    // fwrite(out_bytes, 1, size, file1);
    // fclose(file1);

    return out_bytes;
}

GDF_IO_RESULT GDF_CopyFile(const char* src_path, const char* dest_path, GDF_BOOL overwrite_existing)
{
    char src_path_abs[MAX_PATH_LEN];
    GDF_GetAbsolutePath(src_path, src_path_abs);
    char dest_path_abs[MAX_PATH_LEN];
    GDF_GetAbsolutePath(dest_path, dest_path_abs);
    BOOL result = CopyFile(src_path_abs, dest_path_abs, !overwrite_existing);
    if (result == 0)
        return GDF_IO_RESULT_SUCCESS;

    switch (GetLastError()) {
    case ERROR_ACCESS_DENIED:
        return GDF_IO_RESULT_ACCESS_DENIED;
    default:
        return GDF_IO_RESULT_FAIL;
    }
}

u64 GDF_GetFileSize(const char* rel_path)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);

    return GDF_GetFileSizeAbs(path);
}

u64 GDF_GetFileSizeAbs(const char* abs_path)
{
    FILE* file = fopen(abs_path, "rb");
    if (file == NULL)
    {
        perror("Error");
    }
    fseek(file, 0, SEEK_END); 
    u64 size = ftell(file);
    fclose(file);

    return size;
}

// MUST CALL FREE AFTER USE
char* GDF_StrcatNoOverwrite(const char* s1,const char* s2)
{
  char* p = GDF_Malloc(strlen(s1) + strlen(s2) + 1, GDF_MEMTAG_STRING);

  char* start = p;
  if (p != NULL)
  {
       while (*s1 != '\0')
       *p++ = *s1++;
       while (*s2 != '\0')
       *p++ = *s2++;
      *p = '\0';
 }

  return start;
}

#include <gdfe/strutils.h>
char* GDF_Strdup(const char* str)
{
    // CHECK HERE
    char* dup = GDF_Malloc(strlen(str) + 1, GDF_MEMTAG_STRING);
    strcpy(dup, str);
    return dup;
}

typedef struct GDF_Process_T {
    u32 pid;
    HANDLE handle;
    bool running;
} GDF_Process_T;

GDF_Process GDF_CreateProcess(
    const char* command,
    const char* const args[],
    const char* working_dir,
    const char* const env[]
)
{
    GDF_StringBuilder exec_str;
    GDF_InitStringBuilder(&exec_str);

    GDF_PushString(&exec_str, command);

    for (int i = 0; args && args[i]; i++) {
        GDF_PushChar(&exec_str, ' ');
        GDF_PushString(&exec_str, args[i]);
    }

    LOG_DEBUG("Starting process \"%s\"", exec_str.str);

    STARTUPINFO si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL success = CreateProcess(
        NULL,               // No module name (use cmd line)
        exec_str.str,
        NULL,               // Process handle not inheritable
        NULL,               // Thread handle not inheritable
        FALSE,              // Don't inherit handles
        0,                  // No creation flags
        NULL,               // Use parent's environment
        working_dir,
        &si,
        &pi
    );

    if (!success) {
        return NULL;
    }

    GDF_Process proc = GDF_Malloc(sizeof(GDF_Process_T), GDF_MEMTAG_IO);
    proc->handle = pi.hProcess;
    proc->pid = pi.dwProcessId;
    proc->running = true;

    return proc;
}

GDF_IO_RESULT GDF_WaitForProcess(
    GDF_Process process,
    i32* exit_code,
    u32 timeout_ms
)
{
    if (!process || !process->handle)
        return GDF_IO_RESULT_BAD_HANDLE;

    DWORD wait_result = WaitForSingleObject(
        process->handle,
        timeout_ms == GDF_TIMEOUT_INFINITE ? INFINITE : (DWORD)timeout_ms
    );

    if (wait_result == WAIT_FAILED)
        return GDF_IO_RESULT_FAIL;

    if (wait_result != WAIT_TIMEOUT) {
        process->running = false;

        if (exit_code) {
            DWORD code;
            if (!GetExitCodeProcess(process->handle, &code)) {
                return GDF_IO_RESULT_FAIL;
            }
            *exit_code = (int)code;
        }
    }

    return GDF_IO_RESULT_PROC_WAIT_TIMEOUT;
}

GDF_IO_RESULT GDF_TerminateProcess(GDF_Process process)
{
    if (!process || !process->handle) {
        return GDF_IO_RESULT_BAD_HANDLE;
    }

    if (!process->running) {
        return GDF_IO_RESULT_SUCCESS;
    }

    if (!TerminateProcess(process->handle, 1)) {
        return GDF_IO_RESULT_FAIL;
    }

    process->running = false;
    return GDF_IO_RESULT_SUCCESS;
}

GDF_IO_RESULT GDF_DestroyProcess(GDF_Process process)
{
    GDF_IO_RESULT status = GDF_TerminateProcess(process);
    if (status != GDF_IO_RESULT_SUCCESS)
        return GDF_IO_RESULT_FAIL;

    GDF_FreeProcessHandle(process);
    return GDF_IO_RESULT_SUCCESS;
}

void GDF_FreeProcessHandle(GDF_Process process)
{
    if (process) {
        if (process->handle)
        {
            CloseHandle(process->handle);
            process->handle = NULL;
            process->running = false;
        }
        GDF_Free(process);
    }
}

void GDF_FreeDirInfo(GDF_DirInfo* dir_info)
{
    for (u32 i = 0; i < dir_info->num_nodes; i++)
    {
        GDF_DirInfoNode* node = dir_info->nodes[i];
        GDF_Free(node->full_path);
        GDF_Free(node->name);
        GDF_Free(node);
    }
    GDF_Free(dir_info->nodes);

    GDF_Free(dir_info);
}

#endif