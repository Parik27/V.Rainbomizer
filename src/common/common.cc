#include "Utils.hh"
#include "common.hh"
#include "CStreaming.hh"
#include "CModelInfo.hh"
#include <cstring>
#include <filesystem>
#include "logger.hh"
#include <rage.hh>
#include <Natives.hh>
#include <cstdlib>
#include <system_error>
#include <array>

void (*gameSkeleton__Init) (gameSkeleton *, uint32_t);

namespace Rainbomizer {

/*******************************************************/
void
Common::InitialiseHashes ()
{
    for (size_t i = 0; i < CStreaming::ms_aModelPointers->m_nAllocated; i++)
        {
            auto info = CStreaming::ms_aModelPointers->m_pData[i];
            if (!info)
                continue;

            auto type = info->GetType ();

            switch (type)
                {
                    case MODEL_INFO_PED: {
                        mPedHashes.push_back (info->m_nHash);

                        break;
                    }

                    case MODEL_INFO_VEHICLE: {

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
const std::vector<uint32_t> &
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
                                const std::string &subdirs, bool temp)
{
    std::string baseDir
        = GetGameDirRelativePathA (("rainbomizer/" + subdirs).c_str ());

    if (!std::filesystem::exists (GetGameDirRelativePathA ("rainbomizer/")))
        {
            MessageBox (NULL,
                        "Failed to locate Rainbomizer Data "
                        "Directory.\n\nPlease reinstall the mod following "
                        "the instructions in the README.",
                        "Installation Validation Error", MB_ICONHAND);
            abort ();
        }

    if (temp)
        baseDir = std::filesystem::temp_directory_path ().string ()
                  + "/rainbomizer/" + subdirs;

    std::error_code ec;
    std::filesystem::create_directories (baseDir, ec);

    return baseDir + name;
}

/*******************************************************/
FILE *
Common::GetRainbomizerFile (const std::string &name, const std::string &mode,
                            const std::string &subdirs, bool tempFallback)
{
    FILE *file = fopen (GetRainbomizerFileName (name, subdirs).c_str (),
                        mode.c_str ());

    if (!file && tempFallback)
        return fopen (GetRainbomizerFileName (name, subdirs, true).c_str (),
                      mode.c_str ());

    return file;
}

/*******************************************************/
FILE *
Common::GetRainbomizerDataFile (const std::string &name,
                                const std::string &mode)
{
    FILE *f = GetRainbomizerFile (name, mode, "data/");

    if (!f)
        Logger::LogMessage ("Failed to read Rainbomizer data file: data/%s: %s",
                            name.c_str (), std::strerror (errno));

    return f;
}

/*******************************************************/
const std::vector<uint32_t> &
Common::GetPedHashes ()
{
    if (!mHashesInitialised)
        InitialiseHashes ();

    return mPedHashes;
}
} // namespace Rainbomizer
