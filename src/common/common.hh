#pragma once

#include "common/config.hh"
#include "common/logger.hh"
#include "memory/GameAddress.hh"
#include "memory/Pattern.hh"
#include <vector>
#include <functional>
#include <cstdio>
#include <string>

class CPed;
class gameSkeleton;

namespace Rainbomizer {
class Common
{
    static inline bool mHashesInitialised = false;

    static inline std::vector<uint32_t> mVehicleHashes;
    static inline std::vector<uint32_t> mPedHashes;

    static void InitialiseHashes ();

public:
    static const std::vector<uint32_t> &GetVehicleHashes ();
    static const std::vector<uint32_t> &GetPedHashes ();

    static std::string GetRainbomizerFileName (const std::string &name,
                                               const std::string &subdirs = "",
                                               bool               temp = false);

    static FILE *GetRainbomizerFile (const std::string &name,
                                     const std::string &mode,
                                     const std::string &subdirs      = "",
                                     bool               tempFallback = false);

    static FILE *GetRainbomizerDataFile (const std::string &name,
                                         const std::string &mode = "r");

    static bool VerifyAndValidatePatterns (const std::string &randomizerName)
    {
        if (!ConfigManager::ReadGlobalBool("ValidatePatterns"))
            return true;

        if (!VerifyLocalPatterns ())
            {
                Logger::LogMessage ("Randomizer '%s' will be disabled because "
                                    "its required patterns failed to resolve",
                                    randomizerName.c_str ());
                return false;
            }

        return false;
    }
};
} // namespace Rainbomizer
