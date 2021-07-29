#pragma once

#include <cstdint>
#include <rage.hh>

enum AmbientModelSetType
{
    AMBIENT_PED_MODEL_SET = 0,
    AMBIENT_PROP_MODEL_SET,
    AMBIENT_VEHICLE_MODEL_SET
};

struct CAmbientModel
{
    uint32_t Name;
    uint32_t field_0x4;
    void *   Variations;
    float    Probability;
    uint32_t field_0x14;
};

struct CAmbientModelSet
{
    void *                        vft;
    uint32_t                      Name;
    uint32_t                      field_0xc;
    struct atArray<CAmbientModel> Models;
    uint32_t                      field_0x1c;
};

struct CAmbientModelSets
{
    atArray<CAmbientModelSet *> ModelSets;
    uint8_t                     field_0xc[28];
};

struct CAmbientModelSetsManager
{
    uint8_t                  field_0x0[16];
    struct CAmbientModelSets aSets[3];

    inline static CAmbientModelSetsManager **sm_Instance;

    static CAmbientModelSetsManager *
    Get ()
    {
        return *sm_Instance;
    }

    static void InitialisePatterns ();
};
