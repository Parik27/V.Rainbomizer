#include "logger.hh"
#include "common.hh"
#include <ctime>
#include <windows.h>
#include <Utils.hh>

#ifndef NDEBUG
#define RAINBOMIZER_BUILD "Debug Build: " __DATE__ " " __TIME__
#else
#define RAINBOMIZER_BUILD "Build v0.1 - Beta: " __DATE__ " " __TIME__
#endif

namespace Rainbomizer {

/*******************************************************/
std::string
GetTimeNow ()
{
    time_t currentTime;
    char   str[256];

    time (&currentTime);

    auto tm = std::localtime (&currentTime);
    sprintf (str, "%04d-%02d-%02d", 1900 + tm->tm_year, tm->tm_mon + 1,
             tm->tm_mday);

    return str;
}

/*******************************************************/
FILE *
Logger::GetLogFile ()
{
    if (!mFile)
        {
            mFile = Common::GetRainbomizerFile (GetTimeNow () + ".txt", "a+",
                                                "logs/");
            if (!mFile)
                {
                    MessageBox (NULL, "Unable to open log file", "Error",
                                MB_ICONHAND);
                    mFile = stdout;
                }

            fprintf (mFile, "===========================================\n");
            fprintf (mFile, "%d\n", (int) time (NULL));
            fprintf (mFile, "Rainbomizer V Build: %s \n",
                     RAINBOMIZER_BUILD);
            fprintf (mFile, "===========================================\n");
        }

    return mFile;
}

/*******************************************************/
//?? 8d ?? ?? ?? ?? ?? ?? 8d ?? ?? 30 ?? 8b c0 e8 ?? ?? ?? ?? ?? 8d ?? ?? 30 b2 01     
    
/*******************************************************/
void
Logger::LogMessage (const char *format, ...)
{
    FILE *file = GetLogFile ();
    fprintf (file, "[%d]: ", int (time (NULL)));

    va_list args;
    va_start (args, format);
    vfprintf (file, format, args);
    va_end (args);

    fputc ('\n', file);
    fflush (file);
}

} // namespace Rainbomizer
