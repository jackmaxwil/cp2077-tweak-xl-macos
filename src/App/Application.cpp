#include "Application.hpp"
#include "App/Environment.hpp"
#include "App/Migration.hpp"
#include "App/Project.hpp"
#include "App/Stats/StatService.hpp"
#include "App/Tweaks/TweakService.hpp"
#include "Core/Foundation/RuntimeProvider.hpp"
#include "Support/RedLib/RedLibProvider.hpp"
#include "Support/RED4ext/RED4extProvider.hpp"
#include "Support/Spdlog/SpdlogProvider.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include "Support/MinHook/MinHookProvider.hpp"
#else
// macOS: Use custom address resolver and hooking provider
#include "Support/macOS/TweakXLAddressResolver.hpp"
#include "Support/macOS/MacOSHookingProvider.hpp"
#endif

#include <iostream>
#include <cstdlib>

namespace
{
bool IsBootTraceEnabled()
{
    static const bool enabled = []() {
        const char* value = std::getenv("TWEAKXL_BOOT_TRACE");
        return value && value[0] != '\0' && value[0] != '0';
    }();
    return enabled;
}

void BootTrace(const char* aMessage)
{
    if (IsBootTraceEnabled())
    {
        std::cerr << "[TweakXL::App] " << aMessage << std::endl;
    }
}
}

#if defined(_WIN32) || defined(_WIN64)
App::Application::Application(HMODULE aHandle, const RED4ext::Sdk* aSdk)
#else
App::Application::Application(void* aHandle, const RED4ext::Sdk* aSdk)
#endif
{
    BootTrace("Step 1: RuntimeProvider...");
    // Game root from the executable: bin/x64/Cyberpunk2077.exe on Windows,
    // Cyberpunk2077.app/Contents/MacOS/Cyberpunk2077 on macOS.
    Register<Core::RuntimeProvider>(aHandle)
#if defined(_WIN32) || defined(_WIN64)
        ->SetBaseImagePathDepth(2);
#else
        ->SetBaseImagePathDepth(3);
#endif

#if defined(_WIN32) || defined(_WIN64)
    Register<Support::MinHookProvider>();
#endif

    BootTrace("Step 2: SpdlogProvider...");
    Register<Support::SpdlogProvider>()
        ->AppendTimestampToLogName()
        ->CreateRecentLogSymlink();
    
#if defined(_WIN32) || defined(_WIN64)
    // Windows: Use RED4ext's address library
    Register<Support::RED4extProvider>(aHandle, aSdk)
        ->EnableAddressLibrary()
        ->RegisterScripts(Env::PluginScriptsDir());
#else
    BootTrace("Step 3: TweakXLAddressResolver...");
    // macOS: Use custom address resolver (SDK's resolver requires 126+ addresses we don't have)
    Register<Support::TweakXLAddressResolver>();
    
    BootTrace("Step 3b: MacOSHookingProvider...");
    // macOS: Use hooking provider that forwards to RED4ext's SDK
    Register<Support::MacOSHookingProvider>(aHandle, aSdk);
    
    BootTrace("Step 4: RED4extProvider...");
    Register<Support::RED4extProvider>(aHandle, aSdk)
        ->RegisterScripts(Env::PluginScriptsDir());
#endif

    // Registers TweakXL's native script classes (TweakDBManager, TweakDBBatch, ...). Without them the game's script
    // loader cannot bind the declarations in TweakXL's scripts and stops with "Failed to initialize scripts data!".
    Register<Support::RedLibProvider>();

    BootTrace("Step 5: TweakService...");
    Register<App::TweakService>(Env::GameVer(), Env::GameDir(), Env::TweaksDir(),
                                Env::InheritanceMapPath(), Env::ExtraFlatsPath(),
                                Env::RedModSourcesDir());
    
    BootTrace("Step 6: StatService...");
    Register<App::StatService>();
    
    BootTrace("Construction complete");
}

void App::Application::OnStarting()
{
    try {
        LogInfo("TweakXL is starting...");
        BootTrace("OnStarting: Migration::CleanUp");
        Migration::CleanUp(Env::LegacyScriptsDir());
    }
    catch (const std::exception& e) {
        std::cerr << "[TweakXL::App::OnStarting] Exception: " << e.what() << std::endl;
        throw;
    }
}
