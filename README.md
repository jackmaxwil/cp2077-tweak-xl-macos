# TweakXL (macOS)

Runtime TweakDB modification for Cyberpunk 2077 on macOS ARM64.

**Status:** Not loadable yet. Some of the game addresses TweakXL needs are still unverified, so RED4ext refuses to load it. See [docs/STATUS.md](docs/STATUS.md).

## What it does

TweakXL enables runtime modification of TweakDB values (game stats, items, vehicles, economy). Loads `.yaml` and `.tweak` files from `r6/tweaks/` and applies them during game initialization. Built as a RED4ext `.dylib` plugin; hooks go through RED4ext's native hook engine.

## Prerequisites

- RED4ext installed and functional
- CMake 3.24+, Clang 15+

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## Install

```bash
cp build/TweakXL.dylib "<game>/red4ext/plugins/TweakXL/"
cp -r bundle/Data/ "<game>/red4ext/plugins/TweakXL/Data/"
cp -r bundle/Scripts/ "<game>/red4ext/plugins/TweakXL/Scripts/"
```

## Key files

| File | Purpose |
|------|---------|
| `lib/Support/macOS/TweakXLAddressResolver.cpp` | Resolves addresses through the SDK's verified-only DB |
| `src/Red/Addresses/Library.hpp` | TweakXL-specific hash constants |
| `src/App/Tweaks/TweakService.cpp` | Main hook attachments |
| `docs/STATUS.md` | Port status |

## Related projects

| Project | Description |
|---------|-------------|
| [RED4ext](../RED4ext) | Required mod loader |
| [RED4ext.SDK](../RED4ext.SDK) | SDK dependency |
| [ArchiveXL](../cp2077-archive-xl-macos) | Companion resource plugin |

## Attribution

Forked from [psiberx/cp2077-tweak-xl](https://github.com/psiberx/cp2077-tweak-xl). macOS port by memaxo.
