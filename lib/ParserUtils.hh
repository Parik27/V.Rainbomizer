#pragma once

#include "Parser.hh"

/*******************************************************/
class ParserUtils
{
    inline static std::unordered_map<uint32_t, parStructureStaticData *>
        m_pStaticDatas;

    template <auto &O>
    static void BuildStructureHook (parStructure *, parStructureStaticData *);

    static parMemberCommonData *FindFieldData (parMemberCommonData **datas,
                                               uint32_t              hash);

public:
    static void Register ();

    inline static parStructureStaticData *
    FindStaticData (uint32_t hash)
    {
        return m_pStaticDatas.at (hash);
    }

    inline static parStructureStaticData *
    FindStaticData (parStructure *struc)
    {
        if (!struc)
            return nullptr;

        return FindStaticData (struc->nName);
    }

    static void *FindFieldPtr (parStructureStaticData *data, void *ptr,
                               uint32_t hash);

    template <typename T, typename P>
    static T &
    FindFieldPtr (parStructureStaticData *data, P *ptr, uint32_t hash)
    {
        return *reinterpret_cast<T *> (FindFieldPtr (data, (void*) ptr, hash));
    }
};

/*******************************************************/
template <uint32_t Hash> class ParserWrapper
{
protected:
    inline static parStructureStaticData *pStaticData = nullptr;

public:
    static uint32_t
    GetHash ()
    {
        return Hash;
    }

    template <typename T>
    T &
    Get (uint32_t fieldHash)
    {
        return const_cast<T &> (
            std::as_const (*this).template Get<T> (fieldHash));
    }

    template <typename T>
    const T &
    Get (uint32_t fieldHash) const
    {
        if (!pStaticData)
            pStaticData = ParserUtils::FindStaticData (Hash);

        return ParserUtils::FindFieldPtr<T> (pStaticData, this, fieldHash);
    }

    template <typename T>
    void
    Set (uint32_t fieldHash, T value)
    {
        Get<T> (fieldHash) = value;
    }
};
