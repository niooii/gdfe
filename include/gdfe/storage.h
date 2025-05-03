#pragma once

#include <gdfe/def.h>

/// The relative path (to the executable's working directory) where paths passed into the
/// storage api are written and read from.
#define GDFE_STORAGE_ROOT ".gdfe"

/// Write a buffer to a file in the application's persistent storage.
GDF_BOOL GDF_StorageWrite(const char* path, const char* buf, u64 buf_len);

/// Read a file from the application's persistent storage.
GDF_BOOL GDF_StorageRead(
    const char* path,
    char* out_buf,
    u64* read_bytes
);