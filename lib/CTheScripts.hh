#pragma once

#include <vector>
#include <type_traits>
#include "rage.hh"
#include "scrThread.hh"
#include <unordered_map>
#include <memory>
#include <string>
#include <CEntity.hh>

class CTheScripts
{
public:
    static atArray<scrThread *> *aThreads;
    static void                  InitialisePatterns ();
};

class fwScriptGuid
{
public:
    static CEntity *GetBaseFromGuid (uint32_t guid);
};

/* Utility Classes */
class NativeManager
{
public:
    using NativeFunc = void (*) (scrThread::Info *);

private:
    struct NativeInfo
    {
        NativeFunc func;
        uint64_t   newHash;
        uint64_t   originalHash;
    };

    struct ScriptHookInfo
    {
        bool                                     bAvailable;
        std::unordered_map<uint32_t, NativeInfo> mNatives;
    };
    inline static ScriptHookInfo m_ScriptHookInfo;

    template <auto &O> static bool ProcessNativeHooks (scrProgram *program);

    static auto &
    GetHookedNativesList ()
    {
        static std::unordered_map<uint32_t, NativeFunc> List;
        return List;
    }

    static size_t FindNativesTableWidth (uint64_t *);
    static size_t FindNativesVersionOffset (uint64_t *, size_t);
    static void   InitialiseNativeCmdsFromTable (uint64_t *table, size_t step,
                                                 size_t offset);

    static std::unique_ptr<NativeFunc[]>
    GetCmdsFromHashes (const std::vector<uint64_t> &hashes);

    static void InitialiseNativeHooks ();

public:
    static uint32_t GetJoaatHashFromCmdHash (uint64_t hash);

    static void Initialise ();

    static void
    HookNative (uint32_t hash, NativeFunc hookedFunc)
    {
        GetHookedNativesList ()[hash] = hookedFunc;
    }

    static inline void
    InvokeNative (uint32_t hash, scrThread::Info *info)
    {
        if (!IsScriptHookAvailable ())
            return;

        m_ScriptHookInfo.mNatives.at (hash).func (info);
    }

    static bool
    DoesNativeExist (uint32_t hash)
    {
        return m_ScriptHookInfo.mNatives.count (hash);
    }

    static bool
    IsScriptHookAvailable ()
    {
        return m_ScriptHookInfo.bAvailable;
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

class NativeCallbackMgr
{
    using CombinedCallbackFunc = void (*) (scrThread::Info *, bool);

    template <uint32_t hash, CombinedCallbackFunc cb>
    static void
    Trampoline (scrThread::Info *info)
    {
        cb (info, true);
        NativeManager::InvokeNative (hash, info);
        cb (info, false);
    }

    template <uint32_t hash, NativeManager::NativeFunc cb, bool before>
    static void
    Trampoline (scrThread::Info *info)
    {
        if constexpr (before)
            cb (info);

        NativeManager::InvokeNative (hash, info);

        if constexpr (!before)
            cb (info);
    }

public:
    template <uint32_t hash, NativeManager::NativeFunc cb, bool before>
    static void
    Add ()
    {
        NativeManager::HookNative (hash, Trampoline<hash, cb, before>);
    }

    template <uint32_t hash, CombinedCallbackFunc cb>
    static void
    Add ()
    {
        NativeManager::HookNative (hash, Trampoline<hash, cb>);
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

    void
    Hook (NativeManager::NativeFunc func)
    {
        NativeManager::HookNative (hash, func);
    }
};

inline NativeWrapper operator"" _n (char const *s, size_t len)
{
    return NativeWrapper (
        rage::atLiteralStringHash (std::string_view (s, len)));
}
} // namespace NativeLiterals
