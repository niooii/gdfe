#pragma once
#include "def.h"

typedef struct GDF_StringBuilder {
    char* str;
    u64 len;
} GDF_StringBuilder;

#ifdef __cplusplus
extern "C" {
#endif

/// Allocates memory. Must be destroyed with FreeStrBuilder
void GDF_InitStringBuilder(GDF_StringBuilder* out);
/// This frees the underlying string as well
void GDF_DestroyStringBuilder(GDF_StringBuilder* builder);

void GDF_PushChar(GDF_StringBuilder* builder, char c);
void GDF_PushString(GDF_StringBuilder* builder, const char* str);
void GDF_PushFormat(
    GDF_StringBuilder* builder,
    const char* format_str,
    ...
);

// Does not deallocate used memory.
void GDF_ClearStringBuilder(const GDF_StringBuilder* builder);

void GDF_ReplaceCharWith(char* str, char from, char to);

#ifdef __cplusplus
    }
#endif
