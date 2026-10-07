#include "StatService.hpp"
#include "App/Tweaks/TweakService.hpp"
#include "Core/Facades/Container.hpp"

namespace
{
constexpr auto BaseStatPrefix = "BaseStats.";
constexpr auto BaseStatPrefixLength = std::char_traits<char>::length(BaseStatPrefix);
constexpr auto BaseStatCount = static_cast<uint32_t>(Red::game::data::StatType::Count);
constexpr auto InvalidStat = static_cast<uint32_t>(Red::game::data::StatType::Invalid);
// RedLib derives enum RTTI names from the C++ type name, whose spelling differs between compilers.
static_assert(std::string_view(Red::GetTypeNameStr<Red::game::data::StatType>().data()) == "gamedataStatType");

bool s_statTypesModified = false;
bool s_statsHookInstalled = false;
}

void App::StatService::OnBootstrap()
{
    // Prevent double-initialization (game calls Main twice)
    if (s_statsHookInstalled)
    {
        return;
    }
    
    // Stats hooks are optional on macOS - addresses may not be available
    // TweakXL core functionality works without stats modification support
    auto result = HookAfter<Raw::StatsDataSystem::InitializeRecords>(&OnInitializeStats);
    if (!result)
    {
        LogWarning("StatService: Stats hooks unavailable (address not found). "
                   "Custom stat types will not be supported.");
    }
    else
    {
        s_statsHookInstalled = true;
        LogInfo("StatService: Stats hooks installed successfully. Custom stat types enabled.");
    }
}

void App::StatService::OnShutdown()
{
    // Detach all stats hooks (best-effort).
    Unhook<Raw::StatsDataSystem::InitializeRecords>();

    if (s_statTypesModified)
    {
        Unhook<Raw::StatsDataSystem::GetStatRange>();
        Unhook<Raw::StatsDataSystem::GetStatFlags>();
        Unhook<Raw::StatsDataSystem::CheckStatFlag>();
    }

    s_statTypesModified = false;
    s_statsHookInstalled = false;
}

void App::StatService::OnInitializeStats(void* aSystem)
{
    RegisterStats(aSystem, Core::Resolve<TweakService>()->GetChangelog().GetAffectedRecords());
    RegisterStats(aSystem, Core::Resolve<TweakService>()->GetManager().GetEnums());
}

void App::StatService::RegisterStats(void* aStatSystem, const Core::Set<Red::TweakDBID>& aRecordIDs)
{
    auto statRecords = Raw::StatsDataSystem::StatRecords::Ptr(aStatSystem);
    auto statTypeEnum = Red::GetDescriptor<Red::game::data::StatType>();
    if (!statTypeEnum || !statRecords)
    {
        LogError("StatService: gamedataStatType is not available, custom stat types are disabled.");
        return;
    }

    auto& tweakManager = Core::Resolve<TweakService>()->GetManager();

    for (const auto& recordID : aRecordIDs)
    {
        const auto& recordName = tweakManager.GetName(recordID);

        if (!recordName.starts_with(BaseStatPrefix))
            continue;

        auto enumNameProp = tweakManager.GetFlat({recordID, ".enumName"});

        if (!enumNameProp)
            continue;

        auto enumName = enumNameProp.As<Red::CString>().c_str();

        if (statTypeEnum->HasOption(enumName))
            continue;

        if (recordName.substr(BaseStatPrefixLength) != enumName)
        {
            LogError("{}: Enum name must match the record name.", recordName);
            continue;
        }

        if (statRecords->Size() == BaseStatCount)
        {
            // Add dummy entries for "Count" and "Invalid"
            statRecords->EmplaceBack();
            statRecords->EmplaceBack();
        }

        const auto enumValue = statRecords->Size();

        statTypeEnum->AddOption(enumValue, enumName);
        statRecords->PushBack(recordID);

        if (!s_statTypesModified)
        {
            s_statTypesModified = true;
            Hook<Raw::StatsDataSystem::GetStatRange>(&OnGetStatRange).OrThrow();
            Hook<Raw::StatsDataSystem::GetStatFlags>(&OnGetStatFlags).OrThrow();
            Hook<Raw::StatsDataSystem::CheckStatFlag>(&OnCheckStatFlag).OrThrow();
        }

        {
            const auto record = tweakManager.GetRecord(recordID);
            Raw::StatRecord::EnumValue::Ref(record) = enumValue;
        }
    }
}

#ifdef __APPLE__
Raw::StatsDataSystem::StatRange App::StatService::OnGetStatRange(void* aSystem, uint32_t aStat)
{
    if (aStat != InvalidStat)
    {
        auto& statParams = Raw::StatsDataSystem::StatParams::Ref(aSystem);
        auto& statLock = Raw::StatsDataSystem::StatLock::Ref(aSystem);

        std::shared_lock _(statLock);
        if (aStat < statParams.Size())
        {
            return {statParams[aStat].min, statParams[aStat].max};
        }
    }

    return {};
}
#else
uint64_t* App::StatService::OnGetStatRange(void* aSystem, uint64_t* aRange, uint32_t aStat)
{
    if (aStat != InvalidStat)
    {
        auto& statParams = Raw::StatsDataSystem::StatParams::Ref(aSystem);
        auto& statLock = Raw::StatsDataSystem::StatLock::Ref(aSystem);

        std::shared_lock _(statLock);
        if (aStat < statParams.Size())
        {
            *aRange = statParams[aStat].range;
            return aRange;
        }
    }

    *aRange = 0;
    return aRange;
}
#endif

uint32_t App::StatService::OnGetStatFlags(void* aSystem, uint32_t aStat)
{
    if (aStat != InvalidStat)
    {
        auto& statParams = Raw::StatsDataSystem::StatParams::Ref(aSystem);
        auto& statLock = Raw::StatsDataSystem::StatLock::Ref(aSystem);

        std::shared_lock _(statLock);
        if (aStat < statParams.Size())
        {
            return statParams[aStat].flags;
        }
    }

    return 0;
}

bool App::StatService::OnCheckStatFlag(void* aSystem, uint32_t aStat, uint32_t aFlag)
{
    if (aStat != InvalidStat)
    {
        auto& statParams = Raw::StatsDataSystem::StatParams::Ref(aSystem);
        auto& statLock = Raw::StatsDataSystem::StatLock::Ref(aSystem);

        std::shared_lock _(statLock);
        if (aStat < statParams.Size())
        {
            return statParams[aStat].flags & aFlag;
        }
    }

    return false;
}
