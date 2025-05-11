#include <gdfe/gdfe.h>

int main() {
    GDF_ASSERT(GDF_InitSubsystems());

    GDF_WriteFile("test.txt", "hi world!", 10);

    return 0;
}
