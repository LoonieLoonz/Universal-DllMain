#include "universal_dll.h"

// ============================================================================
// Platform and CRT Detection
// ============================================================================

#ifdef NO_CRT
    #define NOCRT_IMPL
    #include "nocrt.h"
#else
    #include <windows.h>
    #include <winternl.h>
#endif

// ============================================================================
// Global State
// ============================================================================

static HINSTANCE g_hModule = NULL;
static BOOL g_bInitialized = FALSE;
static CRITICAL_SECTION g_InitLock;
static BOOL g_bLockInitialized = FALSE;

// ============================================================================
// Forward Declarations
// ============================================================================

BOOL InitializeLibrary(HINSTANCE hinstDLL, DWORD dwReason);
void CleanupLibrary(HINSTANCE hinstDLL, DWORD dwReason);
void ThreadAttachHandler(HINSTANCE hinstDLL);
void ThreadDetachHandler(HINSTANCE hinstDLL);

// ============================================================================
// TLS Callback (Early Initialization)
// ============================================================================

void NTAPI TlsCallback(PVOID DllHandle, DWORD Reason, PVOID Reserved)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            // Early process initialization before DllMain
            // Useful for manual map scenarios
            break;
            
        case DLL_THREAD_ATTACH:
            if (g_bInitialized)
                ThreadAttachHandler((HINSTANCE)DllHandle);
            break;
            
        case DLL_THREAD_DETACH:
            if (g_bInitialized)
                ThreadDetachHandler((HINSTANCE)DllHandle);
            break;
            
        case DLL_PROCESS_DETACH:
            // Early cleanup
            break;
    }
}

// TLS callback registration (works with manual map and all loader variants)
#ifdef _WIN64
    #pragma comment(linker, "/INCLUDE:_tls_used")
    #pragma comment(linker, "/INCLUDE:tls_callback_func")
    #pragma const_seg(".CRT$XLB")
    extern "C" const PIMAGE_TLS_CALLBACK tls_callback_func = TlsCallback;
    #pragma const_seg()
#else
    #pragma comment(linker, "/INCLUDE:__tls_used")
    #pragma comment(linker, "/INCLUDE:_tls_callback_func")
    #pragma data_seg(".CRT$XLB")
    PIMAGE_TLS_CALLBACK tls_callback_func = TlsCallback;
    #pragma data_seg()
#endif

// ============================================================================
// Thread-Safe Initialization Helper
// ============================================================================

BOOL EnsureInitialized(HINSTANCE hinstDLL, DWORD dwReason)
{
    // Initialize critical section if needed (thread-safe double-check)
    if (!g_bLockInitialized)
    {
        static volatile LONG lockInit = 0;
        
#ifdef NO_CRT
        if (nocrt_InterlockedCompareExchange(&lockInit, 1, 0) == 0)
#else
        if (InterlockedCompareExchange(&lockInit, 1, 0) == 0)
#endif
        {
            InitializeCriticalSection(&g_InitLock);
            g_bLockInitialized = TRUE;
        }
        else
        {
            // Wait for other thread to initialize
            while (!g_bLockInitialized)
            {
#ifdef NO_CRT
                nocrt_Sleep(1);
#else
                Sleep(1);
#endif
            }
        }
    }

    EnterCriticalSection(&g_InitLock);
    
    BOOL result = TRUE;
    if (!g_bInitialized)
    {
        g_hModule = hinstDLL;
        result = InitializeLibrary(hinstDLL, dwReason);
        if (result)
        {
            g_bInitialized = TRUE;
        }
    }
    
    LeaveCriticalSection(&g_InitLock);
    return result;
}

// ============================================================================
// Standard DllMain Entry Point
// ============================================================================

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            if (!EnsureInitialized(hinstDLL, fdwReason))
                return FALSE;
            
            // Optionally disable thread callbacks for performance
            // Uncomment if you don't need per-thread notifications
            // DisableThreadLibraryCalls(hinstDLL);
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            if (g_bInitialized)
            {
                CleanupLibrary(hinstDLL, fdwReason);
                
                if (g_bLockInitialized)
                {
                    DeleteCriticalSection(&g_InitLock);
                    g_bLockInitialized = FALSE;
                }
                
                g_bInitialized = FALSE;
            }
            break;
        }

        case DLL_THREAD_ATTACH:
            if (g_bInitialized)
                ThreadAttachHandler(hinstDLL);
            break;

        case DLL_THREAD_DETACH:
            if (g_bInitialized)
                ThreadDetachHandler(hinstDLL);
            break;
    }
    
    return TRUE;
}

// ============================================================================
// Alternative Entry Points for Various Injection Methods
// ============================================================================

// Manual map entry point (commonly used signature)
extern "C" UNIVERSAL_DLL_EXPORT BOOL WINAPI DllEntry(
    HINSTANCE hinstDLL, 
    DWORD fdwReason, 
    LPVOID lpvReserved)
{
    return DllMain(hinstDLL, fdwReason, lpvReserved);
}

// CRT startup entry point
extern "C" UNIVERSAL_DLL_EXPORT BOOL WINAPI _DllMainCRTStartup(
    HINSTANCE hinstDLL, 
    DWORD fdwReason, 
    LPVOID lpvReserved)
{
    return DllMain(hinstDLL, fdwReason, lpvReserved);
}

// Reflective DLL injection entry point
extern "C" UNIVERSAL_DLL_EXPORT DWORD WINAPI ReflectiveLoader(LPVOID lpParameter)
{
    HINSTANCE hinstDLL = (HINSTANCE)lpParameter;
    
    if (!hinstDLL)
    {
        // Try to find our own base address
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(ReflectiveLoader, &mbi, sizeof(mbi)))
        {
            hinstDLL = (HINSTANCE)mbi.AllocationBase;
        }
    }
    
    if (DllMain(hinstDLL, DLL_PROCESS_ATTACH, NULL))
    {
        return (DWORD)(ULONG_PTR)hinstDLL;
    }
    
    return 0;
}

// Kernel-mode APC injection callback
extern "C" UNIVERSAL_DLL_EXPORT void NTAPI KernelApcCallback(
    PVOID NormalContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2)
{
    HINSTANCE hinstDLL = (HINSTANCE)NormalContext;
    
    if (hinstDLL)
    {
        DllMain(hinstDLL, DLL_PROCESS_ATTACH, NULL);
    }
}

// Thread hijacking entry point
extern "C" UNIVERSAL_DLL_EXPORT DWORD WINAPI ThreadHijackEntry(LPVOID lpParameter)
{
    HINSTANCE hinstDLL = (HINSTANCE)lpParameter;
    
    if (!hinstDLL)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(ThreadHijackEntry, &mbi, sizeof(mbi)))
        {
            hinstDLL = (HINSTANCE)mbi.AllocationBase;
        }
    }
    
    return DllMain(hinstDLL, DLL_PROCESS_ATTACH, NULL) ? 1 : 0;
}

// LdrLoadDll hook compatible entry
extern "C" UNIVERSAL_DLL_EXPORT NTSTATUS NTAPI LdrLoadDllEntry(
    PWCHAR PathToFile,
    PULONG Flags,
    PUNICODE_STRING ModuleFileName,
    PHANDLE ModuleHandle)
{
    if (ModuleHandle && *ModuleHandle)
    {
        DllMain((HINSTANCE)*ModuleHandle, DLL_PROCESS_ATTACH, NULL);
    }
    
    return 0; // STATUS_SUCCESS
}

// Module base finder (for manual map scenarios)
extern "C" UNIVERSAL_DLL_EXPORT HINSTANCE WINAPI GetModuleBase(void)
{
    if (g_hModule)
        return g_hModule;
    
    // Try to find base address
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(GetModuleBase, &mbi, sizeof(mbi)))
    {
        return (HINSTANCE)mbi.AllocationBase;
    }
    
    return NULL;
}

// ============================================================================
// Exception Handler (Optional)
// ============================================================================

LONG WINAPI VectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    // Handle exceptions during initialization if needed
    // Add your exception handling logic here
    
    return EXCEPTION_CONTINUE_SEARCH;
}

// ============================================================================
// Library Implementation Functions
// ============================================================================

BOOL InitializeLibrary(HINSTANCE hinstDLL, DWORD dwReason)
{
    // =======================================================================
    // YOUR INITIALIZATION CODE HERE
    // =======================================================================
    // This is called exactly once regardless of injection method
    // 
    // Examples:
    // - Set up hooks
    // - Allocate resources
    // - Initialize your library state
    // - Add vectored exception handler if needed:
    //   AddVectoredExceptionHandler(1, VectoredExceptionHandler);
    // 
    // Return FALSE if initialization fails
    // =======================================================================
    
    // Example initialization
    g_hModule = hinstDLL;
    
    return TRUE;
}

void CleanupLibrary(HINSTANCE hinstDLL, DWORD dwReason)
{
    // =======================================================================
    // YOUR CLEANUP CODE HERE
    // =======================================================================
    // Remove hooks, free resources, etc.
    // =======================================================================
}

void ThreadAttachHandler(HINSTANCE hinstDLL)
{
    // =======================================================================
    // PER-THREAD INITIALIZATION
    // =======================================================================
    // Called for each new thread in the process
    // =======================================================================
}

void ThreadDetachHandler(HINSTANCE hinstDLL)
{
    // =======================================================================
    // PER-THREAD CLEANUP
    // =======================================================================
    // Called when threads exit
    // =======================================================================
}
