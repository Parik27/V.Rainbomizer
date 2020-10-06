#include "ParserUtils.hh"
#include "Utils.hh"

/*******************************************************/
template <auto &O>
void
ParserUtils::BuildStructureHook (parStructure *          structure,
                                 parStructureStaticData *data)
{
    m_pStaticDatas[data->Name] = data;
    O (structure, data);
}

/*******************************************************/
void
ParserUtils::Register ()
{
    static bool bRegistered = false;
    if (std::exchange (bRegistered, true))
        return;

    REGISTER_HOOK ("8b 4a 10 e8 ? ? ? ? ? 8b 43 10 ? 83 c4 20", 3,
                   BuildStructureHook, void, parStructure *,
                   parStructureStaticData *);
}

/*******************************************************/
parMemberCommonData *
ParserUtils::FindFieldData (parMemberCommonData **datas, uint32_t hash)
{
    while (*datas)
        {
            if ((*datas)->nHash == hash)
                return *datas;

            datas++;
        }

    return nullptr;
}

/*******************************************************/
void *
ParserUtils::FindFieldPtr (parStructureStaticData *data, void *ptr,
                           uint32_t hash)
{
    uint64_t             initialOffset = 0;
    parMemberCommonData *field         = FindFieldData (data->Params, hash);

    while (!field)
        {
            data = FindStaticData (data->Structure->pBaseClass);

            if (!data)
                return nullptr;

            initialOffset += data->Structure->nBaseOffset;
            field = FindFieldData (data->Params, hash);
        }

    return reinterpret_cast<uint8_t *> (ptr) + field->nOffset + initialOffset;
}
