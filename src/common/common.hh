#pragma once

#include <vector>
#include <functional>
#include <cstdio>

class CPed;

namespace Rainbomizer {
class Common
{
    static bool mHashesInitialised;

    static std::vector<int> mVehicleHashes;
    static std::vector<int> mPedHashes;

    static void  InitialiseHashes ();
    static void ProcessInitCallbacks ();

    static void InitialiseInitCallbackHook ();
    static std::vector<std::function<void ()>> &GetCallbacks ();

public:
    
    static std::vector<int> &GetVehicleHashes ();
    static std::vector<int> &GetPedHashes ();

    static std::string GetRainbomizerFileName (const std::string &name,
                                               const std::string &subdirs = "");

    static FILE *GetRainbomizerFile (const std::string &name,
                                     const std::string &mode,
                                     const std::string &subdirs = "");

    static FILE *GetRainbomizerDataFile (const std::string &name,
                                         const std::string &mode     = "r");

    // Called after indices are initialised, so that's pretty late? :thinking:
    static void AddInitCallback (std::function<void ()> callback);
};
}; // namespace Rainbomizer
