#include "Utils.hh"
#include "common.hh"
#include "CStreaming.hh"
#include "CModelInfo.hh"
#include <filesystem>
#include "logger.hh"
#include <rage.hh>
#include <Natives.hh>

void (*InitialiseModelIndices_34431a) ();

namespace Rainbomizer {

/*******************************************************/
void
Common::ProcessInitCallbacks ()
{
    InitialiseModelIndices_34431a ();
    
    for (const auto& i : GetCallbacks())
        i ();
}

/*******************************************************/
void
Common::InitialiseInitCallbackHook ()
{
    static bool isInitialised = false;
    if (std::exchange (isInitialised, true))
        return;

    RegisterHook (
        "e8 ? ? ? ? e8 ? ? ? ? ? 8d 0d ? ? ? ? ba 04 00 00 00", 5,
        InitialiseModelIndices_34431a, ProcessInitCallbacks);
}

/*******************************************************/
std::vector<std::function<void ()>> &
Common::GetCallbacks ()
{
    static std::vector<std::function<void ()>> callbacks;

    InitialiseInitCallbackHook ();
    return callbacks;
}

/*******************************************************/
void
Common::InitialiseHashes ()
{
    auto timestamp = clock ();

    for (size_t i = 0; i < CStreaming::ms_aModelPointers->m_nAllocated; i++)
        {
            auto info = CStreaming::ms_aModelPointers->m_pData[i];
            if(!info)
                continue;
            
            auto type = info->GetType ();

            switch (type)
                {
                    case MODEL_INFO_PED: {
                        mPedHashes.push_back (info->m_nHash);

                        break;
                    }

                    case MODEL_INFO_VEHICLE: {
                        auto vehInfo = static_cast<CVehicleModelInfo*>(info);

                        if (vehInfo->GetVehicleType ()
                            != "VEHICLE_TYPE_TRAIN"_joaat)
                            mVehicleHashes.push_back (info->m_nHash);

                        break;
                    }

                default: break;
                }
        }

       
    Logger::LogMessage ("Initialised model hashes");
    Logger::LogMessage ("Ped models: %d, Vehicle models: %d",
                        mPedHashes.size (), mVehicleHashes.size ());

    mHashesInitialised = true;
}

/*******************************************************/
std::vector<int> &
Common::GetVehicleHashes ()
{
    if (!mHashesInitialised)
        InitialiseHashes ();

    return mVehicleHashes;
}

struct GamePathA
{
    char _path[160 + 1];
    char _temp_path[160 + 1];

    GamePathA ()
    {
        _path[0]  = '\0';
        HMODULE h = NULL;
        h         = GetModuleHandleA (NULL);
        GetModuleFileNameA (h, _path, MAX_PATH);
        char *bslp = strrchr (_path, '\\');
        char *fslp = strrchr (_path, '/');
        char *slp  = (std::max) (bslp, fslp);
        if (slp)
            slp[1] = '\0';
    }
};

/*******************************************************/
char *
GetGameDirRelativePathA (const char *subpath)
{
    static GamePathA gamePath;
    strcpy (gamePath._temp_path, gamePath._path);
    strcat (gamePath._temp_path, subpath);
    return gamePath._temp_path;
}

/*******************************************************/
std::string
Common::GetRainbomizerFileName (const std::string &name,
                                const std::string &subdirs)
{
    std::string baseDir
        = GetGameDirRelativePathA (("rainbomizer/" + subdirs).c_str ());
    std::filesystem::create_directories (baseDir);
    return baseDir + name;
}

/*******************************************************/
FILE *
Common::GetRainbomizerFile (const std::string &name, const std::string &mode,
                            const std::string &subdirs)
{
    return fopen (GetRainbomizerFileName (name, subdirs).c_str (),
                  mode.c_str ());
}

/*******************************************************/
FILE *
Common::GetRainbomizerDataFile (const std::string &name,
                                const std::string &mode)
{
    FILE *f = GetRainbomizerFile (name, mode, "data/");
    
    if (!f)
        Logger::LogMessage ("Failed to read Rainbomizer data file: data/%s",
                            name.c_str ());

    return f;
}

/*******************************************************/
void
Common::AddInitCallback (std::function<void ()> callback)
{
    GetCallbacks().push_back(callback);
}
    
/*******************************************************/
std::vector<int> &
Common::GetPedHashes ()
{
    if (!mHashesInitialised)
        InitialiseHashes ();

    return mPedHashes;
}

bool             Common::mHashesInitialised = false;
std::vector<int> Common::mVehicleHashes;
std::vector<int> Common::mPedHashes;
}; // namespace Rainbomizer
