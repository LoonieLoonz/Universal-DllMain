# Universal DLL - Quick Start Guide

## 🚀 Get Started in 5 Minutes

### Prerequisites
- Windows 10/11
- Visual Studio 2019 or later (with C++ tools)
- CMake 3.15+

### Option 1: Build with Scripts (Easiest)

#### Windows
```batch
# Build with CRT (recommended for most users)
build.bat

# Build without CRT (smaller size, advanced users)
build.bat --no-crt

# Debug build
build.bat --debug
```

#### Linux/WSL
```bash
chmod +x build.sh
./build.sh
```

**Output**: `build/Release/UniversalDLL.dll`

### Option 2: Visual Studio

```batch
generate_vs_solution.bat
```

Then open `vs_solution/with_crt/UniversalDLL.sln` in Visual Studio.

### Option 3: CMake Presets (Modern CMake)

```batch
# List available presets
cmake --list-presets

# Configure
cmake --preset windows-x64-crt-release

# Build
cmake --build --preset windows-x64-crt-release-build
```

## 📋 Build Configurations

| Configuration | File Size | Dependencies | Use Case |
|--------------|-----------|--------------|----------|
| **With CRT (Release)** | ~50KB | MSVCRT.dll | General purpose, recommended |
| **Without CRT (Release)** | ~20KB | None | Minimal footprint, advanced |
| **With CRT (Debug)** | ~150KB | MSVCRT.dll | Development, debugging |
| **Without CRT (Debug)** | ~80KB | None | Advanced debugging |

## 🎯 Quick Test

After building, run the example:

```batch
cd examples
cl /I../include usage_example.cpp /link /LIBPATH:../build/Release UniversalDLL.lib
usage_example.exe
```

## 📖 Next Steps

1. **Customize**: Edit `src/dllmain.cpp` and implement your logic in:
   - `InitializeLibrary()` - Your initialization code
   - `CleanupLibrary()` - Your cleanup code
   - `ThreadAttachHandler()` - Per-thread setup
   - `ThreadDetachHandler()` - Per-thread cleanup

2. **Build**: Run `build.bat` or `build.sh`

3. **Test**: Use any injection method (LoadLibrary, manual map, etc.)

4. **Deploy**: Your DLL works with ALL injection methods automatically!

## 🔧 Common Build Options

```batch
# Release with CRT
build.bat --release --crt

# Release without CRT
build.bat --release --no-crt

# Debug with CRT
build.bat --debug --crt

# Custom build directory
build.bat --build-dir my_build

# Clean build
build.bat --clean
```

## ❓ Troubleshooting

**Build fails with "CMake not found"**
- Install CMake from https://cmake.org/download/
- Or install via Visual Studio Installer → Individual Components → CMake

**Build fails with "Cannot open include file"**
- Install Windows SDK via Visual Studio Installer
- Ensure C++ tools are installed

**DLL fails to load**
- Check architecture (x86 vs x64) matches your target
- Verify all dependencies are available
- Check Windows Event Viewer for details

## 📚 Documentation

- Full documentation: `README.md`
- Code examples: `examples/usage_example.cpp`
- Build system: `CMakeLists.txt`

## 🎓 Supported Injection Methods

✅ LoadLibrary / LoadLibraryEx  
✅ Manual Mapping  
✅ Reflective DLL Injection  
✅ Thread Hijacking  
✅ Kernel APC Injection  
✅ CreateRemoteThread  
✅ QueueUserAPC  
✅ SetWindowsHookEx  
✅ LdrLoadDll / LdrpLoadDll  

**Every method uses the same DLL - no changes needed!**

## 💡 Pro Tips

1. **For production**: Use Release + CRT build  
2. **For stealth**: Use Release + No-CRT build  
3. **For debugging**: Use Debug + CRT build  
4. **Test both**: x86 and x64 architectures  

---

**Happy Coding! 🎉**

For issues or questions, check the README.md or open an issue.
