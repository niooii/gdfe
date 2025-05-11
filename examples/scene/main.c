#include <gdfe/gdfe.h>

int main() {
    GDF_InitInfo init_info = {

    };

    GDF_AppState* app = GDF_Init(init_info);

    GDF_Run();

    return 0;
}
