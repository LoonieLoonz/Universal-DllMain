#ifndef UNIVERSAL_DLL_H
#define UNIVERSAL_DLL_H

// ============================================================================
// Platform Detection
// ============================================================================

#ifndef _WIN32
    #error "This library is Windows-only"
#endif

// ============================================================================
// Export/Import Macros
// ============================================================================

#ifdef UNIVERSAL_DLL_EXPORTS
    #define UNIVERSAL_DLL_EXPORT __declspec(dllexport)
#else
    #define UNIVERSAL_DLL_EXPORT __declspec(dllimport)
#endif

// ============================================================================
// Compiler Detection
// ============================================================================

#ifdef _MSC_VER
    #define UNIVERSAL_DLL_MSVC 1
    #define UNIVERSAL_DLL_COMPILER_VERSION _MSC_VER
#else
    #define UNIVERSAL_DLL_MSVC 0
    #define UNIVERSAL_DLL_COMPILER_VERSION 0
#endif

// ============================================================================
// CRT Detection
// ============================================================================

#ifdef NO_CRT
    #define UNIVERSAL_DLL_NO_CRT 1
#else
    #define UNIVERSAL_DLL_NO_CRT 0
#endif

// ============================================================================
// Architecture Detection
// ============================================================================

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64)
    #define UNIVERSAL_DLL_X64 1
    #define UNIVERSAL_DLL_X86 0
#elif defined(_WIN32) || defined(_M_IX86)
    #define UNIVERSAL_DLL_X64 0
    #define UNIVERSAL_DLL_X86 1
#else
    #error "Unknown architecture"
#endif

// ============================================================================
// Calling Conventions
// ============================================================================

#define UNIVERSAL_CALLCONV __stdcall
#define UNIVERSAL_FASTCALL __fastcall
#define UNIVERSAL_CDECL    __cdecl

// ============================================================================
// C++ Detection
// ============================================================================

#ifdef __cplusplus
    #define UNIVERSAL_EXTERN_C extern "C"
    #define UNIVERSAL_EXTERN_C_BEGIN extern "C" {
    #define UNIVERSAL_EXTERN_C_END }
#else
    #define UNIVERSAL_EXTERN_C
    #define UNIVERSAL_EXTERN_C_BEGIN
    #define UNIVERSAL_EXTERN_C_END
#endif

// ============================================================================
// Windows Headers (conditional based on CRT)
// ============================================================================

#ifdef NO_CRT
    // Minimal Windows types for no-CRT build
    typedef void* PVOID;
    typedef void* LPVOID;
    typedef unsigned long DWORD;
    typedef unsigned long ULONG;
    typedef unsigned long* PULONG;
    typedef int BOOL;
    typedef unsigned short WORD;
    typedef unsigned char BYTE;
    typedef long LONG;
    typedef long NTSTATUS;
    typedef wchar_t WCHAR;
    typedef WCHAR* PWCHAR;
    typedef void* HANDLE;
    typedef HANDLE* PHANDLE;
    typedef HANDLE HINSTANCE;
    
    #ifndef TRUE
        #define TRUE 1
    #endif
    
    #ifndef FALSE
        #define FALSE 0
    #endif
    
    #ifndef NULL
        #define NULL ((void*)0)
    #endif
    
    // DLL notification reasons
    #define DLL_PROCESS_ATTACH 1
    #define DLL_THREAD_ATTACH  2
    #define DLL_THREAD_DETACH  3
    #define DLL_PROCESS_DETACH 0
    
    // Calling conventions
    #ifndef WINAPI
        #define WINAPI __stdcall
    #endif
    
    #ifndef NTAPI
        #define NTAPI __stdcall
    #endif
    
    #ifndef APIENTRY
        #define APIENTRY WINAPI
    #endif
    
    // Unicode string structure
    typedef struct _UNICODE_STRING {
        WORD Length;
        WORD MaximumLength;
        PWCHAR Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;
    
    // TLS callback type
    typedef void (NTAPI *PIMAGE_TLS_CALLBACK)(
        PVOID DllHandle,
        DWORD Reason,
        PVOID Reserved
    );
    
    // Critical section (simplified)
    typedef struct _CRITICAL_SECTION {
        PVOID DebugInfo;
        LONG LockCount;
        LONG RecursionCount;
        HANDLE OwningThread;
        HANDLE LockSemaphore;
        ULONG SpinCount;
    } CRITICAL_SECTION, *PCRITICAL_SECTION;
    
    // Memory basic information
    typedef struct _MEMORY_BASIC_INFORMATION {
        PVOID BaseAddress;
        PVOID AllocationBase;
        DWORD AllocationProtect;
        SIZE_T RegionSize;
        DWORD State;
        DWORD Protect;
        DWORD Type;
    } MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
    
    // Exception record
    typedef struct _EXCEPTION_RECORD {
        DWORD ExceptionCode;
        DWORD ExceptionFlags;
        struct _EXCEPTION_RECORD* ExceptionRecord;
        PVOID ExceptionAddress;
        DWORD NumberParameters;
        ULONG ExceptionInformation[15];
    } EXCEPTION_RECORD, *PEXCEPTION_RECORD;
    
    // Exception pointers
    typedef struct _EXCEPTION_POINTERS {
        PEXCEPTION_RECORD ExceptionRecord;
        PVOID ContextRecord;
    } EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
    
    // Exception disposition
    #define EXCEPTION_CONTINUE_SEARCH 0
    #define EXCEPTION_EXECUTE_HANDLER 1
    #define EXCEPTION_CONTINUE_EXECUTION -1
    
#else
    // Include full Windows headers when using CRT
    #include <windows.h>
    #include <winternl.h>
#endif

// ============================================================================
// Public API Functions
// ============================================================================

UNIVERSAL_EXTERN_C_BEGIN

// Get the module base address
UNIVERSAL_DLL_EXPORT HINSTANCE WINAPI GetModuleBase(void);

// Standard DLL entry points (exported for compatibility)
UNIVERSAL_DLL_EXPORT BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
);

UNIVERSAL_DLL_EXPORT BOOL WINAPI DllEntry(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
);

// Alternative entry points for various injection methods
UNIVERSAL_DLL_EXPORT DWORD WINAPI ReflectiveLoader(LPVOID lpParameter);

UNIVERSAL_DLL_EXPORT void NTAPI KernelApcCallback(
    PVOID NormalContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
);

UNIVERSAL_DLL_EXPORT DWORD WINAPI ThreadHijackEntry(LPVOID lpParameter);

UNIVERSAL_DLL_EXPORT NTSTATUS NTAPI LdrLoadDllEntry(
    PWCHAR PathToFile,
    PULONG Flags,
    PUNICODE_STRING ModuleFileName,
    PHANDLE ModuleHandle
);

UNIVERSAL_EXTERN_C_END

// ============================================================================
// Version Information
// ============================================================================

#define UNIVERSAL_DLL_VERSION_MAJOR 1
#define UNIVERSAL_DLL_VERSION_MINOR 0
#define UNIVERSAL_DLL_VERSION_PATCH 0

#define UNIVERSAL_DLL_VERSION \
    ((UNIVERSAL_DLL_VERSION_MAJOR << 16) | \
     (UNIVERSAL_DLL_VERSION_MINOR << 8) | \
     UNIVERSAL_DLL_VERSION_PATCH)

#endif // UNIVERSAL_DLL_H