# Frequently Asked Questions

## General

### Is this official?
No, this is a community port of TweakXL to macOS. The original Windows version is maintained by [psiberx](https://github.com/psiberx).

### Will my Windows tweaks work?
Yes! YAML and RED tweak files are platform-independent. Just copy them to `r6/tweaks/`.

### Do I need redscript?
Not required for basic YAML tweaks, but needed for Script Extensions.

## Compatibility

### Which game version is supported?
Cyberpunk 2077 2.3+ for macOS (Steam/GOG).

### Does this work with other mods?
Yes, TweakXL is designed for mod compatibility. However, some mods may have their own macOS port requirements.

### Can I use Vortex/MO2?
Mod managers are Windows-only. On macOS, install mods manually by copying files.

## Technical

### Why not Detours?
Detours is Windows-only. On macOS, RED4ext patches game code with its own arm64 hook engine, which needs the game re-signed with `allow-unsigned-executable-memory`.

### Is there a performance impact?
Minimal. TweakDB modifications happen at load time.

### Can I help reverse engineer addresses?
Yes! See the [contributing guide](https://github.com/memaxo/cp2077-tweak-xl-macos/blob/macos-port/CONTRIBUTING.md) for how to help identify game function addresses.
