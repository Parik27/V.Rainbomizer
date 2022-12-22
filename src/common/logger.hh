#pragma once

#include <cstdio>
#include <string>

namespace Rainbomizer {

class Logger
{
    static inline FILE *mFile = nullptr;
    static FILE *       GetLogFile ();

public:
    static void LogMessage (const char *format, ...);

    static uint32_t GetGameBuild ();
};

} // namespace Rainbomizer
