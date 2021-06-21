#pragma once

#include "rage.hh"
#include <cstdint>
#include <unordered_map>

/*******************************************************/
class parMemberType
{
public:
    enum Enum : uint8_t
    {
        BOOL      = 0,
        CHAR      = 1,
        UCHAR     = 2,
        SHORT     = 3,
        USHORT    = 4,
        INT       = 5,
        UINT      = 6,
        FLOAT     = 7,
        VECTOR2   = 8,
        VECTOR3   = 9,
        VECTOR4   = 10,
        STRING    = 11,
        STRUCT    = 12,
        ARRAY     = 13,
        ENUM      = 14,
        BITSET    = 15,
        MAP       = 16,
        MATRIX34  = 17,
        MATRIX44  = 18,
        VEC2V     = 19,
        VEC3V     = 20,
        VEC4V     = 21,
        MAT33V    = 22,
        MAT34V    = 23,
        MAT44V    = 24,
        SCALARV   = 25,
        BOOLV     = 26,
        VECBOOLV  = 27,
        C2DC59DDh = 28,
        SIZET     = 29,
        FLOAT16   = 30,
        INT64     = 31,
        UINT64    = 32,
        DOUBLE    = 33
    };
};

/*******************************************************/
#pragma pack(push, 1)
struct parMemberCommonData
{
    uint32_t            nHash;
    uint32_t            field_0x4;
    uint64_t            nOffset;
    parMemberType::Enum eType;
};

struct parMemberEnumData : parMemberCommonData
{

    enum parMemberEnum_eTypes : uint8_t
    {
        BYTE  = 2,
        INT   = 0,
        SHORT = 1
    } eType;

    uint32_t field_0x12;
    uint16_t m_nStringNamespace;
    uint8_t  field_0x18[16];
    void **  m_pTranslationTable;
};

struct parMemberBitsetData : parMemberCommonData
{

    uint8_t field_0x18[23];
    void ** m_pTranslationTable;
};

#pragma pack(pop)

/*******************************************************/
struct parStructure
{
    void *          vft;
    uint32_t        nName;
    uint32_t        field_0xc;
    parStructure *  pBaseClass;
    uint64_t        nBaseOffset;
    uint64_t        nSize;
    uint8_t         field_0x28;
    uint8_t         field_0x29;
    uint16_t        field_0x2a;
    uint16_t        nMajorVersion;
    uint16_t        field_0x2e;
    atArray<void *> aMembers;
    uint32_t        field_0x3c;
    void *          field_0x40;
    uint32_t        field_0x48;
    uint32_t        field_0x4c;
    void *          pFactories[3];
    void *          field_0x68;
    void *          pThisPtr;
    uint32_t        field_0x78;
    uint32_t        field_0x7c;
    void *          pDestructor;
    bool            bSorted;
    uint8_t         field_0x89[7];
    atArray<void *> pFunctions;
    uint8_t         field_0x9c[4];
};

/*******************************************************/
struct parStructureStaticData
{
    uint32_t              Name;
    uint8_t               field_0x4[12];
    parStructure *        Structure;
    parMemberCommonData **Params;
    uint32_t *            Offsets;
    char **               FieldNames;
    uint8_t               field_0x30[8];
};
