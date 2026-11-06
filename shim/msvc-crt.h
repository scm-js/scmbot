#pragma once
// MSVC's "secure" C library calls, both of their forms (a pointer and a size, or an array whose size the template
// takes), for BWAPI's sources (through windows.h) and for bots (scmbot's msvc-compat.h).
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif
#define _vsnprintf vsnprintf
#define _snprintf snprintf

inline int vsprintf_s(char *buffer, size_t size, const char *format, va_list args) { return vsnprintf(buffer, size, format, args); }
template <size_t N> inline int vsprintf_s(char (&buffer)[N], const char *format, va_list args) { return vsnprintf(buffer, N, format, args); }
inline int vsnprintf_s(char *buffer, size_t size, size_t, const char *format, va_list args) { return vsnprintf(buffer, size, format, args); }
template <size_t N> inline int vsnprintf_s(char (&buffer)[N], size_t, const char *format, va_list args) { return vsnprintf(buffer, N, format, args); }
inline int sprintf_s(char *buffer, size_t size, const char *format, ...) { va_list a; va_start(a, format); int n = vsnprintf(buffer, size, format, a); va_end(a); return n; }
template <size_t N> inline int sprintf_s(char (&buffer)[N], const char *format, ...) { va_list a; va_start(a, format); int n = vsnprintf(buffer, N, format, a); va_end(a); return n; }
inline int _snprintf_s(char *buffer, size_t size, size_t, const char *format, ...) { va_list a; va_start(a, format); int n = vsnprintf(buffer, size, format, a); va_end(a); return n; }
inline int strcpy_s(char *dst, size_t size, const char *src) { if (!size) return 1; strncpy(dst, src, size - 1); dst[size - 1] = 0; return 0; }
template <size_t N> inline int strcpy_s(char (&dst)[N], const char *src) { return strcpy_s(dst, N, src); }
inline int strncpy_s(char *dst, size_t size, const char *src, size_t count) { if (!size) return 1; size_t n = count < size - 1 ? count : size - 1; strncpy(dst, src, n); dst[n] = 0; return 0; }
template <size_t N> inline int strncpy_s(char (&dst)[N], const char *src, size_t count) { return strncpy_s(dst, N, src, count); }
inline int strcat_s(char *dst, size_t size, const char *src) { size_t l = strlen(dst); if (l + 1 >= size) return 1; strncat(dst, src, size - l - 1); return 0; }
template <size_t N> inline int strcat_s(char (&dst)[N], const char *src) { return strcat_s(dst, N, src); }
inline int localtime_s(struct tm *out, const time_t *t) { return localtime_r(t, out) ? 0 : 1; }
inline int gmtime_s(struct tm *out, const time_t *t) { return gmtime_r(t, out) ? 0 : 1; }
inline int fopen_s(FILE **f, const char *name, const char *mode) { *f = fopen(name, mode); return *f ? 0 : 1; }
