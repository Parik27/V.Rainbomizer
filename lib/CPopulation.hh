#include <rage.hh>

struct CPopulationGroup
{

    struct Model
    {
        uint32_t Name;
        uint32_t field_0x4;
        uint8_t  Variations[8];
    };

    uint32_t       Name;
    uint32_t       field_0x4;
    atArray<Model> models;
    uint8_t        field_0x14;
    uint8_t        field_0x15;
    uint8_t        field_0x16;
    uint8_t        field_0x17;
    uint8_t        flags;
    uint8_t        field_0x19;
    uint8_t        field_0x1a;
    uint8_t        field_0x1b;
    bool           field_0x1c;
    uint8_t        field_0x1d;
    uint8_t        field_0x1e;
    uint8_t        field_0x1f;
};

// ba fc 76 c4 68 e8

struct CPopGroupList
{
    atArray<CPopulationGroup> pedGroups;
    uint32_t                  field_0xc;
    atArray<CPopulationGroup> vehGroups;
    uint32_t                  field_0x1c;
    atArray<CPopulationGroup> wildlifeHabitats;
    uint32_t                  field_0x2c;
};
