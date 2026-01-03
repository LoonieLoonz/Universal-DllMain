# Universal DLL - Injection-Method Agnostic DLL

A universally compatible DLL entry point that supports any loading/injection method including LoadLibrary, manual mapping, kernel injection, and more.

## Features

- ✅ **Universal Compatibility**: Works with LoadLibrary, LdrLoadDll, manual mapping, kernel injection, reflective loading, and more
- ✅ **Multiple Entry Points**: Exports various entry point signatures for different injection methods
- ✅ **CRT Optional**: Can be built with or without C Runtime Library
- ✅ **Thread-Safe**: Ensures single initialization across all loading methods
- ✅ **TLS Callbacks**: Handles early thread creation before DllMain
- ✅ **CMake Build System**: Easy configuration and compilation
- ✅ **MSVC Optimized**: Full support for Microsoft Visual C++ compiler
- ✅ **32/64-bit Support**: Works on both x86 and x64 architectures

## Supported Injection Methods

| Method | Entry Point | Status |
|--------|-------------|--------|
| LoadLibrary/LoadLibraryEx | DllMain | ✅ Supported |
| LdrLoadDll | LdrLoadDllEntry | ✅ Supported |
| LdrpLoadDll | DllMain | ✅ Supported |
| Manual Map | DllEntry | ✅ Supported |
| Reflective DLL Injection | ReflectiveLoader | ✅ Supported |
| Thread Hijacking | ThreadHijackEntry | ✅ Supported |
| Kernel APC Injection | KernelApcCallback | ✅ Supported |
| SetWindowsHookEx | DllMain | ✅ Supported |
| QueueUserAPC | DllMain | ✅ Supported |

## Project Structure

```
UniversalDLL/
├── CMakeLists.txt          # CMake build configuration
├── build.bat               # Windows build script
├── build.sh                # Linux/Unix build script
├── include/
│   ├── universal_dll.h     # Main header file
│   └── nocrt.h            # No-CRT helper functions
├── src/
│   └── dllmain.cpp        # Main DLL implementation
└── README.md              # This file
```

## Requirements

### Windows
- Visual Studio 2019 or later (recommended: VS 2022)
- CMake 3.15 or later
- Windows SDK 10.0 or later

### Linux/Unix (Cross-compilation)
- CMake 3.15 or later
- MinGW-w64 cross-compiler
- Ninja or Make

## Building

### Quick Start (Windows)

```batch
# Build with CRT (Release)
build.bat

# Build without CRT (Release)
build.bat --no-crt

# Build with CRT (Debug)
build.bat --debug --crt
```

### Quick Start (Linux/Unix)

```bash
# Make script executable
chmod +x build.sh

# Build with CRT (Release)
./build.sh

# Build without CRT (Release)
./build.sh --no-crt

# Build with CRT (Debug)
./build.sh --debug --crt
```

### Manual CMake Build

```batch
# Create build directory
mkdir build
cd build

# Configure (with CRT)
cmake .. -G "Visual Studio 17 2022" -DUSE_CRT=ON -DBUILD_SHARED=ON

# Configure (without CRT)
cmake .. -G "Visual Studio 17 2022" -DUSE_CRT=OFF -DBUILD_SHARED=ON

# Build
cmake --build . --config Release

# Output will be in: build/Release/UniversalDLL.dll
```

## Build Options

| CMake Option | Values | Default | Description |
|--------------|--------|---------|-------------|
| USE_CRT | ON/OFF | ON | Use C Runtime Library |
| BUILD_SHARED | ON/OFF | ON | Build as DLL (vs static lib) |
| ENABLE_EXCEPTIONS | ON/OFF | OFF | Enable C++ exceptions |
| CMAKE_BUILD_TYPE | Debug/Release | Release | Build configuration |

## Usage Examples

### Basic DLL Usage

```cpp
// Your initialization code
BOOL InitializeLibrary(HINSTANCE hinstDLL, DWORD dwReason)
{
    // Set up your hooks, allocate resources, etc.
    // This is called exactly once regardless of injection method
    
    return TRUE; // Return FALSE if initialization fails
}

// Your cleanup code
void CleanupLibrary(HINSTANCE hinstDLL, DWORD dwReason)
{
    // Remove hooks, free resources, etc.
}
```

### Standard LoadLibrary

```cpp
HMODULE hDll = LoadLibraryA("UniversalDLL.dll");
if (hDll) {
    // DLL loaded via DllMain entry point
    FreeLibrary(hDll);
}
```

### Manual Map Injection

```cpp
// Manual map the DLL into target process
void* baseAddress = ManualMapDll(processHandle, dllBuffer, dllSize);

// Call the DllEntry export
typedef BOOL (WINAPI* DllEntryProc)(HINSTANCE, DWORD, LPVOID);
DllEntryProc entry = (DllEntryProc)GetRemoteProcAddress(baseAddress, "DllEntry");
RemoteCall(processHandle, entry, baseAddress, DLL_PROCESS_ATTACH, NULL);
```

### Reflective DLL Injection

```cpp
// Allocate memory in target process
void* remoteBuffer = VirtualAllocEx(hProcess, NULL, dllSize, ...);
WriteProcessMemory(hProcess, remoteBuffer, dllBuffer, dllSize, NULL);

// Get ReflectiveLoader address
DWORD loaderOffset = GetExportOffset(dllBuffer, "ReflectiveLoader");
void* remoteLoader = (char*)remoteBuffer + loaderOffset;

// Execute loader
HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, 
    (LPTHREAD_START_ROUTINE)remoteLoader, remoteBuffer, 0, NULL);
```

### Thread Hijacking

```cpp
// Get target thread context
CONTEXT ctx = {0};
ctx.ContextFlags = CONTEXT_CONTROL;
GetThreadContext(hThread, &ctx);

// Inject DLL and set RIP/EIP to ThreadHijackEntry
void* entryPoint = GetRemoteProcAddress(dllBase, "ThreadHijackEntry");
#ifdef _WIN64
    ctx.Rip = (DWORD64)entryPoint;
    ctx.Rcx = (DWORD64)dllBase; // hinstDLL parameter
#else
    ctx.Eip = (DWORD)entryPoint;
    // Push parameter on stack for x86
#endif
SetThreadContext(hThread, &ctx);
```

### Kernel APC Injection

```cpp
// Queue APC to target thread
typedef void (NTAPI* KernelApcCallbackProc)(PVOID, PVOID, PVOID);
KernelApcCallbackProc callback = GetRemoteProcAddress(dllBase, "KernelApcCallback");

NTSTATUS status = NtQueueApcThread(
    hThread,
    (PPS_APC_ROUTINE)callback,
    (PVOID)dllBase,  // NormalContext
    NULL,            // SystemArgument1
    NULL             // SystemArgument2
);
```

## Exported Functions

| Function | Signature | Purpose |
|----------|-----------|---------|
| DllMain | `BOOL WINAPI(HINSTANCE, DWORD, LPVOID)` | Standard entry point |
| DllEntry | `BOOL WINAPI(HINSTANCE, DWORD, LPVOID)` | Manual map entry |
| ReflectiveLoader | `DWORD WINAPI(LPVOID)` | Reflective injection |
| ThreadHijackEntry | `DWORD WINAPI(LPVOID)` | Thread hijacking |
| KernelApcCallback | `void NTAPI(PVOID, PVOID, PVOID)` | Kernel APC injection |
| LdrLoadDllEntry | `NTSTATUS NTAPI(PWCHAR, PULONG, PUNICODE_STRING, PHANDLE)` | LdrLoadDll hook |
| GetModuleBase | `HINSTANCE WINAPI(void)` | Get DLL base address |

## Configuration Macros

The library provides several macros for compile-time configuration:

```cpp
// Check if CRT is being used
#if UNIVERSAL_DLL_NO_CRT
    // No-CRT specific code
#endif

// Check architecture
#if UNIVERSAL_DLL_X64
    // 64-bit specific code
#elif UNIVERSAL_DLL_X86
    // 32-bit specific code
#endif

// Check compiler
#if UNIVERSAL_DLL_MSVC
    // MSVC specific code
#endif
```

## Advanced Features

### TLS Callbacks

TLS callbacks execute before DllMain, allowing you to handle:
- Threads created before DLL_PROCESS_ATTACH
- Early initialization requirements
- Thread-local storage setup

### Thread-Safe Initialization

The library uses critical sections and interlocked operations to ensure:
- Single initialization regardless of injection method
- Safe concurrent thread creation
- No race conditions

### No-CRT Mode

When built without CRT (USE_CRT=OFF):
- Minimal dependencies (ntdll.dll, kernel32.dll only)
- Smaller file size
- No CRT initialization overhead
- Custom implementations of critical functions

## Troubleshooting

### Build Errors

**Error: Cannot open include file 'windows.h'**
- Install Windows SDK
- Verify Visual Studio C++ tools are installed

**Error: LNK2001: unresolved external symbol**
- Check if USE_CRT is set correctly
- Ensure proper libraries are linked

### Runtime Issues

**DLL fails to initialize**
- Check InitializeLibrary return value
- Verify all dependencies are available
- Check Windows Event Viewer for loader errors

**Crashes on injection**
- Ensure correct architecture (x86 vs x64)
- Verify injection method implementation
- Check for conflicting modules

## Performance Considerations

- **DisableThreadLibraryCalls**: Uncomment in DllMain if you don't need per-thread notifications
- **TLS Callbacks**: Minimal overhead, safe to keep enabled
- **CRT vs No-CRT**: No-CRT is smaller but requires manual function resolution

## Security Considerations

- This library is designed for legitimate development and research
- Always respect process boundaries and permissions
- Do not use for malicious purposes
- Test thoroughly in isolated environments

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Test on both x86 and x64
4. Test with and without CRT
5. Submit a pull request

## License

This project is provided as-is for educational and development purposes.

## Acknowledgments

- Inspired by various DLL injection techniques and loaders
- Built with compatibility and robustness in mind
- Designed for real-world usage scenarios

## Contact

For issues, questions, or contributions, please open an issue on the project repository.