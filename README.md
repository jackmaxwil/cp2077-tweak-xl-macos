# TweakXL for macOS

TweakXL lets mods change Cyberpunk 2077's TweakDB, the game database of items, stats, vehicles and other records. This is a macOS port of [psiberx/cp2077-tweak-xl](https://github.com/psiberx/cp2077-tweak-xl). It runs as a RED4ext plugin on Cyberpunk 2077 2.3.1 (Steam, Apple silicon). It loads YAML and `.tweak` files from `r6/tweaks/`, creates and overrides records and flats, adds custom stats and runs scriptable tweaks. Tweak files written for Windows work unchanged.

## Install

TweakXL is part of the RED4ext macOS release. Follow [RED4ext's install guide](https://github.com/jackmaxwil/RED4ext-macos/blob/macos-port/docs/INSTALL_MACOS.md):

1. Unzip the release over the game folder (`~/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077`).
2. Run the one-time setup, `red4ext/macos/scripts/install_macos.sh`, from the game folder.
3. Start the game with `launch_red4ext.sh` from the game folder. The Steam Play button starts the game without mods.

TweakXL ends up in `red4ext/plugins/TweakXL/`, and its hot reload key binding in `r6/input/tweakxl.xml`.

## Use it

1. Put `.yaml` files in `r6/tweaks/` in the game folder. Subfolders are fine. For example, `r6/tweaks/my_tweak.yaml`:
   ```yaml
   MyMod.MyNumber:
     $type: Int32
     $value: 2077
   MyMod.LegendaryMoney:
     $base: Items.money
     quality: Quality.Legendary
   ```
2. Start the game with `launch_red4ext.sh`.
3. To change tweaks while playing, edit the files and press `\` (backslash). The game shows "TweakXL: tweaks reloaded". Changed and added records apply at once. Objects that already copied a record, such as an equipped item or a spawned NPC, pick up the change when they are created again.
4. Check `red4ext/plugins/TweakXL/TweakXL.log` for errors. It always points to the newest log, `TweakXL-<date>-<time>.log`.

How to write tweaks: see the [upstream wiki](https://github.com/psiberx/cp2077-tweak-xl/wiki).

## Troubleshooting

- **No TweakXL log, or tweaks do nothing.** Start the game with `launch_red4ext.sh`, not with Steam. Check `red4ext/logs/red4ext-*.log`: if RED4ext refused TweakXL, it says why there.
- **`Ambiguous definition. The value type cannot be determined.`** No record has that name, so TweakXL read the entry as a new flat with no type. Fix the record name, or add `$type` if you meant a new flat.
- **`refers to a non-existent record or flat`.** A value points to a record that does not exist. Fix the name it reports.
- **`\` does nothing.** Make sure `r6/input/tweakxl.xml` exists, then restart the game with `launch_red4ext.sh`. The key works in gameplay, not in menus.
- **Game was updated.** RED4ext refuses to hook a game build it does not know, so TweakXL does not load until a new release is out.

## Build from source

Requires Xcode command line tools and Homebrew.

```bash
brew install cmake spdlog yaml-cpp
git clone --recursive -b macos-port https://github.com/jackmaxwil/cp2077-tweak-xl-macos.git
cd cp2077-tweak-xl-macos
cmake -S . -B build-dev -DCMAKE_BUILD_TYPE=Release
cmake --build build-dev -j8
```

The result is `build-dev/TweakXL.dylib`. If a checkout of [RED4ext.SDK-macos](https://github.com/jackmaxwil/RED4ext.SDK-macos) sits next to this repo (`../RED4ext.SDK`), CMake uses its headers; otherwise it uses the `vendor/RED4ext.SDK` submodule. In the workspace, RED4ext's `tools/cp-dev --plugins` builds and installs TweakXL with its `scripts/` and `data/`, and `scripts/create_release.sh` packages it.

## macOS changes

- CMake build (`CMakeLists.txt`) producing `TweakXL.dylib`, using spdlog and yaml-cpp from Homebrew. `xmake.lua` is kept for Windows.
- Hooks go through RED4ext's native arm64 hook engine (`lib/Support/macOS/MacOSHookingProvider.hpp`) instead of MinHook.
- Game addresses come from the RED4ext.SDK address database (`lib/Support/macOS/TweakXLAddressResolver.cpp`). Only entries marked verified resolve; all 54 that TweakXL uses are verified for 2.3.1.
- Standard containers and allocators replace TiltedCore and hopscotch-map; Win32 calls have POSIX equivalents (`lib/Core/macOS.hpp`, `lib/Core/Runtime/`).
- The game root is three folders above `Cyberpunk2077.app/Contents/MacOS/Cyberpunk2077`.
- Enum RTTI names are resolved the same way under clang as under MSVC (custom stats depend on this).
- New: hot reload on `\` (`scripts/HotReload.reds`, `scripts/r6/input/tweakxl.xml`).

## Credits

TweakXL is by [psiberx](https://github.com/psiberx) and contributors, MIT license (see `LICENSE` and `THIRD_PARTY_LICENSES`). The macOS port keeps the same license.
