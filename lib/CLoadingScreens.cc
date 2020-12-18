#include "Utils.hh"
#include "CLoadingScreens.hh"

void (*CLoadingScreens_Shutdown) (int);
void
CLoadingScreens::Shutdown (int type)
{
    CLoadingScreens_Shutdown (type);
}

void
CLoadingScreens::InitialisePatterns ()
{
    ConvertCall (hook::get_pattern ("? 57 ? 83 ec 20 8b 05 ? ? ? ? 85 c0 "
                                    "0f 84 ? ? ? ? 83 f8 06 "),
                 CLoadingScreens_Shutdown);
}
