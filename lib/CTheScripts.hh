#pragma once

#include <vector>
#include <type_traits>
#include "rage.hh"
#include "scrThread.hh"
#include <unordered_map>
#include <memory>
#include <string>

class CTheScripts
{
public:
    static atArray<scrThread *> *aThreads;
    static void InitialisePatterns ();
};

/* Utility Classes */
class NativeManager
{
    using NativeFunc = void(*)(scrThread::Info*);

    struct ScriptHookInfo
    {
        std::unordered_map<uint32_t, NativeFunc> mNatives;
    };        
    inline static ScriptHookInfo m_ScriptHookInfo;

    static int FindNativesTableWidth (uint64_t*);
    static int FindNativesVersionOffset (uint64_t*, uint32_t);
    static void InitialiseNativeCmdsFromTable (uint64_t *table, uint32_t step,
                                               uint32_t offset);

    static std::unique_ptr<NativeFunc[]>
    GetCmdsFromHashes (const std::vector<uint64_t>& hashes);

public:
    static uint32_t GetJoaatHashFromCmdHash (uint64_t hash);

    static void Initialise ();

    static inline void
    InvokeNative (uint32_t hash, scrThread::Info *info)
    {
        m_ScriptHookInfo.mNatives.at (hash) (info);
    }

    static bool
    DoesNativeExist (uint32_t hash)
    {
        return m_ScriptHookInfo.mNatives.count (hash);
    }

    template <typename Ret, typename... Args>
    static Ret
    InvokeNative (uint32_t hash, Args... args)
    {
        scrThread::Info info (args...);

        InvokeNative (hash, &info);
        
        if constexpr (!std::is_same_v<Ret, void>)
            {
                return info.GetReturn<Ret> ();
            }
    }
};

namespace NativeLiterals {
class NativeWrapper
{
    // Helper class to cast types to return values
    class ReturnType
    {
        uint64_t Value;

    public:
        template <typename T> operator T () { return *(T *) &Value; }
    };

public:

    uint32_t hash;

    NativeWrapper (uint32_t hash) : hash (hash) {}

    template <typename... Args>
    inline ReturnType
    operator() (Args... args)
    {
        return NativeManager::InvokeNative<ReturnType> (hash, args...);
    }
};

inline NativeWrapper operator"" _n (char const *s, size_t len)
{
    return NativeWrapper (rage::atLiteralStringHash (s, len));
}
}; // namespace NativeLiterals
