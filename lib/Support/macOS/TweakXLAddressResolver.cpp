#include "TweakXLAddressResolver.hpp"
#include <iostream>
#include <cstdlib>
#include <RED4ext/Relocation.hpp>

namespace Support
{

std::unordered_map<uint32_t, int> TweakXLAddressResolver::s_requestedAddresses;

namespace
{
bool IsAddressTraceEnabled()
{
    static const bool enabled = []() {
        const char* value = std::getenv("TWEAKXL_ADDR_TRACE");
        return value && value[0] != '\0' && value[0] != '0';
    }();
    return enabled;
}
}

TweakXLAddressResolver::TweakXLAddressResolver()
{
    InitializeAddressTable();
}

void TweakXLAddressResolver::OnInitialize()
{
    // Set this resolver as the default for Core::AddressResolver
    AddressResolver::SetDefault(*this);
    if (IsAddressTraceEnabled())
    {
        std::cerr << "[TweakXLAddressResolver] Registered as default address resolver" << std::endl;
    }
}

uintptr_t TweakXLAddressResolver::GetImageBase()
{
    static const uintptr_t base = RED4ext::RelocBase::GetImageBase();
    return base;
}

void TweakXLAddressResolver::InitializeAddressTable()
{
    // Former hard-coded offsets now live, unverified, in the canonical address DB.
}

uintptr_t TweakXLAddressResolver::ResolveAddress(uint32_t aAddressID)
{
    // All addresses come from the canonical DB (red4ext/bin/x64/cyberpunk2077_addresses.json) through the SDK
    // resolver, which refuses entries not marked "verified" (fail closed). An unverified hook target resolves to 0
    // and the hook is not installed.
    s_requestedAddresses[aAddressID]++;
    return RED4ext::UniversalRelocBase::Resolve(aAddressID);
}

} // namespace Support
