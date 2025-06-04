#include <gdfe/os/io.h>

#ifdef OS_LINUX
    #include <gdfe/prelude.h>
    #define _GNU_SOURCE
    #include <errno.h>
    #include <fcntl.h>
    #include <libgen.h>
    #include <signal.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>

static char* EXECUTABLE_PATH = NULL;

void gdfe_io_init()
{
    EXECUTABLE_PATH = GDF_Malloc(MAX_PATH_LEN, GDF_MEMTAG_STRING);

    // Try different methods to get executable path
    ssize_t len = readlink("/proc/self/exe", EXECUTABLE_PATH, MAX_PATH_LEN - 1);
    if (len == -1)
    {
        // Fallback for systems without /proc (like macOS)
        len = readlink("/proc/curproc/file", EXECUTABLE_PATH, MAX_PATH_LEN - 1);
        if (len == -1)
        {
            // Another fallback - use argv[0] if available or current directory
            if (getcwd(EXECUTABLE_PATH, MAX_PATH_LEN) == NULL)
            {
                strcpy(EXECUTABLE_PATH, "./");
                len = 2;
            }
            else
            {
                len = strlen(EXECUTABLE_PATH);
                if (len < MAX_PATH_LEN - 1)
                {
                    EXECUTABLE_PATH[len]     = '/';
                    EXECUTABLE_PATH[len + 1] = '\0';
                    len++;
                }
            }
        }
    }
    else
    {
        EXECUTABLE_PATH[len] = '\0';
        // Get directory part only
        char* last_slash = strrchr(EXECUTABLE_PATH, '/');
        if (last_slash != NULL)
        {
            *(last_slash + 1) = '\0';
        }
    }
}

void gdfe_io_shutdown()
{
    if (EXECUTABLE_PATH)
    {
        GDF_Free(EXECUTABLE_PATH);
        EXECUTABLE_PATH = NULL;
    }
}

void GDF_ShowConsole()
{
    // not really a thing in posix
}

void GDF_HideConsole()
{
    // not really a thing in posix
}

const char* GDF_GetExecutablePath() { return EXECUTABLE_PATH; }

void GDF_WriteConsole(const char* msg)
{
    size_t len = strlen(msg);
    write(STDOUT_FILENO, msg, len);
}

void GDF_GetAbsolutePath(const char* rel_path, char* out_buf)
{
    if (rel_path[0] == '/')
    {
        strncpy(out_buf, rel_path, MAX_PATH_LEN - 1);
        out_buf[MAX_PATH_LEN - 1] = '\0';
    }
    else
        snprintf(out_buf, MAX_PATH_LEN, "%s%s", EXECUTABLE_PATH, rel_path);
}

void GDF_GetRelativePath(const char* abs_path, char* out_buf)
{
    size_t exec_path_len = strlen(EXECUTABLE_PATH);
    if (strlen(abs_path) <= exec_path_len)
    {
        out_buf[0] = '\0';
        return;
    }

    if (strncmp(abs_path, EXECUTABLE_PATH, exec_path_len) == 0)
    {
        strcpy(out_buf, &abs_path[exec_path_len]);
    }
    else
    {
        out_buf[0] = '\0';
    }
}

GDF_IO_RESULT GDF_MakeFile(const char* rel_path)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);

    int fd = open(path, O_CREAT | O_EXCL, 0644);
    if (fd >= 0)
    {
        close(fd);
        return GDF_IO_RESULT_SUCCESS;
    }

    switch (errno)
    {
    case EEXIST:
        return GDF_IO_RESULT_ALREADY_EXISTS;
    case ENOENT:
        return GDF_IO_RESULT_PATH_NOT_FOUND;
    case EACCES:
        return GDF_IO_RESULT_ACCESS_DENIED;
    default:
        return GDF_IO_RESULT_FAIL;
    }
}

GDF_IO_RESULT GDF_MakeDir(const char* rel_path)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    return GDF_MakeDirAbs(path);
}

GDF_IO_RESULT GDF_MakeDirAbs(const char* abs_path)
{
    int result = mkdir(abs_path, 0755);
    if (result == 0)
    {
        return GDF_IO_RESULT_SUCCESS;
    }

    switch (errno)
    {
    case EEXIST:
        return GDF_IO_RESULT_ALREADY_EXISTS;
    case ENOENT:
        return GDF_IO_RESULT_PATH_NOT_FOUND;
    case EACCES:
        return GDF_IO_RESULT_ACCESS_DENIED;
    default:
        return GDF_IO_RESULT_FAIL;
    }
}

GDF_IO_RESULT GDF_WriteFileOnce(const char* rel_path, const char* buf, u64 len)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        switch (errno)
        {
        case ENOENT:
            return GDF_IO_RESULT_PATH_NOT_FOUND;
        case EACCES:
            return GDF_IO_RESULT_ACCESS_DENIED;
        default:
            return GDF_IO_RESULT_FAIL;
        }
    }

    ssize_t written = write(fd, buf, len);
    close(fd);

    if (written < 0 || (u64)written != len)
    {
        return GDF_IO_RESULT_FAIL;
    }

    return GDF_IO_RESULT_SUCCESS;
}

GDF_IO_RESULT GDF_ReadFileOnce(const char* rel_path, char* out_buf, size_t bytes_to_read)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);

    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        switch (errno)
        {
        case ENOENT:
            return GDF_IO_RESULT_PATH_NOT_FOUND;
        case EACCES:
            return GDF_IO_RESULT_ACCESS_DENIED;
        default:
            return GDF_IO_RESULT_FAIL;
        }
    }

    ssize_t bytes_read = read(fd, out_buf, bytes_to_read);
    close(fd);

    if (bytes_read < 0)
    {
        return GDF_IO_RESULT_FAIL;
    }

    return GDF_IO_RESULT_SUCCESS;
}

GDF_IO_RESULT GDF_FileReadAll(const char* rel_path, u64* read_size, u8** read_bytes)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);

    GDF_IO_RESULT result = GDF_GetFileSizeAbs(path, read_size);
    if (result != GDF_IO_RESULT_SUCCESS)
    {
        return result;
    }

    u8* out_buf = GDF_Malloc(*read_size, GDF_MEMTAG_STRING);
    int fd      = open(path, O_RDONLY);

    if (fd < 0)
    {
        GDF_Free(out_buf);
        switch (errno)
        {
        case ENOENT:
            return GDF_IO_RESULT_PATH_NOT_FOUND;
        case EACCES:
            return GDF_IO_RESULT_ACCESS_DENIED;
        default:
            return GDF_IO_RESULT_FAIL;
        }
    }

    ssize_t bytes_read = read(fd, out_buf, *read_size);
    close(fd);

    if (bytes_read < 0 || (u64)bytes_read != *read_size)
    {
        GDF_Free(out_buf);
        return GDF_IO_RESULT_FAIL;
    }

    *read_bytes = out_buf;
    return GDF_IO_RESULT_SUCCESS;
}

GDF_IO_RESULT GDF_CopyFile(const char* src_path, const char* dest_path, GDF_BOOL overwrite_existing)
{
    char src_abs[MAX_PATH_LEN], dest_abs[MAX_PATH_LEN];
    GDF_GetAbsolutePath(src_path, src_abs);
    GDF_GetAbsolutePath(dest_path, dest_abs);

    // Check if destination exists
    if (!overwrite_existing)
    {
        struct stat st;
        if (stat(dest_abs, &st) == 0)
        {
            return GDF_IO_RESULT_ALREADY_EXISTS;
        }
    }

    int src_fd = open(src_abs, O_RDONLY);
    if (src_fd < 0)
    {
        return GDF_IO_RESULT_PATH_NOT_FOUND;
    }

    int dest_fd = open(dest_abs, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0)
    {
        close(src_fd);
        switch (errno)
        {
        case EACCES:
            return GDF_IO_RESULT_ACCESS_DENIED;
        default:
            return GDF_IO_RESULT_FAIL;
        }
    }

    char          buffer[8192];
    ssize_t       bytes_read, bytes_written;
    GDF_IO_RESULT result = GDF_IO_RESULT_SUCCESS;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0)
    {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read)
        {
            result = GDF_IO_RESULT_FAIL;
            break;
        }
    }

    if (bytes_read < 0)
    {
        result = GDF_IO_RESULT_FAIL;
    }

    close(src_fd);
    close(dest_fd);

    return result;
}

GDF_IO_RESULT GDF_GetFileSize(const char* rel_path, u64* out_size)
{
    char path[MAX_PATH_LEN];
    GDF_GetAbsolutePath(rel_path, path);
    return GDF_GetFileSizeAbs(path, out_size);
}

GDF_IO_RESULT GDF_GetFileSizeAbs(const char* abs_path, u64* out_size)
{
    struct stat st;
    if (stat(abs_path, &st) < 0)
    {
        switch (errno)
        {
        case ENOENT:
            return GDF_IO_RESULT_PATH_NOT_FOUND;
        case EACCES:
            return GDF_IO_RESULT_ACCESS_DENIED;
        default:
            return GDF_IO_RESULT_FAIL;
        }
    }

    *out_size = st.st_size;
    return GDF_IO_RESULT_SUCCESS;
}

typedef struct GDF_Process_T {
    pid_t pid;
    bool  running;
} GDF_Process_T;

GDF_Process GDF_CreateProcess(
    const char* command, const char* const args[], const char* working_dir, const char* const env[])
{
    pid_t pid = fork();

    if (pid < 0)
    {
        // Fork failed
        return NULL;
    }

    if (pid == 0)
    {
        // Child process
        if (working_dir)
        {
            if (chdir(working_dir) < 0)
            {
                _exit(1);
            }
        }

        // Count arguments
        int argc = 0;
        if (args)
        {
            while (args[argc])
                argc++;
        }

        // Prepare argv array
        char** argv = malloc((argc + 2) * sizeof(char*));
        argv[0]     = (char*)command;
        for (int i = 0; i < argc; i++)
        {
            argv[i + 1] = (char*)args[i];
        }
        argv[argc + 1] = NULL;

        // Execute
        if (env)
        {
            execve(command, argv, (char* const*)env);
        }
        else
        {
            execv(command, argv);
        }

        // If we get here, exec failed
        _exit(1);
    }

    // Parent process
    GDF_Process proc = GDF_Malloc(sizeof(GDF_Process_T), GDF_MEMTAG_IO);
    proc->pid        = pid;
    proc->running    = true;

    return proc;
}

GDF_IO_RESULT GDF_WaitForProcess(GDF_Process process, i32* exit_code, u32 timeout_ms)
{
    if (!process)
    {
        return GDF_IO_RESULT_BAD_HANDLE;
    }

    int status;
    int result;

    if (timeout_ms == GDF_TIMEOUT_INFINITE)
    {
        result = waitpid(process->pid, &status, 0);
    }
    else
    {
        // POSIX doesn't have a direct timeout for waitpid
        // Use non-blocking wait in a loop with sleep
        u32       elapsed  = 0;
        const u32 sleep_ms = 10;

        do
        {
            result = waitpid(process->pid, &status, WNOHANG);
            if (result > 0)
                break;
            if (result < 0)
                break;

            usleep(sleep_ms * 1000);
            elapsed += sleep_ms;
        }
        while (elapsed < timeout_ms);

        if (result == 0)
        {
            return GDF_IO_RESULT_PROC_WAIT_TIMEOUT;
        }
    }

    if (result < 0)
    {
        return GDF_IO_RESULT_FAIL;
    }

    process->running = false;

    if (exit_code)
    {
        if (WIFEXITED(status))
        {
            *exit_code = WEXITSTATUS(status);
        }
        else
        {
            *exit_code = -1;
        }
    }

    return GDF_IO_RESULT_SUCCESS;
}

GDF_IO_RESULT GDF_TerminateProcess(GDF_Process process)
{
    if (!process)
    {
        return GDF_IO_RESULT_BAD_HANDLE;
    }

    if (!process->running)
    {
        return GDF_IO_RESULT_SUCCESS;
    }

    if (kill(process->pid, SIGTERM) < 0)
    {
        return GDF_IO_RESULT_FAIL;
    }

    process->running = false;
    return GDF_IO_RESULT_SUCCESS;
}

GDF_IO_RESULT GDF_DestroyProcess(GDF_Process process)
{
    GDF_IO_RESULT status = GDF_TerminateProcess(process);
    if (status != GDF_IO_RESULT_SUCCESS)
    {
        return GDF_IO_RESULT_FAIL;
    }

    GDF_FreeProcessHandle(process);
    return GDF_IO_RESULT_SUCCESS;
}

void GDF_FreeProcessHandle(GDF_Process process)
{
    if (process)
    {
        GDF_Free(process);
    }
}

void GDF_FreeDirInfo(GDF_DirInfo* dir_info)
{
    if (!dir_info)
        return;

    for (u32 i = 0; i < dir_info->num_nodes; i++)
    {
        GDF_DirInfoNode* node = dir_info->nodes[i];
        if (node)
        {
            GDF_Free(node->full_path);
            GDF_Free(node->name);
            GDF_Free(node);
        }
    }
    GDF_Free(dir_info->nodes);
    GDF_Free(dir_info);
}

#endif
