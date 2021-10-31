#include <optional>
#include <string>

#include <common/minhook.hh>
#include <common/config.hh>
#include <common/logger.hh>

#include <CTheScripts.hh>
#include <Utils.hh>

#include "mods.hh"

class ScriptHookCompatibility
{
    struct Context
    {
        bool                      Valid = false;
        scrThread::Info           info;
        NativeManager::NativeFunc func = nullptr;

        Context (){};

        void
        Reset ()
        {
            Valid = false;
            info  = scrThread::Info ();
            func  = nullptr;
        }

    } inline static sm_Ctx;

    template <auto &nativeInit>
    static void
    NativeInitHook (uint64_t hash)
    {
        uint32_t joaatHash = NativeManager::GetJoaatHashFromCmdHash (hash);
        sm_Ctx.Reset ();

        if (auto hook
            = LookupMap (NativeManager::GetHookedNativesList (), joaatHash))
            {
                sm_Ctx.Valid = true;
                sm_Ctx.func  = hook->Cb;

                return;
            }
        nativeInit (hash);
    }

    /*******************************************************/
    template <auto &nativePush64>
    static void
    NativePushHook (uint64_t value)
    {
        if (!sm_Ctx.Valid)
            return nativePush64 (value);

        sm_Ctx.info.PushArg (value);
    }

    /*******************************************************/
    template <auto &nativeCall>
    static void *
    NativeCallHook ()
    {
        static uint64_t                   ret;
        ModCompatibility::InModScriptRAII inModScript;

        if (!sm_Ctx.Valid)
            return nativeCall ();

        sm_Ctx.func (&sm_Ctx.info);
        ret = sm_Ctx.info.GetReturn () & 0xFFFFFFFF;

        return &ret;
    }

public:
    static void
    Initialise ()
    {
        HMODULE scriptHook = LoadLibrary (TEXT ("ScriptHookV.dll"));
        if (!scriptHook)
            return;

        Rainbomizer::Logger::LogMessage (
            "Initialising ScriptHook compatibility hooks");

        REGISTER_MH_HOOK_API ("ScriptHookV.dll", "?nativeInit@@YAX_K@Z",
                              NativeInitHook, void, uint64_t);
        REGISTER_MH_HOOK_API ("ScriptHookV.dll", "?nativePush64@@YAX_K@Z",
                              NativePushHook, void, uint64_t);
        REGISTER_MH_HOOK_API ("ScriptHookV.dll", "?nativeCall@@YAPEA_KXZ",
                              NativeCallHook, void *);
    }
};

/*******************************************************/
/*Because so many people apparently use Menyoo, I make sure that it doesn't
 * cause softlocks with this option enabled so force disable it. If it breaks,
 * just don't use Menyoo ez 4Head Kappa */
/*******************************************************/
class MenyooConfigObliterator
{
    static constexpr const char *MENYOO_CONFIG_PATH
        = "menyooStuff/menyooConfig.ini";

public:
    static std::string
    ReadAndUpdateConfig ()
    {
        FILE *f = fopen (MENYOO_CONFIG_PATH, "r");

        if (!f)
            return "";

        char line[1024] = {0};

        std::string buffer = "";

        while (fgets (line, 512, f))
            {
                std::string lineStr = line;
                if (lineStr.find ("DeathModelReset") != lineStr.npos)
                    {
                        // Don't write if DeathModelReset is already false
                        if (lineStr.find ("DeathModelReset = false")
                            != lineStr.npos)
                            return "";
                        else
                            lineStr = "DeathModelReset = false\n";
                    }

                buffer += lineStr;
            }

        fclose (f);
        return buffer;
    }

    /*******************************************************/
    static void
    Initialise ()
    {
        std::string buffer = ReadAndUpdateConfig ();

        if (!buffer.size ())
            return;

        FILE *f = fopen (MENYOO_CONFIG_PATH, "w");
        if (!f)
            return;

        fwrite (buffer.data (), 1, buffer.size (), f);

        fclose (f);
    }
};

/*******************************************************/
ModCompatibility::ModCompatibility ()
{
    bool MenyooConfigObliterator = true;
    bool ScriptHookHook          = true;

    if (!ConfigManager::ReadConfig (
            "ModCompatibility",
            std::pair ("MenyooConfigEdit", &MenyooConfigObliterator),
            std::pair ("ScriptHookEdit", &ScriptHookHook),
            std::pair ("RandomizeMods", &GetShouldRandomizeMods ())))
        return;

    if (MenyooConfigObliterator)
        MenyooConfigObliterator::Initialise ();

    if (ScriptHookHook)
        ScriptHookCompatibility::Initialise ();

    if (!GetShouldRandomizeMods () && !ScriptHookHook)
        {
            Rainbomizer::Logger::LogMessage (
                "Randomize mods disabled, but ScriptHookCompatibility not "
                "initialised");
        }
}

ModCompatibility g_ModCompatibility;
