#include "CTheScripts.hh"
#include "Patterns/Patterns.hh"
#include "Utils.hh"
#include "NativeTranslationTable.hh"

#include "common/events.hh"
#include "common/logger.hh"
#include "common/common.hh"

#include <cstdint>
#include <memory>
#include <fstream>

#include <sstream>
#include <string>
#include <windows.h>

CEntity *(*fwScriptGuid_GetBaseFromGuid) (uint32_t) = nullptr;
uint32_t (*fwScriptGuid_CreateGuid) (CEntity *)     = nullptr;

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
                                              size_t offset, int totalNatives)
{
    std::vector<uint64_t> tmpHashes;
    std::vector<uint64_t> origHashes;
    std::vector<uint64_t> newHashes;

    for (int i = 0; i < totalNatives; i++)
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

    std::ifstream crosstableFile("Crossmap.txt");
    if (!m_ScriptHookInfo.bAvailable && crosstableFile)
        {
            std::vector<std::vector<uint64_t>> crosstable;
            
            std::string line;
            int tableWidth = -1;

            // Read cross table file
            while (std::getline (crosstableFile, line))
                {
                    auto &nativeEntry = crosstable.emplace_back ();

                    std::istringstream ss (line);
                    std::string        hashEntry;

                    while (getline (ss, hashEntry, ' '))
                        {
                            nativeEntry.push_back (
                                                   std::stoull (hashEntry, nullptr, 16));
                        }

                    if (tableWidth == -1)
                        tableWidth = nativeEntry.size ();

                    tableWidth
                        = std::min (tableWidth, int (nativeEntry.size ()));
                }

            Rainbomizer::Logger::LogMessage ("Num natives in crosstable: %x", crosstable.size());
            
            // Convert crosstable vector to something the existing functions can read.
            if (crosstable.size() == 0)
                return;

            Rainbomizer::Logger::LogMessage ("Native %llx -> %llx", crosstable[20][0], crosstable[20][1]);
            
            m_ScriptHookInfo.bAvailable = true;

            auto tableMem
                = std::make_unique<uint64_t[]> (crosstable.size () * tableWidth);

            for (int i = 0; i < crosstable.size (); i++)
                {
                    memcpy (tableMem.get () + i * tableWidth,
                            crosstable[i].data (), tableWidth * 8);
                }

            size_t verOffset
                = 1;

            m_ScriptHookInfo.bAvailable = true;

            Rainbomizer::Logger::LogMessage ("Table width: %d (%llx -> %llx)", verOffset, tableMem.get()[20*tableWidth + 1], tableMem.get()[20*tableWidth]);
            
            InitialiseNativeCmdsFromTable (tableMem.get (), tableWidth,
                                           verOffset, crosstable.size());
            InitialiseNativeHooks ();
        }
    else
        {
            Rainbomizer::Logger::LogMessage ("Crossmap.txt file not found or ScriptHook available");
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

    ConvertCall (hook::get_pattern (
                     "48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8",
                     -0x68),
                 fwScriptGuid_CreateGuid);

    Rainbomizer::Events ().OnInit +=
        [] (bool) { NativeManager::Initialise (); };
}

/*******************************************************/
uint32_t
fwScriptGuid::CreateGuid (CEntity *entity)
{
    return fwScriptGuid_CreateGuid (entity);
}

/*******************************************************/
CEntity *
fwScriptGuid::GetBaseFromGuid (uint32_t guid)
{
    return fwScriptGuid_GetBaseFromGuid (guid);
}

atArray<scrThread *> *CTheScripts::aThreads;
