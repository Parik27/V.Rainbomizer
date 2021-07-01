#include "CStreaming.hh"
#include "Utils.hh"
#include "base.hh"
#include "common/common.hh"
#include "common/logger.hh"
#include "imgui.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <rage.hh>
#include <CTheScripts.hh>
#include "CItemInfo.hh"
#include <array>

using namespace NativeLiterals;

/* Cool Weapon Models:
   hei_prop_carrier_trailer_01
   hei_prop_heist_tug
   ch_p_ch_rope_tie_01a
 */

class WeaponStatsDebugInterface : public DebugInterface
{
    struct Weapon
    {
        std::string Name;
        enum State : int
        {
            STATE_GOOD = 0,
            STATE_NEUTRAL,
            STATE_BAD
        } Type
            = STATE_NEUTRAL;
    };

    enum CurrentState
    {
        IDLE                   = 0,
        REFRESH_START          = 1,
        REFRESH_WEAPON_REMOVED = 2,
        REFRESH_RANDOMIZED     = 3,
        REFRESH_WEAPON_ADDED   = 4
    } m_CurrentState;

    std::vector<Weapon>     m_Weapons;
    int                     m_CurrentSelected    = 0;
    std::array<uint32_t, 2> m_WeaponHashes       = {0, 0};
    uint32_t                m_NewWeaponModelHash = 0;

    void
    LoadFromFile (const char *fileName)
    {
        FILE *file = Rainbomizer::Common::GetRainbomizerDataFile (fileName);
        m_Weapons.clear ();

        if (!file)
            return;

        char buffer[512] = {0};
        while (fgets (buffer, 512, file))
            {
                buffer[strcspn (buffer, "\n")] = 0;

                int  state     = 1;
                char name[256] = {0};

                sscanf (buffer, " %s %d ", name, &state);
                m_Weapons.push_back ({name, Weapon::State (state)});
            }

        fclose (file);
    }

    void
    SaveToFile (const char *fileName)
    {
        FILE *file
            = Rainbomizer::Common::GetRainbomizerDataFile (fileName, "w");
        for (auto i : m_Weapons)
            fprintf (file, "%s %d\n", i.Name.c_str (), i.Type);

        fclose (file);
    }

    void
    RandomizeTheWeapon ()
    {
        for (auto &info : CWeaponInfoManager::sm_Instance->aItemInfos)
            {
                if (DoesElementExist (m_WeaponHashes, info->Name))
                    {

                        Rainbomizer::Logger::LogMessage (
                            "Randomizing to %x (%s)", m_NewWeaponModelHash,
                            m_Weapons[m_CurrentSelected].Name.c_str ());
                        info->Model = m_NewWeaponModelHash;
                    }
            }
    }

    void
    Process (uint64_t *stack, uint64_t *globals, scrProgram *program,
             scrThreadContext *ctx) override
    {
        if (ctx->m_nScriptHash != "main"_joaat)
            return;

        switch (m_CurrentState)
            {

            case IDLE: break;

            case REFRESH_START:
                "REMOVE_WEAPON_FROM_PED"_n("PLAYER_PED_ID"_n(),
                                           m_WeaponHashes[0]);
                m_CurrentState = REFRESH_WEAPON_REMOVED;
                break;

            case REFRESH_WEAPON_REMOVED:
                RandomizeTheWeapon ();
                m_CurrentState = REFRESH_RANDOMIZED;
                break;

            case REFRESH_RANDOMIZED:
                "GIVE_WEAPON_TO_PED"_n("PLAYER_PED_ID"_n(), m_WeaponHashes[0],
                                       100, false, true);
                m_CurrentState = REFRESH_WEAPON_ADDED;
                break;

            case REFRESH_WEAPON_ADDED: m_CurrentState = IDLE; break;
            }
    }

    void
    Draw () override
    {
        static char sModelName[256] = {0};
        static char sAmmoName[256]  = {0};
        static char sFileName[256]  = {0};

        if (ImGui::InputText ("Weapon Name", sModelName, 256))
            m_WeaponHashes[0] = rage::atStringHash (sModelName);
        if (ImGui::InputText ("Ammo Name", sAmmoName, 256))
            m_WeaponHashes[1] = rage::atStringHash (sAmmoName);

        ImGui::InputText ("File", sFileName, 256);
        ImGui::SameLine ();
        if (ImGui::Button ("Load"))
            LoadFromFile (sFileName);
        ImGui::SameLine ();
        if (ImGui::Button ("Save"))
            SaveToFile (sFileName);

        if (m_CurrentSelected < m_Weapons.size ())
            ImGui::Text ("Current Weapon: %s",
                         m_Weapons[m_CurrentSelected].Name.c_str ());

        ImGui::Columns (2);
        for (int i = 0; i < m_Weapons.size (); i++)
            {
                ImGui::PushID (i);
                if (ImGui::Button (m_Weapons[i].Name.c_str ()))
                    {
                        m_CurrentSelected = i;
                        m_NewWeaponModelHash
                            = rage::atStringHash (m_Weapons[i].Name);
                        m_CurrentState = REFRESH_START;
                    }

                ImGui::NextColumn ();
                if (ImGui::Combo ("##state", (int *) &m_Weapons[i].Type,
                                  "Good\0Neutral\0Bad\0"))
                    {
                        SaveToFile (sFileName);
                    }
                ImGui::NextColumn ();
                ImGui::PopID ();
            }
    }

public:
    const char *
    GetName () override
    {
        return "Weapon Stats";
    }
} g_WeaponStatsDebugInterface;
