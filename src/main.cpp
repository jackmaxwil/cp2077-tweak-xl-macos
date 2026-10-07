#include "App/Application.hpp"
#include "App/Project.hpp"
#include "Core/Facades/Hook.hpp"
#include "Core/Facades/Runtime.hpp"
#include <iostream>
#include <cstdlib>

namespace
{
Core::UniquePtr<App::Application> g_app;
bool g_initialized = false;  // Guard against double initialization

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
        std::cerr << "[TweakXL] " << aMessage << std::endl;
    }
}
}

// RED4ext

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::v1::PluginHandle aHandle, RED4ext::v1::EMainReason aReason,
                                        const RED4ext::v1::Sdk* aSdk)
{
    try {
        if (IsBootTraceEnabled())
        {
            std::cerr << "[TweakXL] Main called with reason: " << static_cast<int>(aReason) << std::endl;
        }
        
        switch (aReason)
        {
        case RED4ext::v1::EMainReason::Load:
        {
            // Guard against double initialization
            if (g_initialized)
            {
                BootTrace("Already initialized, skipping duplicate Load");
                return true;
            }
            g_initialized = true;
            
            BootTrace("Creating Application...");
            g_app = Core::MakeUnique<App::Application>(aHandle, aSdk);
            BootTrace("Application created, calling Bootstrap...");
            g_app->Bootstrap();
            BootTrace("Bootstrap complete");
            break;
        }
        case RED4ext::v1::EMainReason::Unload:
        {
            // Guard against double shutdown
            if (!g_initialized || !g_app)
            {
                BootTrace("Not initialized or already shut down, skipping");
                return true;
            }
            g_initialized = false;
            
            BootTrace("Shutting down...");
            g_app->Shutdown();
            g_app = nullptr;
            BootTrace("Shutdown complete");
            break;
        }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[TweakXL] ERROR: Exception in Main: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "[TweakXL] ERROR: Unknown exception in Main" << std::endl;
        return false;
    }
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::v1::PluginInfo* aInfo)
{
    aInfo->name = App::Project::NameW;
    aInfo->author = App::Project::AuthorW;
    aInfo->version = RED4EXT_V1_SEMVER(App::Project::Version.major,
                                       App::Project::Version.minor,
                                       App::Project::Version.patch);

    aInfo->runtime = RED4EXT_V1_RUNTIME_VERSION_INDEPENDENT;
    aInfo->sdk = RED4EXT_V1_SDK_VERSION_1_0_0_COMPAT_0_5_0;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_1_COMPAT_0;
}

#if defined(_WIN32) || defined(_WIN64)
// ASI (Windows only)

BOOL APIENTRY DllMain(HMODULE aHandle, DWORD aReason, LPVOID)
{
    using GameMain = Core::RawFunc<Red::AddressLib::Main, int32_t (*)(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                                                                      PWSTR pCmdLine, int nCmdShow)>;

    static const bool s_isGame = Core::Runtime::IsEXE(L"Cyberpunk2077.exe");
    static const bool s_isASI = Core::Runtime::IsASI(aHandle);

    switch (aReason) // NOLINT(hicpp-multiway-paths-covered)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(aHandle);

        if (s_isGame && s_isASI)
        {
            g_app = Core::MakeUnique<App::Application>(aHandle);

            Core::Hook::Before<GameMain>(+[]() {
                g_app->Bootstrap();
            });
        }
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        if (s_isGame && s_isASI)
        {
            g_app->Shutdown();
            g_app = nullptr;
        }
        break;
    }
    }

    return TRUE;
}
#else
// macOS: No ASI loader support - RED4ext handles plugin lifecycle
#endif
