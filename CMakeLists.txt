cmake_minimum_required(VERSION 3.20)
project(TweakXL VERSION 1.11.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(APPLE)
    message(STATUS "Building TweakXL for macOS")
    add_compile_definitions(RED4EXT_PLATFORM_MACOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    set(OUTPUT_NAME "TweakXL")
    
    # macOS linker flags for dylib
    # Note: -flat_namespace removed because it causes TweakXL to use RED4ext's
    # SDK symbols instead of its own modified versions
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
else()
    message(STATUS "Building TweakXL for Windows")
    add_compile_definitions(WINVER=0x0601)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(NOMINMAX)
    set(OUTPUT_NAME "TweakXL")
endif()

# Generate Project.hpp from template
set(NAME "TweakXL")
set(AUTHOR "psiberx")
set(VERSION "${PROJECT_VERSION}")
set(MODE "release")
set(DEBUG "")
configure_file(
    "${CMAKE_SOURCE_DIR}/config/Project.hpp.in"
    "${CMAKE_SOURCE_DIR}/src/App/Project.hpp"
    @ONLY
)

# Vendor dependencies (header-only)
add_library(nameof INTERFACE)
target_include_directories(nameof INTERFACE "${CMAKE_SOURCE_DIR}/vendor/nameof/include")

add_library(semver INTERFACE)
target_include_directories(semver INTERFACE "${CMAKE_SOURCE_DIR}/vendor/semver/include")

add_library(pegtl INTERFACE)
target_include_directories(pegtl INTERFACE "${CMAKE_SOURCE_DIR}/vendor/pegtl/include")

# RED4ext.SDK (header-only for macOS)
add_library(RED4ext.SDK INTERFACE)

# RED4ext.SDK headers. RED4EXT_SDK_DIR is the SDK repository root; it defaults to the pinned vendor/RED4ext.SDK
# submodule. RED4ext's tools/cp-dev and scripts/create_release.sh pass the workspace checkout explicitly.
set(RED4EXT_SDK_DIR "${CMAKE_SOURCE_DIR}/vendor/RED4ext.SDK" CACHE PATH "RED4ext.SDK repository root")
if(NOT EXISTS "${RED4EXT_SDK_DIR}/include/RED4ext/RED4ext.hpp")
    message(FATAL_ERROR "RED4ext.SDK not found at ${RED4EXT_SDK_DIR}. "
                        "Run `git submodule update --init --recursive`, or pass -DRED4EXT_SDK_DIR=<path>.")
endif()
message(STATUS "RED4ext.SDK: ${RED4EXT_SDK_DIR}")
target_include_directories(RED4ext.SDK INTERFACE "${RED4EXT_SDK_DIR}/include")

# Platform-specific dependencies
if(NOT APPLE)
    # WIL (Windows only)
    add_library(wil INTERFACE)
    target_include_directories(wil INTERFACE "${CMAKE_SOURCE_DIR}/vendor/wil/include")
endif()

# Find external packages
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

# TiltedCore and hopscotch - for macOS, we need to provide these
if(APPLE)
    # For macOS, we'll use standard allocator instead of TiltedPhoques
    # This is handled via platform abstraction in Stl.hpp
endif()

# Source files
set(TWEAKXL_SOURCES
    src/main.cpp
    src/pch.cpp
    src/rtti.cpp
    src/App/Application.cpp
    src/App/Facade.cpp
    src/App/Stats/StatService.cpp
    src/App/Tweaks/Batch/TweakChangelog.cpp
    src/App/Tweaks/Batch/TweakChangeset.cpp
    src/App/Tweaks/Declarative/TweakImporter.cpp
    src/App/Tweaks/Declarative/TweakReader.cpp
    src/App/Tweaks/Declarative/Red/RedReader.cpp
    src/App/Tweaks/Declarative/Red/RedReader.Types.cpp
    src/App/Tweaks/Declarative/Red/RedReader.Values.cpp
    src/App/Tweaks/Declarative/Yaml/YamlReader.cpp
    src/App/Tweaks/Declarative/Yaml/YamlReader.Legacy.cpp
    src/App/Tweaks/Declarative/Yaml/YamlReader.Template.cpp
    src/App/Tweaks/Declarative/Yaml/YamlReader.Values.cpp
    src/App/Tweaks/Executable/TweakExecutor.cpp
    src/App/Tweaks/Executable/Scriptable/ScriptBatch.cpp
    src/App/Tweaks/Executable/Scriptable/ScriptInterface.cpp
    src/App/Tweaks/Executable/Scriptable/ScriptManager.cpp
    src/App/Tweaks/Metadata/MetadataExporter.cpp
    src/App/Tweaks/Metadata/MetadataImporter.cpp
    src/App/Tweaks/TweakService.cpp
    src/Red/TweakDB/Buffer.cpp
    src/Red/TweakDB/Manager.cpp
    src/Red/TweakDB/Reflection.cpp
    src/Red/TweakDB/Source/Parser.cpp
    lib/Core/Facades/Runtime.cpp
    lib/Core/Foundation/Application.cpp
    lib/Core/Foundation/RuntimeProvider.cpp
    lib/Core/Hooking/HookingAgent.cpp
    lib/Core/Hooking/HookingDriver.cpp
    lib/Core/Logging/LoggingAgent.cpp
    lib/Core/Logging/LoggingDriver.cpp
    lib/Core/Memory/AddressResolver.cpp
    lib/Core/Runtime/HostImage.cpp
    lib/Core/Runtime/ModuleImage.cpp
    lib/Core/Runtime/OwnerMutex.cpp
    lib/Support/RED4ext/RED4extProvider.cpp
    lib/Support/Spdlog/SpdlogProvider.cpp
    lib/Support/RedLib/RedLibProvider.cpp
)

# Platform-specific sources
if(APPLE)
    list(APPEND TWEAKXL_SOURCES
        lib/Support/macOS/TweakXLAddressResolver.cpp
    )
else()
    list(APPEND TWEAKXL_SOURCES
        lib/Support/MinHook/MinHookProvider.cpp
    )
endif()

# Create shared library
add_library(TweakXL SHARED ${TWEAKXL_SOURCES})

# Set output name
set_target_properties(TweakXL PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

if(APPLE)
    set_target_properties(TweakXL PROPERTIES
        SUFFIX ".dylib"
        PREFIX ""
    )
else()
    set_target_properties(TweakXL PROPERTIES
        SUFFIX ".dll"
        PREFIX ""
    )
endif()

# Include directories
target_include_directories(TweakXL PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

# Link libraries
target_link_libraries(TweakXL PRIVATE
    RED4ext.SDK
    nameof
    semver
    pegtl
    spdlog::spdlog
    yaml-cpp::yaml-cpp
)

if(NOT APPLE)
    target_link_libraries(TweakXL PRIVATE
        wil
        Version
        User32
    )
endif()

# Precompiled header
target_precompile_headers(TweakXL PRIVATE src/pch.hpp)

# Compiler options
if(APPLE)
    target_compile_options(TweakXL PRIVATE
        -fno-rtti
        -fexceptions
        -Wno-deprecated-declarations
        -fvisibility=hidden
        -fvisibility-inlines-hidden
    )
else()
    target_compile_options(TweakXL PRIVATE
        /MP /GR- /EHsc
    )
endif()

# Installation
install(TARGETS TweakXL
    LIBRARY DESTINATION red4ext/plugins
    RUNTIME DESTINATION red4ext/plugins
)

message(STATUS "TweakXL ${PROJECT_VERSION} configured")
