#include <rage.hh>
#include <Utils.hh>
#include <CStreaming.hh>

#include <common/minhook.hh>
#include <common/logger.hh>
#include <common/config.hh>

#include <common/common.hh>

class CCarGen;

class CarGeneratorRandomizer
{

    RB_C_CONFIG_START
    {
        bool        Enable            = true;
        bool        ForceOnRandomGens = false;
        std::string Vehicle           = "";
    }
    RB_C_CONFIG_END

    /*******************************************************/
    static uint32_t
    GetRandomVehicleModel (uint32_t orig)
    {
        if (orig == 0 && !Config ().ForceOnRandomGens)
            return orig;

        if (Config ().Vehicle.size ())
            return rage::atStringHash (Config ().Vehicle);

        return GetRandomElement (Rainbomizer::Common::GetVehicleHashes ());
    }

    /*******************************************************/
    template <auto &CCarGen__Init>
    static void
    RandomizeCarGenerator (CCarGen *p1, float x, float y, float z, float rotX,
                           float rotY, float rotZ, uint32_t modelHash,
                           int32_t colPrim, int32_t colSec, int32_t colTert,
                           int32_t colQuat, int32_t col5, int32_t col6,
                           uint32_t flags, uint32_t p14, uint64_t p15,
                           uint64_t p16, uint64_t p17, uint32_t *vehicleGroup,
                           uint64_t p19, uint64_t p20, uint64_t p21,
                           uint64_t p22)
    {
#ifdef LOG_CREATED_GENERATORS
        Rainbomizer::Logger::LogMessage (
            "CCarGen(%p %f %f %f %f %f %f %x %x "
            "%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x)",
            p1, x, y, z, rotX, rotY, rotZ, modelHash, colPrim, colSec, colTert,
            colQuat, col5, col6, flags, p14, p15, p16, p17, *vehicleGroup, p19,
            p20, p21, p22);
#endif

        if (Config().Enable)
            {
                *vehicleGroup = 0;
        
                // Remove flags that make it spawn ambulance/firetruck/police car
                flags &= (~(0b11001100));

                // 0 to get the engine to randomize them (still need a way to
                // control the randomness).
                
                // If you set the model hash directly here, the randomness
                // doesn't work as intended. (I'm guessing some sort of conflict
                // or bounds checks by the game)
                modelHash = 0;
            }

        return CCarGen__Init (p1, x, y, z, rotX, rotY, rotZ, modelHash, colPrim,
                              colSec, colTert, colQuat, col5, col6, flags, p14, p15, p16,
                              p17, vehicleGroup, p19, p20, p21, p22);
    }

public:
    /*******************************************************/
    CarGeneratorRandomizer ()
    {
        RB_C_DO_CONFIG ("CarGeneratorRandomizer", Enable, Vehicle, ForceOnRandomGens);

        REGISTER_HOOK ("f3 0f 11 7c ? ? f3 0f 11 74 ? ? e8 ? ? ? ? ? 01 25 ? ? "
                       "? ? ? 84 f6 74 ?",
                       12, RandomizeCarGenerator, void, CCarGen *, float, float,
                       float, float, float, float, uint32_t, int32_t, int32_t,
                       int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t, uint64_t, uint64_t, uint64_t,
                       uint32_t *, uint64_t, uint64_t, uint64_t, uint64_t);
    }
} g_CarGeneratorRandomizer;
