#include "CTheScripts.hh"
#include "Utils.hh"
#include "NativeTranslationTable.hh"
#include "common/common.hh"
#include <memory>

#include <windows.h>

/*******************************************************/
std::unique_ptr<NativeManager::NativeFunc[]>
NativeManager::GetCmdsFromHashes (const std::vector<uint64_t>& hashes)
{
    auto cmds = std::make_unique<NativeManager::NativeFunc[]>(hashes.size());
    memcpy(cmds.get(), hashes.data(), hashes.size() * sizeof(uint64_t));

    scrProgram program;
    program.m_nNativesCount = hashes.size ();
    program.m_pNativesPointer = (void**) cmds.get();
    program.InitNativesTable ();

    return cmds;
}

/*******************************************************/
int
NativeManager::FindNativesVersionOffset (uint64_t *hashes, uint32_t width)
{
    std::vector<uint64_t> tmpHashes = {-1ull};
    for (int i = 0; i < width; ++i)
        tmpHashes.push_back (hashes[i]);

    auto cmds = GetCmdsFromHashes (tmpHashes);
    for (int i = 1; i < tmpHashes.size (); ++i)
        {
            if (cmds[i] != cmds[0])
                return i - 1;
        }

    return -1;
}

/*******************************************************/
int
NativeManager::FindNativesTableWidth (uint64_t *table)
{
    for (int i = 0;; i++)
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
    return NativeTranslationMap.at (hash);
}

/*******************************************************/
void
NativeManager::InitialiseNativeCmdsFromTable (uint64_t *table, uint32_t step,
                                              uint32_t offset)
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

    auto  cmds = GetCmdsFromHashes (tmpHashes);
    for (int i = 0; i < tmpHashes.size (); i++)
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

                    for (int i = 0; i < program->m_nNativesCount; i++)
                        {
                            uint64_t funcHash = reinterpret_cast<uint64_t &> (
                                program->m_pNativesPointer[i]);

                            if (funcHash == orig.newHash)
                                m_Operations.push_back (
                                    {reinterpret_cast<NativeFunc *> (
                                         &program->m_pNativesPointer[i]),
                                     hooked});
                        }
                }
        }

    bool ret = O (program);

    for (const auto& operation : m_Operations)
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

    HMODULE scriptHook = LoadLibrary (TEXT ("ScriptHookV.dll"));
    if (scriptHook)
        {
            hook::pattern p (scriptHook, "B6 E5 5E D5 E7 A8 6A 84 C9");
            uint64_t *    table = p.count (1).get_first<uint64_t> ();

            int numVersions = FindNativesTableWidth (table);
            int verOffset   = FindNativesVersionOffset (table, numVersions);

            InitialiseNativeCmdsFromTable (table, numVersions, verOffset);
            FreeLibrary (scriptHook);

            InitialiseNativeHooks ();
        }
}

/*******************************************************/
void
CTheScripts::InitialisePatterns ()
{
    aThreads = GetRelativeReference<atArray<scrThread *>> (
        "8b 0d ? ? ? ? 3b ca 7d ? ? 8b 0d ", 13, 17);

    Rainbomizer::Common::AddInitCallback (
        [] (bool session) { NativeManager::Initialise (); });
}

atArray<scrThread *>* CTheScripts::aThreads;
