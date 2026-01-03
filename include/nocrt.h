#ifndef NOCRT_H
#define NOCRT_H

#ifdef NO_CRT

// ============================================================================
// No-CRT Helper Functions
// ============================================================================
// This file provides minimal replacements for CRT functions when building
// without the C runtime library.
// ============================================================================

#include "universal_dll.h"

// ============================================================================
// Intrinsic Functions (compiler built-ins)
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Memory operations
#pragma intrinsic(memcpy, memset, memcmp)
void* memcpy(void* dest, const void* src, size_t count);
void* memset(void* dest, int value, size_t count);
int memcmp(const void* buf1, const void* buf2, size_t count);

#ifdef __cplusplus
}
#endif

// ============================================================================
// Windows API Imports (ntdll.dll and kernel32.dll)
// ============================================================================

// These are the minimal Windows APIs we need without CRT
typedef LONG (WINAPI* pfnRtlInitializeCriticalSection)(PCRITICAL_SECTION);
typedef LONG (WINAPI* pfnRtlDeleteCriticalSection)(PCRITICAL_SECTION);
typedef LONG (WINAPI* pfnRtlEnterCriticalSection)(PCRITICAL_SECTION);
typedef LONG (WINAPI* pfnRtlLeaveCriticalSection)(PCRITICAL_SECTION);
typedef LONG (WINAPI* pfnRtlInterlockedCompareExchange)(volatile LONG*, LONG, LONG);
typedef void (WINAPI* pfnSleep)(DWORD);
typedef SIZE_T (WINAPI* pfnVirtualQuery)(LPCVOID, PMEMORY_BASIC_INFORMATION, SIZE_T);

// ============================================================================
// Function Pointer Storage
// ============================================================================

static pfnRtlInitializeCriticalSection g_pfnInitializeCriticalSection = NULL;
static pfnRtlDeleteCriticalSection g_pfnDeleteCriticalSection = NULL;
static pfnRtlEnterCriticalSection g_pfnEnterCriticalSection = NULL;
static pfnRtlLeaveCriticalSection g_pfnLeaveCriticalSection = NULL;
static pfnRtlInterlockedCompareExchange g_pfnInterlockedCompareExchange = NULL;
static pfnSleep g_pfnSleep = NULL;
static pfnVirtualQuery g_pfnVirtualQuery = NULL;

// ============================================================================
// Manual Function Resolution
// ============================================================================

// Get module base by walking PEB
static inline PVOID nocrt_GetModuleBase(const WCHAR* moduleName)
{
#ifdef _WIN64
    PVOID peb = (PVOID)__readgsqword(0x60);
#else
    PVOID peb = (PVOID)__readfsdword(0x30);
#endif
    
    // This is a simplified version - in production you'd walk the PEB properly
    // For now, we'll use a more direct approach with kernel32 and ntdll
    
    return NULL; // Implement PEB walking if needed
}

// Simple string comparison for ASCII
static inline int nocrt_strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Get procedure address manually
static inline PVOID nocrt_GetProcAddress(HINSTANCE hModule, const char* procName)
{
    if (!hModule || !procName)
        return NULL;
    
    // Walk PE headers to find export table
    unsigned char* baseAddr = (unsigned char*)hModule;
    
    // DOS header
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)baseAddr;
    if (dosHeader->e_magic != 0x5A4D) // "MZ"
        return NULL;
    
    // NT headers
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(baseAddr + dosHeader->e_lfanew);
    if (ntHeaders->Signature != 0x4550) // "PE"
        return NULL;
    
    // Export directory
    IMAGE_DATA_DIRECTORY* exportDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (exportDir->VirtualAddress == 0)
        return NULL;
    
    IMAGE_EXPORT_DIRECTORY* exports = (IMAGE_EXPORT_DIRECTORY*)(baseAddr + exportDir->VirtualAddress);
    
    DWORD* nameTable = (DWORD*)(baseAddr + exports->AddressOfNames);
    WORD* ordinalTable = (WORD*)(baseAddr + exports->AddressOfNameOrdinals);
    DWORD* addressTable = (DWORD*)(baseAddr + exports->AddressOfFunctions);
    
    // Search for function name
    for (DWORD i = 0; i < exports->NumberOfNames; i++)
    {
        const char* exportName = (const char*)(baseAddr + nameTable[i]);
        if (nocrt_strcmp(exportName, procName) == 0)
        {
            WORD ordinal = ordinalTable[i];
            DWORD rva = addressTable[ordinal];
            return (PVOID)(baseAddr + rva);
        }
    }
    
    return NULL;
}

// Initialize no-CRT function pointers
static inline BOOL nocrt_Initialize(void)
{
    static BOOL initialized = FALSE;
    if (initialized)
        return TRUE;
    
    // Get ntdll.dll handle (it's always loaded)
    // We'll use a simplified approach - in production you'd walk the PEB
    HINSTANCE hNtdll = (HINSTANCE)0x77000000; // Typical ntdll base (will vary)
    HINSTANCE hKernel32 = (HINSTANCE)0x75000000; // Typical kernel32 base (will vary)
    
    // Better approach: use __ImageBase which is always valid
    extern IMAGE_DOS_HEADER __ImageBase;
    
    // For now, we'll use direct syscalls or intrinsics
    // In a real implementation, you'd resolve these properly
    
    initialized = TRUE;
    return TRUE;
}

// ============================================================================
// Wrapper Functions
// ============================================================================

static inline void InitializeCriticalSection(PCRITICAL_SECTION lpCriticalSection)
{
    if (!g_pfnInitializeCriticalSection)
        nocrt_Initialize();
    
    // Simplified initialization
    memset(lpCriticalSection, 0, sizeof(CRITICAL_SECTION));
}

static inline void DeleteCriticalSection(PCRITICAL_SECTION lpCriticalSection)
{
    if (!g_pfnDeleteCriticalSection)
        nocrt_Initialize();
    
    // Simplified cleanup
    memset(lpCriticalSection, 0, sizeof(CRITICAL_SECTION));
}

static inline void EnterCriticalSection(PCRITICAL_SECTION lpCriticalSection)
{
    if (!g_pfnEnterCriticalSection)
        nocrt_Initialize();
    
    // Simplified - in production use proper synchronization
    while (__InterlockedCompareExchange(&lpCriticalSection->LockCount, 1, 0) != 0)
    {
        // Spin
    }
}

static inline void LeaveCriticalSection(PCRITICAL_SECTION lpCriticalSection)
{
    if (!g_pfnLeaveCriticalSection)
        nocrt_Initialize();
    
    __InterlockedExchange(&lpCriticalSection->LockCount, 0);
}

static inline LONG nocrt_InterlockedCompareExchange(volatile LONG* Destination, LONG Exchange, LONG Comparand)
{
    return __InterlockedCompareExchange(Destination, Exchange, Comparand);
}

static inline void nocrt_Sleep(DWORD dwMilliseconds)
{
    // Simple busy wait for no-CRT (not ideal but works)
    volatile DWORD i, j;
    for (i = 0; i < dwMilliseconds * 1000; i++)
    {
        for (j = 0; j < 100; j++)
        {
            // Busy loop
        }
    }
}

static inline SIZE_T VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
{
    // This would need to be implemented via direct syscall or resolved from kernel32
    // For now, return failure
    return 0;
}

// ============================================================================
// Memory Allocation (if needed)
// ============================================================================

// Simple heap allocation using VirtualAlloc
static inline PVOID nocrt_malloc(SIZE_T size)
{
    // Would need to resolve VirtualAlloc from kernel32
    // For now, this is a placeholder
    return NULL;
}

static inline void nocrt_free(PVOID ptr)
{
    // Would need to resolve VirtualFree from kernel32
    // For now, this is a placeholder
}

#endif // NO_CRT

#endif // NOCRT_H