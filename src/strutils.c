#include <gdfe/collections/list.h>
#include <gdfe/strutils.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void GDF_StringInit(GDF_String* out)
{
    out->str = GDF_ListReserve(char, 32);
    out->len = 0;
}

void GDF_StringInitFrom(GDF_String* out, const char* buf, u64 len)
{
    out->str = GDF_ListReserve(char, MAX(len, 32));
    out->len = len - 1;
    GDF_Memcpy(out->str, buf, len);
    GDF_ListSetLen(out->str, len - 1);
}

GDF_String GDF_StringCreate()
{
    GDF_String str;
    GDF_StringInit(&str);

    return str;
}

GDF_String GDF_StringCreateFrom(const char* buf, u64 len)
{
    GDF_String str;
    GDF_StringInitFrom(&str, buf, len);

    return str;
}

void GDF_StringDestroy(GDF_String* builder) { gdfe_list_destroy(builder->str); }

void GDF_StringPushChar(GDF_String* builder, const char c)
{
    GDF_ListPushPtr(builder->str, &c);
    builder->len++;

    // null terminate
    if (UNLIKELY(GDF_ListLen(builder->str) >= GDF_ListCapacity(builder->str)))
    {
        GDF_ListPushPtr(builder->str, NULL);
    }

    builder->str[builder->len] = 0;
}

void GDF_StringPush(GDF_String* builder, const char* to_push, u64 len)
{
    GDF_ListAppend(builder->str, to_push, len);
    builder->len += len;

    // null terminate
    if (UNLIKELY(GDF_ListLen(builder->str) >= GDF_ListCapacity(builder->str)))
        GDF_ListPushPtr(builder->str, NULL);

    builder->str[builder->len] = 0;
}

void GDF_StringPushf(GDF_String* builder, const char* format, ...)
{
    va_list arg_ptr;
    va_list arg_ptr_copy;
    va_start(arg_ptr, format);
    va_copy(arg_ptr_copy, arg_ptr);

    u32 add_len = vsnprintf(NULL, 0, format, arg_ptr_copy);
    va_end(arg_ptr_copy);

    GDF_ListAppend(builder->str, NULL, add_len + 1);

    u64   len   = builder->len;
    char* start = builder->str + len;

    vsnprintf(start, add_len + 1, format, arg_ptr);

    GDF_ListSetLen(builder->str, len + add_len);
    builder->len += add_len;

    va_end(arg_ptr);

    // null terminate
    // unneeded for this one
    // builder->str[builder->len] = 0;
}

void GDF_StringClear(GDF_String* builder)
{
    GDF_ListClear(builder->str);
    builder->str[0] = 0;
    builder->len    = 0;
}

void GDF_ReplaceCharWith(char* str, char from, char to)
{
    u32 i = 0;
    while (str[i])
    {
        if (str[i] == from)
            str[i] = to;
        i++;
    }
}
