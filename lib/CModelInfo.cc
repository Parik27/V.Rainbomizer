#include "CModelInfo.hh"
#include "memory/GameAddress.hh"
#include <Utils.hh>
#include <cstdint>
#include <rage.hh>
#include <cassert>

static_assert ("VEHICLE_TYPE_NONE"_joaat == 0x02f2b9bc);

/*******************************************************/
uint32_t *
InitialiseTypeTranslationMap ()
{
    const int       MAX_IDS = 32;
    static uint32_t translationMap[MAX_IDS]; // doubt they'll add more

    struct parEnumTranslationMap
    {
        uint32_t hash;
        int32_t  value;
    } *typeEnumMap
        = hook::get_pattern<parEnumTranslationMap> ("bc b9 f2 02 ff ff ff ff");

    for (int i = 0;; i++)
        {
            if (typeEnumMap[i].hash == 0)
                break;

            if (typeEnumMap[i].hash == "VEHICLE_TYPE_NONE"_joaat
                || typeEnumMap[i].value == -1)
                continue;

            if (i > MAX_IDS)
                assert (!"Type ID greater than expected");

            translationMap[typeEnumMap[i].value] = typeEnumMap[i].hash;
        }

    return translationMap;
}

/*******************************************************/
uint32_t
CVehicleModelInfo::TranslateType (int32_t type)
{
    static uint32_t *aTranslationTypeMap = InitialiseTypeTranslationMap ();
    return aTranslationTypeMap[type];
}
