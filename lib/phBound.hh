#pragma once

#include <cstdint>
#include <rage.hh>

enum eBoundType : uint8_t
{
    SPHERE,
    CAPSULE,
    TYPE_2,
    BOX,
    GEOMETRY,
    TYPE_5,
    TYPE_6,
    TYPE_7,
    BVH,
    TYPE_9,
    COMPOSITE,
    TYPE_11,
    DISC,
    CYLINDER,
    TYPE_14,
    PLANE
};

class phBound
{
public:
    void *      vft;
    uint64_t    field_0x8;
    eBoundType  Type;
    uint8_t     Flags;
    uint16_t    PartIndex;
    float       RadiusAroundCentroid;
    uint64_t    field_0x18;
    rage::Vec4V BoundingBoxMaxXYZMarginW;
    rage::Vec4V BoundingBoxMinXYZRefCountW;
    rage::Vec4V CentroidOffsetXYZMaterialId0W;
    rage::Vec4V CGOffsetXYZMaterialId1W;
    rage::Vec4V VolumeDistribution;

    static void InitialisePatterns ();
};

class phBoundComposite : public phBound
{
public:
    phBound **    Bounds;
    rage::Mat34V *CurrentMatrices;
    rage::Mat34V *LastMatrices;
    void *        _0x1ebbf47a;
    uint32_t *    TypeAndIncludeFlags;
    uint32_t *    OwnedTypeAndIncludeFlags;
    uint16_t      MaxNumBounds;
    uint16_t      NumBounds;
    uint64_t      field_0xa8;

    void AllocateTypeAndIncludeFlags ();
};

static_assert (sizeof (phBoundComposite) == 176, "phBoundComposite wrong size");
