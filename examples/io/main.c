#include <gdfe/gdfe.h>

int main()
{
    GDF_ASSERT(GDF_InitSubsystems());

    GDF_WriteFileOnce("test.txt", "hi world!", 10);

    u64 read;
    u8* buf;

    GDF_ASSERT(GDF_FileReadAll("this_path_does_not_exist", &read, &buf) !=
        GDF_IO_RESULT_SUCCESS);
    GDF_ASSERT(GDF_FileReadAll("resources/shaders/grid.vert", &read, &buf));

    LOG_DEBUG("%s", buf);

    GDF_FlushLogBuffer();

    return 0;
}
