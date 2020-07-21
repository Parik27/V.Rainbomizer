#include "scrThread.hh"
#include "Utils.hh"

/*******************************************************/
void
scrThread::InitialisePatterns ()
{
    sm_pActiveThread = GetRelativeReference<scrThread *> (
        "? 89 2d ? ? ? ? e8 ? ? ? ? ? 8b 8d b0 00 00 00 ? 8d 4d 08",
        3, 7);
}

scrThread **scrThread::sm_pActiveThread = nullptr;
