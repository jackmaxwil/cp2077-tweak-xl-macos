#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/StatsDataSystem.hpp"

namespace App
{
class StatService
    : public Core::Feature
    , public Core::HookingAgent
    , public Core::LoggingAgent
{
protected:
    void OnBootstrap() override;
    void OnShutdown() override;

    static void OnInitializeStats(void* aSystem);
#ifdef __APPLE__
    static Raw::StatsDataSystem::StatRange OnGetStatRange(void* aSystem, uint32_t aStat);
#else
    static uint64_t* OnGetStatRange(void* aSystem, uint64_t* aRange, uint32_t aStat);
#endif
    static uint32_t OnGetStatFlags(void* aSystem, uint32_t aStat);
    static bool OnCheckStatFlag(void* aSystem, uint32_t aStat, uint32_t aFlag);

    static void RegisterStats(void* aStatSystem, const Core::Set<Red::TweakDBID>& aRecordIDs);
};
}
