#pragma once
#include "def.h"

/// A string builder type.  Either create it on the stack and initialize it with
/// GDF_InitString, or call GDF_CreateString to return a copy by value as it is not
/// expensive to copy.
typedef struct GDF_String {
    char* str;
    /// Does not include the null terminator.
    u64 len;
} GDF_String;

EXTERN_C_BEGIN

/// Allocates memory. Must be destroyed with FreeStrBuilder
void GDF_StringInit(GDF_String* out);

/// Allocates memory. Must be destroyed with FreeStrBuilder
void GDF_StringInitFrom(GDF_String* out, const char* buf, u64 len);

/// Allocates memory. Must be destroyed with FreeStrBuilder.
/// This function returns a GDF_String by copy, as it is cheap to do so.
/// For an in-place initialization, call GDF_InitString.
GDF_String GDF_StringCreate();

/// Allocates memory. Must be destroyed with FreeStrBuilder.
/// This function returns a GDF_String by copy, as it is cheap to do so.
/// For an in-place initialization, call GDF_InitString.
GDF_String GDF_StringCreateFrom(const char* buf, u64 len);

/// Destroys the resources owned by a GDF_String.
/// This frees the underlying string as well (the char*).
void GDF_StringDestroy(GDF_String* str);

void GDF_StringPushChar(GDF_String* str, char c);
void GDF_StringPush(GDF_String* str, const char* to_push, u64 len);
void GDF_StringPushf(GDF_String* str, const char* format_str, ...);

/// "Clears" the contents of the string by null terminating the underlying string
/// and setting the len attribute to 0.
/// Does not deallocate used memory.
void GDF_StringClear(GDF_String* str);

void GDF_ReplaceCharWith(char* str, char from, char to);

EXTERN_C_END
