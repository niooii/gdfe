#include <stdarg.h>
#include <stdio.h>
#include <gdfe/strutils.h>
#include <gdfe/collections/list.h>

void GDF_InitStringBuilder(GDF_StringBuilder* out)
{
    out->str = GDF_LIST_Reserve(char, 32);
    out->len = 0;
}

void GDF_DestroyStringBuilder(GDF_StringBuilder* builder)
{
    __list_destroy(builder->str);
}

void GDF_PushChar(GDF_StringBuilder* builder, const char c)
{
    GDF_LIST_PushPtr(builder->str, &c);
    builder->len++;

    // null terminate
    if (UNLIKELY(GDF_LIST_GetLength(builder->str) >= GDF_LIST_GetCapacity(builder->str)))
    {
        GDF_LIST_PushPtr(builder->str, NULL);
    }

    builder->str[builder->len] = 0;
}

void GDF_PushString(GDF_StringBuilder* builder, const char* str)
{
    u32 len = strlen(str);
    GDF_LIST_Append(builder->str, str, len);
    builder->len += len;

    // null terminate
    if (UNLIKELY(GDF_LIST_GetLength(builder->str) >= GDF_LIST_GetCapacity(builder->str)))
    {
        GDF_LIST_PushPtr(builder->str, NULL);
    }

    builder->str[builder->len] = 0;
}

void GDF_PushFormat(
    GDF_StringBuilder* builder,
    const char* format,
    ...
)
{
    va_list arg_ptr;
    va_list arg_ptr_copy;
    va_start(arg_ptr, format);
    va_copy(arg_ptr_copy, arg_ptr);

    u32 add_len = vsnprintf(NULL, 0, format, arg_ptr_copy);
    va_end(arg_ptr_copy);

    GDF_LIST_Append(builder->str, NULL, add_len + 1);

    u64 len = builder->len;
    char* start = builder->str + len;

    vsnprintf(start, add_len + 1, format, arg_ptr);

    GDF_LIST_SetLength(builder->str, len + add_len);
    builder->len += add_len;

    va_end(arg_ptr);

    // null terminate
    // unneeded for this one
    // builder->str[builder->len] = 0;
}