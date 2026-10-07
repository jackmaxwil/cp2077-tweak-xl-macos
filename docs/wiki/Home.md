# TweakXL for macOS

Welcome to the TweakXL macOS port documentation!

> **Note:** This is a macOS ARM64 port. For general TweakXL documentation, see the [original wiki](https://github.com/psiberx/cp2077-tweak-xl/wiki).

## Quick Links

- [Installation Guide](Installation)
- [Building from Source](Building)
- [Troubleshooting](Troubleshooting)
- [FAQ](FAQ)

## What is TweakXL?

TweakXL is a modding tool that allows you to modify TweakDB - Cyberpunk 2077's database containing game entity definitions, stats, items, and behaviors.

### Features

- **YAML Tweaks** - Modify game data using simple YAML files
- **RED Tweaks** - Use RED format for more complex modifications  
- **Script Extensions** - Add dynamic logic with Redscript
- **Hot Reloading** - Test changes without restarting the game

## macOS-Specific Information

The macOS port uses:
- RED4ext's native arm64 hook engine (instead of Windows Detours/MinHook)
- **CMake** build system (instead of xmake)
- ARM64 native binaries for Apple Silicon

## Getting Started

1. Install [RED4ext-macos](https://github.com/memaxo/RED4ext-macos)
2. Download TweakXL from [Releases](https://github.com/memaxo/cp2077-tweak-xl-macos/releases)
3. Follow the [Installation Guide](Installation)
4. Create your first tweak!

## Example

Create `r6/tweaks/my_tweak.yaml`:

```yaml
# Double all pistol damage
BaseStats.DamagePerHit_Handgun:
  value: 100
```

Launch the game and your tweak is automatically applied!
