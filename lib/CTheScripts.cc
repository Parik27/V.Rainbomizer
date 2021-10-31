#include "CTheScripts.hh"
#include "Patterns/Patterns.hh"
#include "Utils.hh"
#include "NativeTranslationTable.hh"
#include "common/events.hh"
#include <memory>

#include <windows.h>

CEntity *(*fwScriptGuid_GetBaseFromGuid) (uint32_t) = nullptr;

/*******************************************************/
std::unique_ptr<NativeManager::NativeFunc[]>
NativeManager::GetCmdsFromHashes (const std::vector<uint64_t> &hashes)
{
    auto cmds = std::make_unique<NativeManager::NativeFunc[]> (hashes.size ());
    memcpy (cmds.get (), hashes.data (), hashes.size () * sizeof (uint64_t));

    scrProgram program;
    program.m_nNativesCount   = static_cast<uint32_t> (hashes.size ());
    program.m_pNativesPointer = (void **) cmds.get ();
    program.InitNativesTable ();

    return cmds;
}

/*******************************************************/
size_t
NativeManager::FindNativesVersionOffset (uint64_t *hashes, size_t width)
{
    std::vector<uint64_t> tmpHashes = {-1ull};
    for (size_t i = 0; i < width; ++i)
        tmpHashes.push_back (hashes[i]);

    auto cmds = GetCmdsFromHashes (tmpHashes);
    for (size_t i = 1; i < tmpHashes.size (); ++i)
        {
            if (cmds[i] != cmds[0])
                return i - 1;
        }

    return -1;
}

/*******************************************************/
size_t
NativeManager::FindNativesTableWidth (uint64_t *table)
{
    for (size_t i = 0;; i++)
        {
            if (table[i] == 0xd3a58a12c77d9d4b)
                return i;
        }
    return -1;
}

/*******************************************************/
uint32_t
NativeManager::GetJoaatHashFromCmdHash (uint64_t hash)
{
    if (auto joaatHash = LookupMap (NativeTranslationMap, hash))
        return *joaatHash;

    return -1;
}

/*******************************************************/
void
NativeManager::InitialiseNativeCmdsFromTable (uint64_t *table, size_t step,
                                              size_t offset)
{
    std::vector<uint64_t> tmpHashes;
    std::vector<uint64_t> origHashes;
    std::vector<uint64_t> newHashes;

    const int TOTAL_NATIVES = 5253;
    for (int i = 0; i < TOTAL_NATIVES; i++)
        {
            if (!table[i * step])
                continue;
            tmpHashes.push_back (table[i * step + offset]);
            newHashes.push_back (table[i * step + offset]);
            origHashes.push_back (table[i * step]);
        }

    auto cmds = GetCmdsFromHashes (tmpHashes);
    for (size_t i = 0; i < tmpHashes.size (); i++)
        {

            if (NativeTranslationMap.count (origHashes[i]))
                {
                    m_ScriptHookInfo
                        .mNatives[GetJoaatHashFromCmdHash (origHashes[i])]
                        = {cmds[i], newHashes[i], origHashes[i]};
                }
        }
}

template <auto &O>
bool
NativeManager::ProcessNativeHooks (scrProgram *program)
{
    std::vector<std::pair<NativeFunc *, NativeFunc>> m_Operations;
    if (program->m_pCodeBlocks)
        {
            for (auto [hash, hooked] : GetHookedNativesList ())
                {
                    auto orig = m_ScriptHookInfo.mNatives.at (hash);

                    for (size_t i = 0; i < program->m_nNativesCount; i++)
                        {
                            uint64_t funcHash = reinterpret_cast<uint64_t &> (
                                program->m_pNativesPointer[i]);

                            if (funcHash == orig.newHash)
                                m_Operations.push_back (
                                    {reinterpret_cast<NativeFunc *> (
                                         &program->m_pNativesPointer[i]),
                                     hooked.Cb});
                        }
                }
        }

    bool ret = O (program);

    for (const auto &operation : m_Operations)
        *operation.first = operation.second;

    return ret;
}

/*******************************************************/
void
NativeManager::InitialiseNativeHooks ()
{
    REGISTER_HOOK ("8b cb e8 ? ? ? ? 8b 43 70 ? 03 c4 a9 00 c0 ff ff", 2,
                   ProcessNativeHooks, bool, scrProgram *);
}

/*******************************************************/
void
NativeManager::Initialise ()
{
    static bool initialised = false;

    if (std::exchange (initialised, true))
        return;

    m_ScriptHookInfo.bAvailable = false;

    HMODULE scriptHook = LoadLibrary (TEXT ("ScriptHookV.dll"));
    if (scriptHook)
        {
            hook::pattern p (scriptHook, "B6 E5 5E D5 E7 A8 6A 84 C9");
            uint64_t *    table = p.count (1).get_first<uint64_t> ();

            size_t numVersions = FindNativesTableWidth (table);
            size_t verOffset   = FindNativesVersionOffset (table, numVersions);

            m_ScriptHookInfo.bAvailable = true;

            InitialiseNativeCmdsFromTable (table, numVersions, verOffset);
            InitialiseNativeHooks ();

            FreeLibrary (scriptHook);
        }
}

/*******************************************************/
void
CTheScripts::InitialisePatterns ()
{
    aThreads = GetRelativeReference<atArray<scrThread *>> (
        "8b 0d ? ? ? ? 3b ca 7d ? ? 8b 0d ", 13, 17);

    ConvertCall (hook::get_pattern (
                     "83 f9 ff 74 ? ? 8b 0d ? ? ? ? 44 8b c1 ? 8b"),
                 fwScriptGuid_GetBaseFromGuid);

    Rainbomizer::Events ().OnInit +=
        [] (bool) { NativeManager::Initialise (); };
}

/*******************************************************/
CEntity *
fwScriptGuid::GetBaseFromGuid (uint32_t guid)
{
    return fwScriptGuid_GetBaseFromGuid (guid);
}

atArray<scrThread *> *CTheScripts::aThreads;
