#include <cstdint>
#include <CMath.hh>
#include <rage.hh>

struct cutfObject;
struct cutfNamedObject;
struct cutfModelObject;

enum class eCutfObjectType : uint32_t
    {
        ASSETMGR = 0,
        OVERLAY,
        FADE,
        SUBTITLE,
        MODEL,
        HIDDEN_MODEL,
        FIXUP_MODEL,
        PARTICLE_FX,
        BLOCKING_BOUNDS,
        LIGHT,
        CAMERA,
        AUDIO,
        EVENT,
        ANIMMGR,
        ANIMATED_PARTICLEFX,
        REMOVAL_BOUNDS,
        RAY_FIRE,
        DECAL,
        ANIMATED_LIGHT
    };

#pragma pack(push, 1)
struct cutfObject__vftable
{
    void *destructor;
    eCutfObjectType (*GetType) (cutfObject*);
    bool (*IsThisType) (cutfObject*);
    char *(*GetTypeName) (cutfObject*);
    char *(*GetDisplayName) (cutfObject*);
    void *Clone;
    void *SetName;
    uint32_t (*GetAnimStreamingType) (cutfObject*);
    void *MergeFrom;
    void *operator_eq;
    void *operator_neq;
    void *parser_GetStructure;
};

struct cutfNamedObject__vftable : public cutfObject__vftable
{
    void *SetName;
};

struct cutfModelObject__vftable : public cutfNamedObject__vftable
{
    void *SetStreamingName;
    void *OverrideStreamingName;
    uint32_t (*GetModelType) (cutfModelObject*);
};

struct cutfObject
{
    cutfObject__vftable *vft;
    int32_t         iObjectId;
    uint8_t         field_0xc;
    uint8_t         field_0xd;
    uint8_t         field_0xe;
    uint8_t         field_0xf;
    struct atString m_szName;
    uint8_t         field_0x1c;
    uint8_t         field_0x1d;
    uint8_t         field_0x1e;
    uint8_t         field_0x1f;
    void *          attributeList;
    uint32_t        field_0x28;
    uint8_t         field_0x2c;
    uint8_t         field_0x2d;
    uint8_t         field_0x2e;
    uint8_t         field_0x2f;
    uint16_t        field_0x30;
    uint16_t        field_0x32;
    uint8_t         field_0x34;
    uint8_t         field_0x35;
    uint8_t         field_0x36;
    uint8_t         field_0x37;
    void *          cutfAttributes;

    inline eCutfObjectType
    GetType ()
    {
        return vft->GetType (this);
    }
};

struct cutfNamedObject : public cutfObject
{
    uint32_t cName;
    uint8_t  field_0x44;
    uint8_t  field_0x45;
    uint8_t  field_0x46;
    uint8_t  field_0x47;
};

struct cutfLightObject : public cutfNamedObject {
    uint8_t field_0x48[24];
    Vector4 vDirection;
    Vector4 vColour;
    Vector4 vPosition;
    float fIntensity;
    float fFallOff;
    float fConeAngle;
    float fVolumeIntensity;
    float fVolumeSizeScale;
    float fCoronaSize;
    float fCoronaIntensity;
    float fCoronaZBias;
    float fInnerConeAngle;
    float fExponentialFallOff;
    float fShadowBlur;
    int32_t iLightType;
    int32_t iLightProperty;
    int32_t TextureDictID;
    int32_t TextureKey;
    uint8_t field_0xcc;
    uint8_t field_0xcd;
    uint8_t field_0xce;
    uint8_t field_0xcf;
    uint32_t uLightFlags;
    uint32_t uHourFlags;
    uint8_t field_0xd8;
    uint8_t field_0xd9;
    bool bStatic;
    uint8_t field_0xdb;
    uint8_t field_0xdc;
    uint8_t field_0xdd;
    uint8_t field_0xde;
    uint8_t field_0xdf;
};

struct cutfNamedStreamedObject : public cutfNamedObject
{
    uint32_t StreamingName;
    uint8_t  field_0x4c;
    uint8_t  field_0x4d;
    uint8_t  field_0x4e;
    uint8_t  field_0x4f;
};

struct cutfNamedAnimatedStreamedObject : public cutfNamedStreamedObject
{
    uint32_t AnimStreamingBase;
    uint8_t  field_0x54;
    uint8_t  field_0x55;
    uint8_t  field_0x56;
    uint8_t  field_0x57;
};

struct cutfModelObject : public cutfNamedAnimatedStreamedObject
{
    uint32_t cAnimExportCtrlSpecFile;
    uint32_t cFaceExportCtrlSpecFile;
    uint32_t cAnimCompressionFile;
    uint8_t  field_0x64;
    uint8_t  field_0x65;
    uint8_t  field_0x66;
    uint8_t  field_0x67;
    uint8_t  field_0x68;
    uint8_t  field_0x69;
    uint8_t  field_0x6a;
    uint8_t  field_0x6b;
    uint32_t cHandle;
    uint32_t typeFile;
    uint8_t  field_0x74;
    uint8_t  field_0x75;
    uint8_t  field_0x76;
    uint8_t  field_0x77;

    uint8_t GetModelType ()
    {
        return static_cast<cutfModelObject__vftable *> (vft)->GetModelType (
            this);
    }
};

struct cutfCutsceneFile2
{
    void *                vft;
    uint8_t               field_0x9;
    char                  m_szFileName[256];
    uint8_t               field_0x109;
    uint8_t               field_0x10a;
    uint8_t               field_0x10b;
    float                 fTotalDuration;
    char                  cFaceDir[256];
    uint8_t               iCutsceneFlags;
    uint8_t               field_0x211[31];
    Vector4               vOffset;
    float                 fRotation;
    uint8_t               field_0x244[12];
    Vector4               vTriggerOffset;
    atArray<cutfObject *> pCutsceneObjects;
    uint8_t               field_0x26c;
    uint8_t               field_0x26d;
    uint8_t               field_0x26e;
    uint8_t               field_0x26f;
    atArray<>             pCutsceneLoadEventList;
    uint8_t               field_0x27c;
    uint8_t               field_0x27d;
    uint8_t               field_0x27e;
    uint8_t               field_0x27f;
    atArray<>             pCutsceneEventList;
    uint8_t               field_0x28c;
    uint8_t               field_0x28d;
    uint8_t               field_0x28e;
    uint8_t               field_0x28f;
    atArray<>             pCutsceneEventArgsList;
    uint8_t               field_0x29c;
    uint8_t               field_0x29d;
    uint8_t               field_0x29e;
    uint8_t               field_0x29f;
    void *                attributes;
    uint8_t               field_0x2a8[16];
    void *                cutfAttributes;
    int32_t               iRangeStart;
    int32_t               iRangeEnd;
    int32_t               iAltRangeEnd;
    float                 fSectionByTimeSliceDuration;
    float                 fFadeOutCutsceneDuration;
    float                 fFadeInGameDuration;
    uint32_t              fadeInColor;
    int32_t               iBlendOutCutsceneDuration;
    int32_t               iBlendOutCutsceneOffset;
    float                 fFadeOutGameDuration;
    float                 fFadeInCutsceneDuration;
    uint32_t              fadeOutColor;
    uint32_t              DayCoCHours;
    uint8_t               field_0x2f4[4];
    atArray<>             cameraCutList;
    uint8_t               field_0x304[4];
    atArray<>             sectionSplitList;
    uint8_t               field_0x314[12];
    uint8_t               concatDataList[2560 /* 0x40 * 0x28 */];
    uint8_t               field_0xd21[15];
    atArray<>             discardFrameList;
    uint8_t               field_0xd3c[4];

    static cutfCutsceneFile2 **sm_pCurrentFile;
    static void         InitialisePatterns ();

    static cutfCutsceneFile2 *&
    GetCurrentFile ()
    {
        return *sm_pCurrentFile;
    }
};

#pragma pack(pop)
