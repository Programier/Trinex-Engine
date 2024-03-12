#ifndef _MSC_VER
#ifndef SLANG_CORE_SECURE_CRT_H
#define SLANG_CORE_SECURE_CRT_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include <wchar.h>

inline void slang_memcpy_s(void *dest, [[maybe_unused]] size_t destSize, const void * src, size_t count)
{
    assert(destSize >= count);
    memcpy(dest, src, count);
}

#define _TRUNCATE ((size_t)-1)
#define _stricmp strcasecmp

inline void slang_fopen_s(FILE**f, const char * fileName, const char * mode)
{
	*f = fopen(fileName, mode);
}

inline size_t slang_fread_s(void * buffer, [[maybe_unused]] size_t bufferSize, size_t elementSize, size_t count, FILE * stream)
{
    assert(bufferSize >= elementSize * count);
    return fread(buffer, elementSize, count, stream);
}

inline size_t slang_wcsnlen_s(const wchar_t * str, size_t /*numberofElements*/)
{
	return wcslen(str);
}

inline size_t slang_strnlen_s(const char * str, size_t numberOfElements)
{
#if defined( __CYGWIN__ )
    const char* cur = str;
    if (str)
    {
        const char*const end = str + numberOfElements;
        while (*cur && cur < end) cur++;
    }
    return size_t(cur - str);
#else
	return strnlen(str, numberOfElements);
#endif
}

__attribute__((format(printf, 3, 4)))
inline int slang_sprintf_s(char * buffer, size_t sizeOfBuffer, const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int rs = vsnprintf(buffer, sizeOfBuffer, format, argptr);
	va_end(argptr);
	return rs;
}

// A patch was submitted to GCC wchar_t support in 2001, so I'm sure we can
// enable this any day now...
// __attribute__((format(wprintf, 3, 4)))
inline int slang_swprintf_s(wchar_t * buffer, size_t sizeOfBuffer, const wchar_t * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int rs = vswprintf(buffer, sizeOfBuffer, format, argptr);
	va_end(argptr);
	return rs;
}

inline void slang_wcscpy_s(wchar_t * strDestination, size_t /*numberOfElements*/, const wchar_t * strSource)
{
	wcscpy(strDestination, strSource);
}
inline void slang_strcpy_s(char * strDestination, size_t /*numberOfElements*/, const char * strSource)
{
	strcpy(strDestination, strSource);
}

inline void slang_wcsncpy_s(wchar_t * strDestination, size_t /*numberOfElements*/, const wchar_t * strSource, size_t count)
{
	wcsncpy(strDestination, strSource, count);
}
inline void slang_strncpy_s(char * strDestination, size_t /*numberOfElements*/, const char * strSource, size_t count)
{
	strncpy(strDestination, strSource, count);
}

#define memcpy_s slang_memcpy_s
#define fopen_s slang_fopen_s
#define fread_s slang_fread_s
#define wcsnlen_s slang_wcsnlen_s
#define strnlen_s slang_strnlen_s
#define swprintf_s slang_swprintf_s
#define wcscpy_s slang_wcscpy_s
#define strcpy_s slang_strcpy_s
#define wcsncpy_s slang_wcsncpy_s
#define strncpy_s slang_strncpy_s
#define sprintf_s slang_sprintf_s

#endif
#endif
