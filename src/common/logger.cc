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

constexpr int RAINBOMIZER_BUILD_NUMBER =
#include "../../build-count.txt"
    ;

char *(*rage__formatf6eb9) (char *, char const *, ...);

#ifdef ENABLE_DEBUG_MENU

#include "debug/logger.hh"

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

/*******************************************************/
class DisplayBuildVersion
{
    static char *
    AppendBuildVersion (char *out, char const *format, char *build,
                        char *version)
    {
        return rage__formatf6eb9 (out, "Rainbomizer Build %d   Build %s",
                                  RAINBOMIZER_BUILD_NUMBER + 1, version);
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
