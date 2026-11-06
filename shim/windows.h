#pragma once
// The few Windows types and calls BWAPI's in-game code uses, for a wasm32 build that shares its memory with
// the game (1.16.1). Sizes match 32-bit Windows (long is 32-bit on wasm32 too).
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <strings.h>

#define WINAPI
#define APIENTRY
#define CALLBACK
#define __stdcall
#define __fastcall
#define __cdecl
#define __thiscall
#define __declspec(x)
#define __forceinline inline
#define FAR
#define NEAR
#define CONST const
#define IN
#define OUT
#define OPTIONAL
#define MAX_PATH 260
#define TRUE 1
#define FALSE 0
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef int BOOL;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef void VOID;
typedef void *LPVOID, *PVOID, *HANDLE, *HMODULE, *HINSTANCE, *HWND, *HDC, *HGDIOBJ, *HBITMAP, *HFONT, *HKEY, *HMENU, *HICON, *HCURSOR, *HBRUSH, *HPALETTE;
typedef const void *LPCVOID;
typedef char *LPSTR, *PSTR;
typedef const char *LPCSTR, *PCSTR;
typedef BYTE *LPBYTE, *PBYTE;
typedef DWORD *LPDWORD, *PDWORD;
typedef WORD *LPWORD;
typedef BOOL *LPBOOL;
typedef LONG *LPLONG;
typedef intptr_t LPARAM, LRESULT, INT_PTR, LONG_PTR;
typedef uintptr_t WPARAM, UINT_PTR, ULONG_PTR, DWORD_PTR, SIZE_T;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef union { struct { DWORD LowPart; LONG HighPart; }; LONGLONG QuadPart; } LARGE_INTEGER;
typedef struct { LONG x, y; } POINT, *LPPOINT;
typedef struct { LONG left, top, right, bottom; } RECT, *LPRECT;
typedef const RECT *LPCRECT;
typedef struct { LONG cx, cy; } SIZE, *LPSIZE;
typedef struct { BYTE peRed, peGreen, peBlue, peFlags; } PALETTEENTRY, *LPPALETTEENTRY;
typedef struct { DWORD dwLowDateTime, dwHighDateTime; } FILETIME;
typedef struct { void *p; } CRITICAL_SECTION;

struct IDirectDraw; struct IDirectDrawSurface; struct IDirectDrawPalette;
typedef IDirectDraw *LPDIRECTDRAW;
typedef IDirectDrawSurface *LPDIRECTDRAWSURFACE;
typedef IDirectDrawPalette *LPDIRECTDRAWPALETTE;

#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((DWORD_PTR)(l) >> 16) & 0xffff))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define MB_OK 0
#define MB_ICONERROR 0x10
#define MB_ICONINFORMATION 0x40

// Calls BWAPI makes, answered by the host (bwapi/host.cpp) or here.
extern "C" DWORD GetTickCount();
inline int MessageBoxA(HWND, LPCSTR text, LPCSTR caption, UINT) { fprintf(stderr, "[%s] %s\n", caption ? caption : "", text ? text : ""); return 1; }
#define MessageBox MessageBoxA
inline void OutputDebugStringA(LPCSTR s) { fputs(s, stderr); }
#define OutputDebugString OutputDebugStringA
inline void Sleep(DWORD) {}

#include "msvc-crt.h"
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define stricmp strcasecmp

// storm.h, Server.h
#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long
#define NO_ERROR 0
#define ERROR_SUCCESS 0
typedef void *HRGN, *PSID, *PACL, *PSECURITY_DESCRIPTOR, *HHOOK;
typedef DWORD LCID;
typedef WORD LANGID;
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef struct { unsigned short sa_family; char sa_data[14]; } SOCKADDR;
typedef struct { DWORD nLength; LPVOID lpSecurityDescriptor; BOOL bInheritHandle; } SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
typedef struct { BYTE Value[6]; } SID_IDENTIFIER_AUTHORITY;

// Declarations the headers of BWAPI's Windows layer (Detours.h, WMode.h, NewHackUtil.h) name. Nothing calls them in
// this build; the files that define them are not compiled.
typedef FILETIME *LPFILETIME;
typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID);
typedef intptr_t (*FARPROC)();
typedef struct { DWORD dwFileAttributes; FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime; DWORD nFileSizeHigh, nFileSizeLow, dwReserved0, dwReserved1; CHAR cFileName[MAX_PATH]; CHAR cAlternateFileName[14]; } WIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef struct { BYTE rgbBlue, rgbGreen, rgbRed, rgbReserved; } RGBQUAD;
typedef struct { DWORD biSize; LONG biWidth, biHeight; WORD biPlanes, biBitCount; DWORD biCompression, biSizeImage; LONG biXPelsPerMeter, biYPelsPerMeter; DWORD biClrUsed, biClrImportant; } BITMAPINFOHEADER;
typedef struct { DWORD Characteristics, TimeDateStamp, ForwarderChain, Name, FirstThunk; } IMAGE_IMPORT_DESCRIPTOR;
typedef struct { DWORD u1; } IMAGE_THUNK_DATA32;
typedef struct { WORD e_magic; } IMAGE_DOS_HEADER;
HANDLE FindFirstFileA(LPCSTR, LPWIN32_FIND_DATAA);
BOOL DeleteFileA(LPCSTR);
DWORD GetFileAttributesA(LPCSTR);
HANDLE CreateFileA(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
HWND CreateWindowExA(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
HANDLE CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
HANDLE CreateEventA(LPSECURITY_ATTRIBUTES, BOOL, BOOL, LPCSTR);
void GetSystemTimeAsFileTime(LPFILETIME);
LPSTR GetCommandLineA();

// Calls in the files that are compiled. Module loading finds the bot linked into the same wasm (bwapi/host.cpp);
// the shared memory server is off (serverEnabled false), so its Windows calls never run.
#define TEXT(s) s
#define MAKEFOURCC(a, b, c, d) ((DWORD)(BYTE)(a) | ((DWORD)(BYTE)(b) << 8) | ((DWORD)(BYTE)(c) << 16) | ((DWORD)(BYTE)(d) << 24))
#define _strcmpi strcasecmp
#define PAGE_READWRITE 4
#define FILE_MAP_WRITE 2
#define FILE_MAP_READ 4
#define ERROR_ALREADY_EXISTS 183
#define VK_LBUTTON 1
#define VK_RBUTTON 2
#define VK_MBUTTON 4
#define VK_RETURN 13
#define WM_CHAR 0x102
#define WM_LBUTTONDOWN 0x201
#define WM_LBUTTONUP 0x202
DWORD GetModuleFileNameA(HMODULE, LPSTR, DWORD);
HMODULE LoadLibraryA(LPCSTR);
#define LoadLibrary LoadLibraryA
FARPROC GetProcAddress(HMODULE, LPCSTR);
BOOL FreeLibrary(HMODULE);
inline DWORD GetLastError() { return 0; }
inline DWORD GetCurrentProcessId() { return 1; }
inline short GetKeyState(int) { return 0; }
inline BOOL PostMessage(HWND, UINT, WPARAM, LPARAM) { return FALSE; }
#define PostMessageA PostMessage
inline BOOL SetEnvironmentVariableA(LPCSTR, LPCSTR) { return TRUE; }
inline HANDLE CreateFileMappingA(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCSTR) { return nullptr; }
inline LPVOID MapViewOfFile(HANDLE, DWORD, DWORD, DWORD, SIZE_T) { return nullptr; }
inline BOOL UnmapViewOfFile(LPCVOID) { return TRUE; }
inline BOOL CloseHandle(HANDLE) { return TRUE; }
#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0

// Timers (BWEM's winutils.cpp). Answered by the host's clock, as GetTickCount is.
BOOL QueryPerformanceFrequency(LARGE_INTEGER *);
BOOL QueryPerformanceCounter(LARGE_INTEGER *);
inline HWND GetForegroundWindow() { return nullptr; }
inline int GetWindowTextA(HWND, LPSTR s, int n) { if (s && n) s[0] = 0; return 0; }
#define GetWindowText GetWindowTextA
