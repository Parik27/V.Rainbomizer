#pragma once

#include "Parser.hh"
#include "rage.hh"
#include <cstdint>
#include <string_view>

/*******************************************************/
template <typename T = uint32_t> class ParserEnumEquate
{
    T *pData;

    struct TranslationTable
    {
        uint32_t nHash;
        T        nEquatedValue;
    } * Table;

public:
    ParserEnumEquate (void *Table, void *data)
    {
        pData       = reinterpret_cast<T *> (data);
        this->Table = reinterpret_cast<TranslationTable *> (Table);
    }

    uint32_t
    ToHash ()
    {
        TranslationTable *t = Table;
        while (t->nHash != 0 && t->nEquatedValue != -1u)
            {
                if (*pData == t->nEquatedValue)
                    return t->nHash;

                t++;
            }
        return -1;
    }

    T &
    GetValue ()
    {
        return *pData;
    }

    void
    operator= (uint32_t hash)
    {
        TranslationTable *t = Table;
        while (t->nHash != 0 && t->nEquatedValue != -1)
            {
                if (hash == t->nHash)
                    *pData = t->nEquatedValue;

                t++;
            }
    }

    bool
    operator== (uint32_t hash)
    {
        return hash == ToHash ();
    }
};

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

    static ParserEnumEquate<> FindFieldEnum (parStructureStaticData *data,
                                             void *ptr, uint32_t hash);

    template <typename P>
    inline static ParserEnumEquate<>
    FindFieldEnum (parStructureStaticData *data, P *ptr, uint32_t hash)
    {
        return FindFieldEnum (data, (void *) ptr, hash);
    }

    template <typename T, typename P>
    inline static T &
    FindFieldPtr (parStructureStaticData *data, P *ptr, uint32_t hash)
    {
        return *reinterpret_cast<T *> (FindFieldPtr (data, (void *) ptr, hash));
    }
};

template <typename T>
constexpr auto
type_name () noexcept
{
    std::string_view name = "Error: unsupported compiler", prefix, suffix;
#ifdef __clang__
    name   = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name   = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name   = __FUNCSIG__;
    prefix = "auto __cdecl type_name<class ";
    suffix = ">(void) noexcept";
#endif
    name.remove_prefix (prefix.size ());
    name.remove_suffix (suffix.size ());
    return name;
}

/*******************************************************/
template <typename Class> class ParserWrapper
{
protected:
    inline static parStructureStaticData *pStaticData = nullptr;

public:
    using Type = Class;

    inline static constexpr uint32_t
    GetHash ()
    {
        return rage::atLiteralStringHash (type_name<Class> ());
    }

    inline static constexpr uint32_t
    GetLowercaseHash ()
    {
        return rage::atStringHash (type_name<Class> ());
    }

    void *
    GetPtr ()
    {
        return reinterpret_cast<void *> (static_cast<Class *> (this));
    }

    parStructureStaticData *
    GetStaticData () const
    {
        if (!pStaticData)
            pStaticData = ParserUtils::FindStaticData (GetHash ());

        return pStaticData;
    }

    ParserEnumEquate<>
    Equate (uint32_t fieldHash)
    {
        return ParserUtils::FindFieldEnum (GetStaticData (),
                                           static_cast<Class *> (this),
                                           fieldHash);
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
        return ParserUtils::FindFieldPtr<T> (GetStaticData (),
                                             static_cast<const Class *> (this),
                                             fieldHash);
    }

    template <typename T>
    void
    Set (uint32_t fieldHash, T value)
    {
        Get<T> (fieldHash) = value;
    }
};
