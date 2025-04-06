#pragma once
#include "def.h"

typedef struct GDF_StringBuilder {
    char* str;
    u64 len;
} GDF_StringBuilder;

#ifdef __cplusplus
extern "C" {
#endif

// Allocates memory. Must be destroyed with FreeStrBuilder
void GDF_InitStringBuilder(GDF_StringBuilder* out);
// This frees the underlying string as well
void GDF_DestroyStringBuilder(GDF_StringBuilder* builder);

void GDF_PushChar(GDF_StringBuilder* builder, char c);
void GDF_PushString(GDF_StringBuilder* builder, const char* str);
void GDF_PushFormat(
    GDF_StringBuilder* builder,
    const char* format_str,
    ...
);

#ifdef __cplusplus
    }
#endif
