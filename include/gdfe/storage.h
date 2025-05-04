#pragma once

#include <gdfe/def.h>
#include <gdfe/os/io.h>

/// The relative path (to the executable's working directory) where paths passed into the
/// storage api are written and read from.
#define GDFE_STORAGE_ROOT ".gdfe"

EXTERN_C_BEGIN

/// Write a buffer to a file in the application's persistent storage.
GDF_IO_RESULT GDF_StorageWrite(const char* path, const char* buf, u64 len);

/// Read a file from the application's persistent storage.
GDF_IO_RESULT GDF_StorageRead(
    const char* path,
    char* out_buf,
    u64* read_bytes
);

EXTERN_C_END