# TweakXL macOS Port - Change Summary

## Overview

This document summarizes all changes made to port TweakXL to macOS Apple Silicon.

## Build System

### Replaced: xmake.lua → CMakeLists.txt

- New CMake build system supporting macOS
- Platform-conditional compilation
- Uses system spdlog and yaml-cpp from Homebrew
- Header-only RED4ext.SDK integration

### Dependencies

| Dependency | Windows | macOS |
|------------|---------|-------|
| RED4ext.SDK | WopsS/RED4ext.SDK | memaxo/RED4ext.SDK-macos |
| Hooking | MinHook | RED4ext native hook engine |
| WIL | vendor/wil | Removed |
| TiltedCore | Package | Replaced with std |
| hopscotch | Package | std::unordered_* |

## Platform Abstraction

### New Files

- `lib/Core/Platform.hpp` - Platform switch header
- `lib/Core/macOS.hpp` - macOS implementations

### Modified Files

#### Core Infrastructure
- `lib/Core/Win.hpp` - Added platform guards
- `lib/Core/Stl.hpp` - Replaced TiltedCore with std on macOS
- `lib/Core/Raw.hpp` - Platform-specific GetImageBase()

#### Runtime
- `lib/Core/Runtime/HostImage.cpp` - macOS module/path handling
- `lib/Core/Runtime/HostImage.hpp` - Platform-specific signature
- `lib/Core/Runtime/ModuleImage.cpp` - dladdr-based path resolution
- `lib/Core/Runtime/ModuleImage.hpp` - void* handle type
- `lib/Core/Runtime/OwnerMutex.cpp` - File-based locking on macOS
- `lib/Core/Runtime/OwnerMutex.hpp` - Platform-specific members

#### Facades
- `lib/Core/Facades/Runtime.cpp` - Platform-specific IsEXE/IsASI
- `lib/Core/Facades/Runtime.hpp` - Platform-specific signatures

#### Foundation
- `lib/Core/Foundation/RuntimeProvider.cpp` - void* handle
- `lib/Core/Foundation/RuntimeProvider.hpp` - Platform-specific types

### Providers

#### MinHook (Disabled on macOS)
- `lib/Support/MinHook/MinHookProvider.cpp` - #ifdef _WIN32
- `lib/Support/MinHook/MinHookProvider.hpp` - #ifdef _WIN32

#### RED4extProvider (Unchanged)
- Uses RED4ext SDK hooking API
- Uses RED4ext's native hook engine on macOS

### Application

- `src/main.cpp` - DllMain ifdef'd out on macOS
- `src/App/Application.cpp` - MinHook/RedLib conditionally included
- `src/App/Application.hpp` - void* handle signature
- `src/App/Environment.hpp` - Narrow strings on macOS

## Code Fixes

### Iterator Compatibility
- Replaced `.value()` with `->second` for std::unordered_map iterators
- Files: YamlReader.Template.cpp, Reflection.cpp, Manager.cpp

### Exception Handling
- Changed `std::exception(msg)` to `std::runtime_error(msg)`
- File: Parser.cpp

### Type Compatibility
- Fixed `auto` type deduction in Resolving.hpp
- Added `->second` access for non-hopscotch maps

## SDK Changes

### ScriptGameInstance-inl.hpp
- Added platform guards for MessageBox
- Uses std::cerr on macOS

## Removed Dependencies

- **MinHook** - Windows-only, replaced by RED4ext's hooking API
- **WIL** - Windows Implementation Library, not needed
- **TiltedCore** - Windows allocator, replaced with std::allocator
- **hopscotch-map** - Replaced with std::unordered_map on macOS

## Testing Checklist

- [x] CMake configuration
- [x] Compilation succeeds
- [x] dylib created
- [ ] Loads via RED4ext
- [ ] TweakDB hooks fire
- [ ] YAML tweaks parse
- [ ] Game modifications apply

## Known Limitations

1. **No ASI loader support** - macOS doesn't use ASI files
2. **No RedLib integration** - Disabled on macOS build
3. **Hardcoded game version** - No GetFileVersionInfo equivalent
