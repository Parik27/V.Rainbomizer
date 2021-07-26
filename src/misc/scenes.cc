#include "scrThread.hh"
#include <cstdint>
#include <mission/missions_YscUtils.hh>

#include <common/ysc.hh>
#include <common/config.hh>

class SwitchSceneRandomizer
{
    static bool
    InitRandomSwitchScenes (YscUtilsOps &utils)
    {
        if (!utils.IsAnyOf ("player_controller_b"_joaat))
            return false;

        static constexpr uint8_t SHELL_CODE[]
            = {// PUSH_CONST_0
               uint8_t (YscOpCode::PUSH_CONST_0),

               // PUSH_CONST_S16 314 (total number of thingies)
               uint8_t (YscOpCode::PUSH_CONST_S16), 0x3a, 0x1,

               // NATIVE (GET_RANDOM_INT_IN_RANGE)
               uint8_t (YscOpCode::NATIVE), 0x2, 0x0, 0x0,

               // LOCAL_U8_LOAD 0x4
               uint8_t (YscOpCode::LOCAL_U8_LOAD), 0x1,

               // STORE
               uint8_t (YscOpCode::STORE),

               // PUSH_CONST_FM1
               uint8_t (YscOpCode::PUSH_CONST_FM1),

               // LOCAL_U8_LOAD 0x8
               uint8_t (YscOpCode::LOCAL_U8_LOAD), 0x2,

               // STORE
               uint8_t (YscOpCode::STORE),

               // PUSH_CONST_1
               uint8_t (YscOpCode::PUSH_CONST_1),

               // LEAVE 0x6, 0x1
               uint8_t (YscOpCode::LEAVE), 0x6, 0x1};

        constexpr uint32_t NATIVE_OPCODE_OFFSET = 4;
        constexpr uint32_t NATIVE_OPCODE_SIZE   = 4;

        static_assert (SHELL_CODE[NATIVE_OPCODE_OFFSET] == YscOpCode::NATIVE,
                       "Native Opcode Offset Incorrect");

        // // Generate dynamic shellcode
        uint8_t shellCode[sizeof (SHELL_CODE)];
        memcpy (shellCode, SHELL_CODE, sizeof (SHELL_CODE));

        utils.Init ("2d 02 ? ? ? 38 00 37 ? 65 ? 6e 38 01 2c ? ? ? 62 ?");

        auto *nativeCode = utils.Get<uint8_t> (14);
        memcpy (&shellCode[NATIVE_OPCODE_OFFSET], nativeCode,
                NATIVE_OPCODE_SIZE);

        utils.Init ("2d 06 ? ? ? 43 3e 01 38 01 30 76 38 02 30 6e");
        utils.WriteBytes (5, shellCode);

        return true;
    }

public:
    SwitchSceneRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("SwitchSceneRandomizer"))
            return;

        YscCodeEdits::Add ("Switch Scene Randomizer", InitRandomSwitchScenes);
    }
} scene;
