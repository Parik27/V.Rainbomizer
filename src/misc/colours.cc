#include <CHud.hh>
#include <Utils.hh>

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
    ColoursRandomizer ()
    {
        return;
        //FILE* logging = fopen("log.txt", "w");
        InitialiseAllComponents ();
        puts("Stuff");

        RegisterHook (
            "8b ? ? ? ? ? 8b ? ? ? ? ? 8b cb 89 44 ? ? e8", 18,
            CHud__SetHudColour, SetNewHudColour);

        printf ("%x",
                hook::get_pattern (
                    "? 53 ? 83 ec 30 ? 8b 15 ? ? ? ? ? 8b d9 ? 63 ? ? ? ? ? "));

        //89 ?? ?? 89 ?? ?? e8 ?? ?? ?? ?? 83 f8 03 
    }
} _cols;
