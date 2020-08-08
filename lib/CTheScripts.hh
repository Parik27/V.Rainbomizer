#pragma once

#include "rage.hh"
#include "scrThread.hh"

class CTheScripts
{
public:
    static atArray<scrThread *> *aThreads;
    static void InitialisePatterns ();
};
