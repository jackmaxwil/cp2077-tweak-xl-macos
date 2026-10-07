#pragma once

#include "Red/TweakDB/Alias.hpp"
#include "Red/TweakDB/Buffer.hpp"
#include "Red/TweakDB/Reflection.hpp"

namespace Red
{
class TweakDBManager
{
public:
    enum class Result
    {
        OK,
        InvalidID,
        InvalidType,
        InvalidValue,
        Unallocated,
    };

    class Batch
    {
        Core::Set<Red::TweakDBID> flats;
        Core::Map<Red::TweakDBID, const Red::TweakDBRecordInfo*> records;
        Core::Map<Red::TweakDBID, const std::string> names;
        std::shared_mutex mutex;
        friend TweakDBManager;
    };

    using BatchPtr = Core::SharedPtr<Batch>;

    TweakDBManager();
    explicit TweakDBManager(Red::TweakDB* aTweakDb);
    explicit TweakDBManager(Core::SharedPtr<Red::TweakDBReflection> aReflection);

    TweakDBManager(const TweakDBManager&) = delete;
    TweakDBManager& operator=(const TweakDBManager&) = delete;

    Red::Value<> GetFlat(Red::TweakDBID aFlatId);
    Red::Value<> GetDefault(const Red::CBaseRTTIType* aType);
    Red::Handle<Red::TweakDBRecord> GetRecord(Red::TweakDBID aRecordId);
    const Red::CClass* GetRecordType(Red::TweakDBID aRecordId);
    bool IsFlatExists(Red::TweakDBID aFlatId);
    bool IsRecordExists(Red::TweakDBID aRecordId);
    Result SetFlat(Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType, Red::Instance aInstance);
    Result SetFlat(Red::TweakDBID aFlatId, const Red::Value<>& aData);
    bool CreateRecord(Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool InheritProps(Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(Red::TweakDBID aRecordId);
    void RegisterEnum(Red::TweakDBID aRecordId);
    void RegisterName(const std::string& aName, const Red::CClass* aType = nullptr);
    void RegisterName(Red::TweakDBID aId, const std::string& aName, const Red::CClass* aType = nullptr);

    BatchPtr StartBatch();
    const Core::Set<Red::TweakDBID>& GetFlats(const BatchPtr& aBatch);
    Red::Value<> GetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId);
    const Red::CClass* GetRecordType(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    bool IsFlatExists(const BatchPtr& aBatch, Red::TweakDBID aFlatId);
    bool IsRecordExists(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    Result SetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId, const Red::CBaseRTTIType* aType,
                   Red::Instance aValue);
    Result SetFlat(const BatchPtr& aBatch, Red::TweakDBID aFlatId, const Red::Value<>& aData);
    bool CreateRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId, const Red::CClass* aType);
    bool CloneRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool InheritProps(const BatchPtr& aBatch, Red::TweakDBID aRecordId, Red::TweakDBID aSourceId);
    bool UpdateRecord(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    void RegisterEnum(const BatchPtr& aBatch, Red::TweakDBID aRecordId);
    void RegisterName(const BatchPtr& aBatch, Red::TweakDBID aId, const std::string& aName);
    void CommitBatch(const BatchPtr& aBatch);

    const Core::Set<Red::TweakDBID>& GetEnums();
    const Core::Map<Red::TweakDBID, std::pair<std::string, std::string>>& GetConflicts();
    std::string_view GetName(Red::TweakDBID aId);

    Red::TweakDB* GetTweakDB();
    const Core::SharedPtr<Red::TweakDBBuffer>& GetBuffer() const;
    const Core::SharedPtr<Red::TweakDBReflection>& GetReflection() const;

private:
    template<class SharedLockable>
    inline Result AssignFlat(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aFlatId,
                             const Red::CBaseRTTIType* aType, Red::Instance aInstance, SharedLockable& aMutex);
    inline void InheritFlats(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                             const Red::TweakDBRecordInfo* aRecordInfo);
    inline void InheritFlats(Red::SortedUniqueArray<Red::TweakDBID>& aFlats, Red::TweakDBID aRecordId,
                             const Red::TweakDBRecordInfo* aRecordInfo, Red::TweakDBID aSourceId);

    inline Result AssignFlat(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aFlatId,
                             const Red::Value<>& aValue);
    inline void InheritFlats(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                             const Red::TweakDBRecordInfo* aRecordInfo);
    inline void InheritFlats(const Red::TweakDBManager::BatchPtr& aBatch, Red::TweakDBID aRecordId,
                             const Red::TweakDBRecordInfo* aRecordInfo, Red::TweakDBID aSourceId);

    void CreateBaseName(Red::TweakDBID aId, const std::string& aName);
    void CreateExtraNames(Red::TweakDBID aId, const std::string& aName, const Red::CClass* aType = nullptr);

    Red::TweakDB* m_tweakDb;
    Core::SharedPtr<Red::TweakDBBuffer> m_buffer;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;
    Core::Map<Red::TweakDBID, std::string> m_knownNames;
    Core::Map<Red::TweakDBID, std::pair<std::string, std::string>> m_conflictNames;
    Core::Set<Red::TweakDBID> m_knownEnums;
    std::shared_mutex m_mutex;
};
}
