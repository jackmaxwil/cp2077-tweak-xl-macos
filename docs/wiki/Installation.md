# Installation Guide

## Prerequisites

Before installing TweakXL, you need:

1. **macOS 14+ (Sonoma)** on Apple Silicon (M1/M2/M3/M4)
2. **Cyberpunk 2077** for macOS (Steam or GOG)
3. **RED4ext-macos** - The mod loader

### Installing RED4ext

```bash
# Clone and build RED4ext-macos
git clone --recursive https://github.com/memaxo/RED4ext-macos.git
cd RED4ext-macos
./scripts/macos_install.sh
```

See [RED4ext-macos documentation](https://github.com/memaxo/RED4ext-macos) for detailed setup.

## Installing TweakXL

### Option 1: Pre-built Release (Recommended)

1. Download the latest release from [Releases](https://github.com/memaxo/cp2077-tweak-xl-macos/releases)
2. Extract `TweakXL-x.x.x-macos-arm64.zip`
3. Copy to your game directory:

```bash
GAME_DIR="/Users/YOU/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077"

# Copy plugin
cp -r TweakXL "$GAME_DIR/red4ext/plugins/"

# Copy scripts and tweaks folder
cp -r r6/* "$GAME_DIR/r6/"
```

### Option 2: Build from Source

```bash
# Clone with submodules
git clone --recursive https://github.com/memaxo/cp2077-tweak-xl-macos.git
cd cp2077-tweak-xl-macos

# Switch to macos-port branch
git checkout macos-port

# Install dependencies
brew install spdlog yaml-cpp

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)

# Install
cp TweakXL.dylib "$GAME_DIR/red4ext/plugins/TweakXL/"
```

## Verification

After installation, your directory structure should look like:

```
Cyberpunk 2077/
├── red4ext/
│   ├── RED4ext.dylib
│   └── plugins/
│       └── TweakXL/
│           ├── TweakXL.dylib
│           ├── ExtraFlats.dat
│           └── InheritanceMap.dat
└── r6/
    ├── scripts/
    │   ├── TweakXL.reds
    │   └── ... (other .reds files)
    └── tweaks/
        └── (your YAML tweaks go here)
```

## Launching the Game

Always launch via the RED4ext launcher:

```bash
cd "$GAME_DIR"
./launch_red4ext.sh
```

Or use the provided Steam integration if set up.

## Next Steps

- Read about [YAML Tweaks](https://github.com/psiberx/cp2077-tweak-xl/wiki/YAML-Tweaks)
- Check [Examples](https://github.com/psiberx/cp2077-tweak-xl/wiki/Examples)
- Join the modding community on Discord
