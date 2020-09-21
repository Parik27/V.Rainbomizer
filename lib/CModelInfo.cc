#include "CModelInfo.hh"
#include <Utils.hh>
#include <rage.hh>
#include <cassert>

static_assert ("VEHICLE_TYPE_NONE"_joaat == 0x02f2b9bc);

/*******************************************************/
uint32_t *
InitialiseTypeTranslationMap ()
{
    const int MAX_IDS = 32;
    static uint32_t translationMap[MAX_IDS]; // doubt they'll add more

    struct parEnumTranslationMap {
        uint32_t hash;
        int32_t value;
    } *typeEnumMap
        = hook::get_pattern<parEnumTranslationMap> ("bc b9 f2 02 ff ff ff ff");

    for (int i = 0;; i++)
        {
            if (typeEnumMap[i].hash == 0)
                break;
            
            if (typeEnumMap[i].hash == "VEHICLE_TYPE_NONE"_joaat
                || typeEnumMap[i].value == -1)
                continue;
            
#ifndef NDEBUG
            if (i > MAX_IDS)
                assert (!"Type ID greater than expected");
#endif

            translationMap[typeEnumMap[i].value] = typeEnumMap[i].hash;
        }

    return translationMap;
}

/*******************************************************/
uint32_t
CVehicleModelInfo::GetVehicleType ()
{
    static uint32_t nTypeIndex = *hook::get_pattern<uint32_t> (
        "e8 ? ? ? ? 85 c0 74 ? 8b ? ? ? ? ? ? 8b cd", 11);

    static uint32_t *aTranslationTypeMap = InitialiseTypeTranslationMap ();

    // Should be safe since we have asserts for stuff
    int32_t type = *(injector::raw_ptr (this) + nTypeIndex).get<int32_t> ();

    if (type == -1)
        return "VEHICLE_TYPE_NONE"_joaat;

    return aTranslationTypeMap[type];
}

/*******************************************************/
CPedModelInfo::InitInfo &
CPedModelInfo::GetInitInfo ()
{
    static uint32_t nPedCapsuleNameIndex = *hook::get_pattern<uint32_t> (
        "8b ? ? ? ? ? ? 85 d2 74 ? 0f b7 05 ? ? ? ? 8b d6", 2);

    return *GetAtOffset<InitInfo> (this, nPedCapsuleNameIndex
                                             - offsetof (InitInfo,
                                                         m_nPedCapsuleName));

    static_assert (offsetof (InitInfo, m_nPedCapsuleName) == 8,
                   "PedCapsuleName at wrong offset");
}

/*******************************************************/
char*
CVehicleModelInfo::GetGameName()
{
    static uint32_t nGameNameIndex = *hook::get_pattern<uint32_t> (
        "bb 0c 00 00 00 ? 8d ? ? ? ? ? ? 8d ? d0 ? 8d 05 ? ? ? ? 8b d3 ", 8);

    return (reinterpret_cast<char*>(this) + nGameNameIndex);
}
