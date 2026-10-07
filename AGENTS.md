# TweakXL macOS Port - Agent Guidelines

## Project Context

TweakXL is a Cyberpunk 2077 mod that enables runtime modification of TweakDB values (game stats, items, vehicles). This port adapts the Windows plugin to macOS ARM64, using RED4ext for loading and RED4ext's native hook engine for hooking.

## Current Status (Canonical)

See `docs/STATUS.md`. TweakXL is not loadable yet: RED4ext refuses it until every address hash it needs is verified. The authoritative progress table is §0 of `~/Development/cyberpunk/RESUME_PLAN.md`.

## Development Practices

### macOS Porting

1. **Address resolution.** `lib/Support/macOS/TweakXLAddressResolver.cpp` forwards every hash to the SDK resolver, which reads the canonical `cyberpunk2077_addresses.json` and resolves only entries marked verified.
2. **Custom hashes.** TweakXL's hash constants live in `src/Red/Addresses/Library.hpp`; their offsets live in the SDK's canonical DB, not in this repo.
3. **No Windows dependencies.** Remove or stub all `<windows.h>`, MinHook, and Win32 API usage.
4. **dylib output.** Build target is `TweakXL.dylib`, not `.dll`.

### Hook Requirements

TweakXL hooks these game functions (offsets live in the SDK's canonical DB):

| Function | Hash | Purpose |
|----------|------|---------|
| Main | 0x0E54032B | Entry point hook |
| TweakDB_Init | 0xB6832FEA | TweakDB initialization |
| TweakDB_Load | 0xD6B1DB5A | TweakDB loading |
| TweakDB_TryLoad | 0xD16A2999 | TweakDB async load |
| TweakDB_CreateRecord | 0x31FB0F6A | Record creation |
| TweakDBID_Derive | 0x137620C0 | ID derivation |
| StatsDataSystem_* | Various | Stats modification |

### Building

1. **CMake build.** Use `cmake .. -DCMAKE_BUILD_TYPE=Release && make -j8`.
2. **Vendor SDK.** SDK is in `vendor/RED4ext.SDK/`; don't modify directly.
3. **Output location.** Built `TweakXL.dylib` goes in `build/`.
4. **Installation.** Copy to `red4ext/plugins/TweakXL/TweakXL.dylib`.

## Architecture

### Directory Structure

```
src/
├── App/
│   ├── Tweaks/        # Tweak loading and application
│   ├── Stats/         # Stats system hooks
│   └── Facade.cpp     # Public API
├── Red/
│   ├── Addresses/     # Custom address definitions
│   └── TweakDB/       # TweakDB interaction
└── main.cpp           # Plugin entry point
lib/
├── Core/              # Base infrastructure (logging, hooking)
└── Support/
    └── macOS/         # macOS-specific support
        └── TweakXLAddressResolver.cpp  # Forwards to the SDK's verified-only resolver
```

### Key Files

1. **`lib/Support/macOS/TweakXLAddressResolver.cpp`** - Forwards TweakXL hashes to the SDK resolver (canonical DB, verified entries only).
2. **`src/Red/Addresses/Library.hpp`** - Defines TweakXL's custom address hash constants.
3. **`src/Red/TweakDB/Raws.hpp`** - Raw function pointers using address resolution.
4. **`src/App/Tweaks/TweakService.cpp`** - Main hook attachments (`HookAfter<Raw::TryLoadTweakDB>`).

## Code Standards

### Naming

1. **Namespaces.** `App::` for application logic, `Red::` for game interaction, `Core::` for infrastructure.
2. **Services.** PascalCase with `Service` suffix: `TweakService`, `StatService`.
3. **Raw functions.** `Raw::FunctionName` pattern: `Raw::TryLoadTweakDB`, `Raw::CreateRecord`.

### Hooking Pattern

```cpp
// In service initialization:
HookAfter<Raw::TryLoadTweakDB>([&](bool& aSuccess) {
    // Hook implementation
}).OrThrow();  // Always use OrThrow() to surface failures
```

### Error Handling

1. **OrThrow pattern.** All hook attachments must use `.OrThrow()` to fail loudly.
2. **Log failures.** If a hook fails to attach, log the hash and expected address.
3. **Graceful degradation.** If optional hooks fail (stats), continue with core functionality.

## Address Resolution Debugging

### When TweakXL Fails to Load

1. **Check RED4ext log.** Look for "One of the required parameters for attaching hook is NULL".
2. **Identify failing hash.** The log should indicate which address resolved to 0.
3. **Fix the DB entry.** Correct the offset in `RED4ext.SDK/cyberpunk2077_addresses.json` and mark it verified only with evidence in `RED4ext.SDK/docs/ADDRESS_AUDIT.md`. `RED4ext.SDK/scripts/plugin_requirements.py TweakXL.dylib` lists every unverified hash.
4. **Rebuild and reinstall.** `make && cp build/TweakXL.dylib <game>/red4ext/plugins/TweakXL/`.

### Finding New Addresses

1. **Use analysis scripts.** Run `RED4ext/scripts/` tools against the game binary.
2. **String pattern matching.** TweakDB functions reference ".tweak" and "TweakDB" strings.
3. **Function clustering.** TweakDB methods are grouped together in memory (~0x2B7xxxx range).
4. **Verify prologues.** Valid ARM64 function starts: `STP X29, X30` or `SUB SP, SP`.

## Testing

### Build Verification

```bash
cd build && cmake .. && make -j8
# Should produce TweakXL.dylib without errors
```

### Runtime Verification

1. **Install.** Copy `TweakXL.dylib` to `red4ext/plugins/TweakXL/`.
2. **Launch.** Run game via `launch_red4ext.sh`.
3. **Check logs.** `red4ext/plugins/TweakXL/TweakXL.log` should show successful initialization.
4. **In-game test.** Verify tweak files in `r6/tweaks/` are loaded.

### Expected Log Output (Success)

```
[TweakXL] TweakXL v1.11.3 initialized
[TweakXL] Registered X tweak directories
[TweakXL] Loaded X tweak files
```

### Expected Log Output (Failure)

```
[RED4ext] One of the required parameters for attaching hook is NULL
[RED4ext] TweakXL did not initialize properly, unloading...
```

## Installation

### Files to Install

```
red4ext/plugins/TweakXL/
├── TweakXL.dylib    # The built plugin
├── Data/            # From bundle/
└── Scripts/         # From scripts/
```

### Dependencies

- RED4ext.dylib must be installed and functional
- `cyberpunk2077_addresses.json` for SDK address resolution

## Common Pitfalls

1. **Wrong hash values.** TweakXL uses different hashes than SDK for same functions—verify in `Library.hpp`.
2. **Missing StatsDataSystem.** Stats hooks are optional but will warn; core functionality works without them.
3. **Old addresses.** After game updates, all addresses may change—re-run analysis scripts.
4. **Unverified addresses.** An unverified DB entry resolves to 0, and RED4ext refuses to load the plugin.
