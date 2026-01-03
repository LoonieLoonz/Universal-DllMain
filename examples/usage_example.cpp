#include <windows.h>
#include <stdio.h>
#include "universal_dll.h"

// ============================================================================
// Example Usage of Universal DLL with Various Injection Methods
// ============================================================================

// Example 1: Standard LoadLibrary
void Example_LoadLibrary()
{
    printf("=== Example 1: Standard LoadLibrary ===\n");
    
    HMODULE hDll = LoadLibraryA("UniversalDLL.dll");
    if (hDll)
    {
        printf("✓ DLL loaded successfully via LoadLibrary\n");
        printf("  Base address: 0x%p\n", hDll);
        
        // Use the DLL...
        
        FreeLibrary(hDll);
        printf("✓ DLL unloaded\n");
    }
    else
    {
        printf("✗ Failed to load DLL (error: %lu)\n", GetLastError());
    }
    printf("\n");
}

// Example 2: Manual Map (simplified demonstration)
void Example_ManualMap()
{
    printf("=== Example 2: Manual Map Injection ===\n");
    
    // This is a simplified example - real manual mapping is more complex
    // and involves parsing PE headers, relocations, imports, etc.
    
    HMODULE hDll = LoadLibraryA("UniversalDLL.dll");
    if (hDll)
    {
        // Get the DllEntry export
        typedef BOOL (WINAPI* DllEntryProc)(HINSTANCE, DWORD, LPVOID);
        DllEntryProc pDllEntry = (DllEntryProc)GetProcAddress(hDll, "DllEntry");
        
        if (pDllEntry)
        {
            printf("✓ Found DllEntry export at 0x%p\n", pDllEntry);
            
            // In real manual mapping, you would:
            // 1. Allocate memory in target process
            // 2. Copy DLL sections
            // 3. Fix relocations
            // 4. Resolve imports
            // 5. Call DllEntry in remote process
            
            // For demonstration, call it locally:
            BOOL result = pDllEntry(hDll, DLL_PROCESS_ATTACH, NULL);
            printf("✓ DllEntry returned: %d\n", result);
        }
        else
        {
            printf("✗ Failed to find DllEntry export\n");
        }
        
        FreeLibrary(hDll);
    }
    printf("\n");
}

// Example 3: Reflective DLL Injection (simplified)
void Example_ReflectiveLoader()
{
    printf("=== Example 3: Reflective Loader ===\n");
    
    HMODULE hDll = LoadLibraryA("UniversalDLL.dll");
    if (hDll)
    {
        typedef DWORD (WINAPI* ReflectiveLoaderProc)(LPVOID);
        ReflectiveLoaderProc pLoader = (ReflectiveLoaderProc)GetProcAddress(hDll, "ReflectiveLoader");
        
        if (pLoader)
        {
            printf("✓ Found ReflectiveLoader export at 0x%p\n", pLoader);
            
            // Call reflective loader (passing DLL base as parameter)
            DWORD result = pLoader((LPVOID)hDll);
            printf("✓ ReflectiveLoader returned: 0x%08X\n", result);
        }
        else
        {
            printf("✗ Failed to find ReflectiveLoader export\n");
        }
        
        FreeLibrary(hDll);
    }
    printf("\n");
}

// Example 4: Get Module Base
void Example_GetModuleBase()
{
    printf("=== Example 4: Get Module Base ===\n");
    
    HMODULE hDll = LoadLibraryA("UniversalDLL.dll");
    if (hDll)
    {
        typedef HINSTANCE (WINAPI* GetModuleBaseProc)(void);
        GetModuleBaseProc pGetModuleBase = (GetModuleBaseProc)GetProcAddress(hDll, "GetModuleBase");
        
        if (pGetModuleBase)
        {
            HINSTANCE base = pGetModuleBase();
            printf("✓ Module base from GetModuleBase: 0x%p\n", base);
            printf("  LoadLibrary returned: 0x%p\n", hDll);
            printf("  Match: %s\n", (base == hDll) ? "Yes" : "No");
        }
        else
        {
            printf("✗ Failed to find GetModuleBase export\n");
        }
        
        FreeLibrary(hDll);
    }
    printf("\n");
}

// Example 5: Thread Hijack Entry (demonstration)
void Example_ThreadHijackEntry()
{
    printf("=== Example 5: Thread Hijack Entry ===\n");
    
    HMODULE hDll = LoadLibraryA("UniversalDLL.dll");
    if (hDll)
    {
        typedef DWORD (WINAPI* ThreadHijackEntryProc)(LPVOID);
        ThreadHijackEntryProc pEntry = (ThreadHijackEntryProc)GetProcAddress(hDll, "ThreadHijackEntry");
        
        if (pEntry)
        {
            printf("✓ Found ThreadHijackEntry export at 0x%p\n", pEntry);
            
            // In real thread hijacking, you would:
            // 1. Suspend target thread
            // 2. Get thread context
            // 3. Modify RIP/EIP to point to this function
            // 4. Set parameter (RCX/stack) to DLL base
            // 5. Resume thread
            
            // For demonstration, call it directly:
            DWORD result = pEntry((LPVOID)hDll);
            printf("✓ ThreadHijackEntry returned: %lu\n", result);
        }
        else
        {
            printf("✗ Failed to find ThreadHijackEntry export\n");
        }
        
        FreeLibrary(hDll);
    }
    printf("\n");
}

// Example 6: List All Exports
void Example_ListExports()
{
    printf("=== Example 6: List All Exports ===\n");
    
    HMODULE hDll = LoadLibraryA("UniversalDLL.dll");
    if (hDll)
    {
        printf("✓ DLL loaded, exports:\n");
        
        const char* exports[] = {
            "DllMain",
            "DllEntry",
            "_DllMainCRTStartup",
            "ReflectiveLoader",
            "ThreadHijackEntry",
            "KernelApcCallback",
            "LdrLoadDllEntry",
            "GetModuleBase"
        };
        
        for (int i = 0; i < sizeof(exports) / sizeof(exports[0]); i++)
        {
            FARPROC proc = GetProcAddress(hDll, exports[i]);
            if (proc)
            {
                printf("  ✓ %-25s -> 0x%p\n", exports[i], proc);
            }
            else
            {
                printf("  ✗ %-25s -> Not found\n", exports[i]);
            }
        }
        
        FreeLibrary(hDll);
    }
    printf("\n");
}

// Example 7: Remote Process Injection (CreateRemoteThread)
void Example_RemoteThreadInjection()
{
    printf("=== Example 7: Remote Thread Injection (Local Demo) ===\n");
    
    // For demonstration, we'll inject into ourselves
    HANDLE hProcess = GetCurrentProcess();
    
    // Allocate memory for DLL path
    const char* dllPath = "UniversalDLL.dll";
    SIZE_T pathSize = strlen(dllPath) + 1;
    
    LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, pathSize, 
                                        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemotePath)
    {
        SIZE_T written;
        if (WriteProcessMemory(hProcess, pRemotePath, dllPath, pathSize, &written))
        {
            // Get LoadLibraryA address (same in all processes)
            LPTHREAD_START_ROUTINE pLoadLibrary = 
                (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
            
            if (pLoadLibrary)
            {
                printf("✓ Creating remote thread...\n");
                HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, 
                                                    pLoadLibrary, pRemotePath, 0, NULL);
                if (hThread)
                {
                    WaitForSingleObject(hThread, INFINITE);
                    
                    DWORD exitCode;
                    GetExitCodeThread(hThread, &exitCode);
                    printf("✓ Remote thread completed, DLL base: 0x%08X\n", exitCode);
                    
                    CloseHandle(hThread);
                }
                else
                {
                    printf("✗ Failed to create remote thread (error: %lu)\n", GetLastError());
                }
            }
        }
        
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    }
    printf("\n");
}

// Main function
int main()
{
    printf("============================================================================\n");
    printf("Universal DLL - Usage Examples\n");
    printf("============================================================================\n\n");
    
    // Run examples
    Example_LoadLibrary();
    Example_ManualMap();
    Example_ReflectiveLoader();
    Example_GetModuleBase();
    Example_ThreadHijackEntry();
    Example_ListExports();
    Example_RemoteThreadInjection();
    
    printf("============================================================================\n");
    printf("All examples completed!\n");
    printf("============================================================================\n");
    
    printf("\nPress Enter to exit...");
    getchar();
    
    return 0;
}