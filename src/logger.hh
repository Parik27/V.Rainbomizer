#pragma once

#include <cstdio>
#include <string>

namespace Rainbomizer {

class Logger
{
    static FILE *mFile;
    static FILE *GetLogFile ();

public:
    static void LogMessage (const char *format, ...);
};

} // namespace Rainbomizer
