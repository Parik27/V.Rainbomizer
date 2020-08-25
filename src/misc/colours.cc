#include <CHud.hh>
#include <Utils.hh>
#include "common/config.hh"

void (*CHud__SetHudColour) (int, int, int, int, int);

class ColoursRandomizer
{
    /*******************************************************/
    static void SetNewHudColour(int index, int r, int g, int b, int a)
    {
        CHud__SetHudColour (index, RandomInt (255), RandomInt (255),
                            RandomInt (255), a);
    }
    
public:
    /*******************************************************/
    ColoursRandomizer ()
    {
        if (!ConfigManager::ReadConfig("ColourRandomizer"))
            return;
        
        InitialiseAllComponents ();

        RegisterHook (
            "8b ? ? ? ? ? 8b ? ? ? ? ? 8b cb 89 44 ? ? e8", 18,
            CHud__SetHudColour, SetNewHudColour); 
    }
} _cols;
