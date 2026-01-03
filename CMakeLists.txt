cmake_minimum_required(VERSION 3.15)
project(UniversalDLL VERSION 1.0.0 LANGUAGES C CXX)

# Options
option(USE_CRT "Use C Runtime Library" ON)
option(BUILD_SHARED "Build as shared library (DLL)" ON)
option(ENABLE_EXCEPTIONS "Enable C++ exceptions" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform checks
if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only")
endif()

# Source files
set(SOURCES
    src/dllmain.cpp
)

set(HEADERS
    include/universal_dll.h
)

# Create the library
if(BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
endif()

# Include directories
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler-specific settings
if(MSVC)
    # Common MSVC flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4                 # Warning level 4
        /WX-                # Warnings not as errors (adjust as needed)
        /MP                 # Multi-processor compilation
        /permissive-        # Standards conformance
    )

    # Exception handling
    if(ENABLE_EXCEPTIONS)
        target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /EHs-c-)
        target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_EXCEPTIONS=0)
    endif()

    # CRT configuration
    if(USE_CRT)
        # Use dynamic CRT for Release, debug CRT for Debug
        set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CRT)
    else()
        # No CRT - static linking or no CRT at all
        target_compile_options(${PROJECT_NAME} PRIVATE
            /Zl                 # Omit default library name
            /GS-                # Disable security checks
            /Oi                 # Enable intrinsic functions
        )
        
        target_compile_definitions(${PROJECT_NAME} PRIVATE 
            NO_CRT
            _NO_CRT_STDIO_INLINE
        )
        
        # Remove default libraries
        if(BUILD_SHARED)
            target_link_options(${PROJECT_NAME} PRIVATE
                /NODEFAULTLIB
                /ENTRY:DllMain
            )
        endif()
        
        # Add only essential libraries
        target_link_libraries(${PROJECT_NAME} PRIVATE
            ntdll
            kernel32
        )
    endif()

    # Additional linker flags for DLL
    if(BUILD_SHARED)
        target_link_options(${PROJECT_NAME} PRIVATE
            /DLL
            /OPT:REF            # Eliminate unused functions
            /OPT:ICF            # Enable COMDAT folding
        )
    endif()

    # Architecture-specific settings
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN64 _AMD64_)
    else()
        target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32 _X86_)
    endif()

    # Debug vs Release
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>
        $<$<CONFIG:Release>:/O2 /Ob2 /GL>
    )
    
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
    )

else()
    message(WARNING "Non-MSVC compiler detected. Some features may not work correctly.")
endif()

# Export symbols
if(BUILD_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNIVERSAL_DLL_EXPORTS)
endif()

# Installation rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS} DESTINATION include)

# Generate export header
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
    BASE_NAME UNIVERSAL_DLL
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/universal_dll_export.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

# Print configuration summary
message(STATUS "=== Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Use CRT: ${USE_CRT}")
message(STATUS "Build shared: ${BUILD_SHARED}")
message(STATUS "Enable exceptions: ${ENABLE_EXCEPTIONS}")
message(STATUS "Architecture: ${CMAKE_SIZEOF_VOID_P}0-bit")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
