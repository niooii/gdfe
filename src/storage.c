#include <gdfe/prelude.h>
#include <gdfe/storage.h>
#include <gdfe/strutils.h>
#include <string.h>

GDF_IO_RESULT GDF_StorageWrite(const char* path, const char* buf, u64 len)
{
    GDF_String rel_path = GDF_StringCreateFrom(GDFE_STORAGE_ROOT, sizeof(GDFE_STORAGE_ROOT));

    GDF_StringPushf(&rel_path, "/%s", path);

    const GDF_IO_RESULT res = GDF_WriteFile(rel_path.str, buf, len);

    GDF_StringDestroy(&rel_path);

    return res;
}

GDF_IO_RESULT GDF_StorageRead(
    const char* path,
    char* out_buf,
    u64* read_bytes
)
{
    TODO("unimplemented");
}