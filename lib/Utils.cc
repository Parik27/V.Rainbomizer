#include <random>
#include <ctime>

#include "CHud.hh"
#include "CStreaming.hh"
#include "CPed.hh"
#include "Natives.hh"
#include "scrThread.hh"
#include "CItemInfo.hh"
#include "CutSceneManager.hh"
#include "CPools.hh"
#include "CTheScripts.hh"
#include "CText.hh"
#include "audEngine.hh"
#include "CWeather.hh"
#include "CTimecycle.hh"
#include "sysMemoryAllocator.hh"
#include "ParserUtils.hh"
#include "CHandlingDataMgr.hh"
#include "phBound.hh"
#include "CTask.hh"
#include "CModelIndices.hh"
#include "CCredits.hh"
#include "CAmbientModelSets.hh"

#include "Utils.hh"

/*******************************************************/
std::mt19937 &
RandEngine ()
{
    thread_local static std::mt19937 engine{(unsigned int) time (NULL)};

    return engine;
}

/*******************************************************/
int
RandomInt (int min, int max)
{
    std::uniform_int_distribution<int> dist{min, max};
    return dist (RandEngine ());
}

/*******************************************************/
unsigned int
RandomWeighed (const std::vector<double> &weights)
{
    std::discrete_distribution<unsigned int> dist{weights.begin (),
                                                  weights.end ()};
    return dist (RandEngine ());
}

/*******************************************************/
int
RandomInt (int max)
{
    return RandomInt (0, max);
}

/*******************************************************/
size_t
RandomSize (size_t max)
{
    std::uniform_int_distribution<size_t> dist{0, max};
    return dist (RandEngine ());
}

/*******************************************************/
float
RandomFloat (float min, float max)
{
    std::uniform_real_distribution<float> dist{min, max};
    return dist (RandEngine ());
}

/*******************************************************/
float
RandomFloat (float max)
{
    return RandomFloat (0, max);
}

/*******************************************************/
bool
RandomBool (float Odds, float Precision)
{
    return RandomFloat (100.0f * Precision) < (Odds * Precision);
}

/*******************************************************/
void
MakeJMP64 (injector::memory_pointer_tr at, injector::memory_pointer_raw dest)
{
    const uint8_t prologue[] = {0x48, 0xB8};
    const uint8_t epilogue[] = {0xFF, 0xE0};

    memcpy (at.get (), prologue, sizeof (prologue));
    memcpy ((at + 2).get (), &dest, sizeof (dest));
    memcpy ((at + 10).get (), epilogue, sizeof (epilogue));
}

/*******************************************************/
void
RegisterJmpHook (void *addr, void *dst, void **outOrignal, int size)
{
    LPVOID mem = Trampoline::MakeTrampoline (GetModuleHandle (nullptr))
                     ->Space (size + 5, 1);
    memcpy (mem, addr, size);
    injector::MakeJMP (uintptr_t (mem) + size, uintptr_t (addr) + size);
    MakeJMP64 (addr, dst);

    *outOrignal = mem;
}

/*******************************************************/
void
InitialiseAllComponents ()
{

    static bool initialised = false;

    if (initialised)
        return;

    CHud::InitialisePatterns ();
    CStreaming::InitialisePatterns ();
    CPedInventory::InitialisePatterns ();
    CWeaponInfoManager::InitialisePatterns ();
    scrThread::InitialisePatterns ();
    cutfCutsceneFile2::InitialisePatterns ();
    CPools::InitialisePatterns ();
    CTheScripts::InitialisePatterns ();
    CText::InitialisePatterns ();
    audEngine::InitialisePatterns ();
    CWeather::InitialisePatterns ();
    tcManager::InitialisePatterns ();
    sysUseAllocator::InitialisePatterns ();
    CHandlingDataMgr::InitialisePatterns ();
    phBound::InitialisePatterns ();
    aiTask::InitialisePatterns ();
    CModelIndices::InitialisePatterns ();
    CCreditArray::InitialisePatterns ();
    CAmbientModelSetsManager::InitialisePatterns ();

#ifndef NO_PARSER_UTILS
    ParserUtils::Register ();
#endif

    InitialiseNatives ();

    initialised = true;
}
