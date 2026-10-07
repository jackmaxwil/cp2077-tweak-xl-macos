# TweakXL macOS Port: Status

Target: Cyberpunk 2077 macOS 2.3.1 (arm64).

**Not loadable yet.** TweakXL builds, but some of the game addresses it needs are still unverified in the SDK's canonical address DB. RED4ext refuses to load a plugin unless every address hash compiled into it is verified, so TweakXL is not loaded in game.

- Authoritative progress: §0 of `~/Development/cyberpunk/RESUME_PLAN.md` (a workspace file outside this repo).
- Address evidence: `RED4ext.SDK/docs/ADDRESS_AUDIT.md`.
- Remaining work: `python3 RED4ext.SDK/scripts/plugin_requirements.py TweakXL.dylib` lists every required hash and whether it is verified.

## Key files

- `lib/Support/macOS/TweakXLAddressResolver.cpp`: forwards every hash to the SDK resolver (canonical DB, verified entries only).
- `src/Red/Addresses/Library.hpp`: TweakXL's hash constants.
- `src/main.cpp` and the services under `src/App/`: hook wiring.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j "$(sysctl -n hw.ncpu)"
```
