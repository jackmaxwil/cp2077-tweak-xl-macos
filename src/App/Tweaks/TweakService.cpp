#include "TweakService.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "App/Tweaks/Metadata/MetadataExporter.hpp"
#include "App/Tweaks/Metadata/MetadataImporter.hpp"
#include "Red/TweakDB/Raws.hpp"

App::TweakService::TweakService(const Core::SemvVer& aProductVer, std::filesystem::path aGameDir,
                                std::filesystem::path aTweaksDir, std::filesystem::path aInheritanceMapPath,
                                std::filesystem::path aExtraFlatsPath, std::filesystem::path aSourcesDir)
    : m_gameDir(std::move(aGameDir))
    , m_tweaksDir(std::move(aTweaksDir))
    , m_sourcesDir(std::move(aSourcesDir))
    , m_inheritanceMapPath(std::move(aInheritanceMapPath))
    , m_extraFlatsPath(std::move(aExtraFlatsPath))
    , m_productVer(aProductVer)
{
    m_importPaths.push_back(m_tweaksDir);
}

void App::TweakService::OnBootstrap()
{
    CreateTweaksDir();

    auto hookTryLoad = HookAfter<Raw::TryLoadTweakDB>([&](bool& aSuccess) {
        if (aSuccess)
        {
            m_reflection = Core::MakeShared<Red::TweakDBReflection>();
            m_manager = Core::MakeShared<Red::TweakDBManager>(m_reflection);
            m_context = Core::MakeShared<App::TweakContext>(m_productVer);
            m_importer = Core::MakeShared<App::TweakImporter>(m_manager, m_context);
            m_executor = Core::MakeShared<App::TweakExecutor>(m_manager);
            m_changelog = Core::MakeShared<App::TweakChangelog>();

            ReportUsage();

            if (ImportMetadata())
            {
                EnsureRuntimeAccess();
                ApplyPatches();
                LoadTweaks();
                ReportUsage();
            }
        }
    });

    hookTryLoad.OrThrow("Failed to hook TryLoadTweakDB.");

    auto hookInit = HookAfter<Raw::InitTweakDB>([&]() {
        EnsureRuntimeAccess();
        CheckForIssues();
    });
    hookInit.OrThrow("Failed to hook InitTweakDB.");
}

void App::TweakService::OnShutdown()
{
    // Detach hooks first to avoid callbacks into partially torn-down state.
    Unhook<Raw::TryLoadTweakDB>();
    Unhook<Raw::InitTweakDB>();

    // Release runtime state.
    m_executor.reset();
    m_importer.reset();
    m_changelog.reset();
    m_context.reset();
    m_manager.reset();
    m_reflection.reset();
}

void App::TweakService::LoadTweaks()
{
    if (m_manager)
    {
        m_importer->ImportTweaks(m_importPaths, m_changelog);
        m_executor->ExecuteTweaks();
    }
}

void App::TweakService::ImportTweaks()
{
    if (m_manager)
    {
        m_importer->ImportTweaks(m_importPaths, m_changelog);
    }
}

void App::TweakService::ExecuteTweaks()
{
    if (m_manager)
    {
        m_executor->ExecuteTweaks();
    }
}

void App::TweakService::ExecuteTweak(Red::CName aName)
{
    if (m_manager)
    {
        m_executor->ExecuteTweak(aName);
    }
}

void App::TweakService::EnsureRuntimeAccess()
{
    if (m_manager)
    {
        m_manager->GetTweakDB()->unk160 = 0;
    }
}

void App::TweakService::ApplyPatches()
{
    if (m_manager)
    {
        m_manager->CloneRecord("Vendors.IsPresent", "Vendors.Always_Present");
        m_manager->RegisterName("Vendors.IsPresent");
    }
}

void App::TweakService::CheckForIssues()
{
    if (m_manager && m_changelog)
    {
        m_changelog->CheckForIssues(m_manager);
    }
}

void App::TweakService::ReportUsage()
{
    if (m_manager)
    {
        auto stats = m_manager->GetBuffer()->GetStats();

        LogDebug("TweakDB contains {} records and {} flats.", stats.recordEntries, stats.flatEntries);
        LogDebug("TweakDB buffer is used at {:.3f} MiB / {:.0f} MiB ({:.1f}%) with {} pooled values.",
                 static_cast<float>(stats.bufferSize) / (1024 * 1024),
                 static_cast<float>(stats.bufferMaxSize) / (1024 * 1024),
                 static_cast<float>(stats.bufferSize) / static_cast<float>(stats.bufferMaxSize) * 100.0f,
                 stats.poolValues);
    }
}

void App::TweakService::CreateTweaksDir()
{
    std::error_code error;

    if (!std::filesystem::exists(m_tweaksDir, error))
    {
        if (!std::filesystem::create_directories(m_tweaksDir, error))
        {
            LogWarning("Cannot create tweaks directory \"{}\": {}.",
                       std::filesystem::relative(m_tweaksDir, m_gameDir).string(), error.message());
        }
    }
}

bool App::TweakService::RegisterTweak(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_regular_file(aPath, error))
    {
        LogError("Can't register non-existing tweak \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_importPaths.emplace_back(std::move(aPath));
    return true;
}

bool App::TweakService::RegisterDirectory(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_directory(aPath, error))
    {
        LogError("Can't register non-existing tweak directory \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_importPaths.emplace_back(std::move(aPath));
    return true;
}

bool App::TweakService::ImportMetadata()
{
    MetadataImporter importer{m_manager};

    LogInfo("Loading inheritance metadata...");

    if (!importer.ImportInheritanceMap(m_inheritanceMapPath))
    {
        LogError("Can't load inheritance metadata from \"{}\".", m_inheritanceMapPath.string());
        return false;
    }

    LogInfo("Loading extra flats metadata...");

    if (!importer.ImportExtraFlats(m_extraFlatsPath))
    {
        LogError("Can't load extra flats metadata from \"{}\".", m_extraFlatsPath.string());
        return false;
    }

    return true;
}

void App::TweakService::ExportMetadata()
{
    MetadataExporter exporter{m_manager};
    exporter.LoadSource(m_sourcesDir);
    exporter.ExportInheritanceMap(m_inheritanceMapPath);
    exporter.ExportExtraFlats(m_extraFlatsPath);
    exporter.ExportInheritanceMap(m_inheritanceMapPath.replace_extension(".yaml"));
    exporter.ExportExtraFlats(m_extraFlatsPath.replace_extension(".yaml"));
}

Red::TweakDBManager& App::TweakService::GetManager()
{
    return *m_manager;
}

Red::TweakDBReflection& App::TweakService::GetReflection()
{
    return *m_reflection;
}

App::TweakChangelog& App::TweakService::GetChangelog()
{
    return *m_changelog;
}
