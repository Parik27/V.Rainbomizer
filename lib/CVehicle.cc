#include <CVehicle.hh>
#include <Utils.hh>

CARGB *
CCustomShaderEffectVehicle::GetColours ()
{
    static uint32_t offset = *hook::get_pattern<uint32_t> (
        "? 89 ? 65 ? 8b 04 ? 58 00 00 00 ? 89 ? ? ? ? ? ? 89", 15);

    return GetAtOffset<CARGB> (this, offset);
}
