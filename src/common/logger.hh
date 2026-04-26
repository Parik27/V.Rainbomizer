#pragma once

#include <cstdio>
#include <source_location>
#include <string>
#include <chrono>

namespace Rainbomizer {

class Logger
{
    static inline FILE *mFile = nullptr;
    static FILE *       GetLogFile ();

public:
    static void LogMessage (const char *format, ...);

    static uint32_t GetGameBuild ();

    class FunctionBenchmark
    {
        std::source_location                                        location;
        std::chrono::time_point<std::chrono::high_resolution_clock> start;

    public:
        FunctionBenchmark (const std::source_location &location
                           = std::source_location::current ())
            : location (location),
              start (std::chrono::high_resolution_clock::now ())
        {
        }

        ~FunctionBenchmark ()
        {
            auto end = std::chrono::high_resolution_clock::now ();
            auto duration
                = std::chrono::duration_cast<std::chrono::microseconds> (
                    end - start);

            LogMessage ("%s:%d %s took %ld microseconds", location.file_name (),
                        location.line (), location.function_name (),
                        duration.count ());
        }
    };
};

} // namespace Rainbomizer
