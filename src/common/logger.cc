#include "logger.hh"
#include "common.hh"
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <Utils.hh>

#ifndef NDEBUG
#define RAINBOMIZER_BUILD "Debug Build: " __DATE__ " " __TIME__
#else
#define RAINBOMIZER_BUILD "Release v3.3.6: " __DATE__ " " __TIME__
#define RAINBOMIZER_BUILD_SHORT "Release v3.3.6"
#endif

constexpr int RAINBOMIZER_BUILD_NUMBER =
#include "../../build-count.txt"
    ;

char *(*rage__formatf6eb9) (char *, char const *, ...);

#ifdef ENABLE_DEBUG_MENU

#include "debug/logger.hh"

#endif

namespace Rainbomizer {

static uint32_t sg_GameBuild = 0;

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
                                                "logs/", true);
            if (!mFile)
                {
                    MessageBox (NULL, "Failed to open log file for writing.",
                                "Error", MB_ICONHAND);
                    mFile = stdout;
                }

            fprintf (mFile, "===========================================\n");
            fprintf (mFile, "%d\n", (int) time (NULL));
            fprintf (mFile, "Rainbomizer V Build: %s \n", RAINBOMIZER_BUILD);
            fprintf (mFile, "===========================================\n");
        }

    return mFile;
}

/*******************************************************/
void
Logger::LogMessage (const char *format, ...)
{
    FILE *file = GetLogFile ();
    fprintf (file, "[%d]: ", int (time (NULL)));

    va_list args;
    va_start (args, format);
    unsigned int size = vfprintf (file, format, args) + 1;
    va_end (args);

#ifdef ENABLE_DEBUG_MENU
    std::unique_ptr<char[]> buf (new char[size]);
    va_start (args, format);
    vsnprintf (buf.get (), size, format, args);
    va_end (args);

    LoggerDebugInterface::PublishLogMessage (buf.get ());
#endif

    fputc ('\n', file);
    fflush (file);
}

uint32_t
Logger::GetGameBuild ()
{
    return sg_GameBuild;
}

/*******************************************************/
class DisplayBuildVersion
{
    static char *
    AppendBuildVersion (char *out, char const *format, char *build,
                        char *version)
    {
        sg_GameBuild = std::stoi (version);
        Logger::LogMessage("Set Game Build to %d", sg_GameBuild);

#ifndef NDEBUG
        return rage__formatf6eb9 (out, "Rainbomizer Build %d   Build %s",
                                  RAINBOMIZER_BUILD_NUMBER + 1, version);
#else
        return rage__formatf6eb9 (out, "Rainbomizer %s   Build %s",
                                  RAINBOMIZER_BUILD_SHORT, version);
#endif
    }

public:
    DisplayBuildVersion ()
    {
        RegisterHook ("8d ? ? ? ? ? ? 8d ? ? ? ? 8b c0 e8 ? ? ? ? ? 8d ? ? ? "
                      "b2 01 e8 ? ? ? ? ? 8d ",
                      14, rage__formatf6eb9, AppendBuildVersion);
    }
} _display;

} // namespace Rainbomizer
