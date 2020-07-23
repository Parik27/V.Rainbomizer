#include <string>
#define CPPTOML_NO_RTTI

#include "config.hh"
#include "cpptoml/cpptoml.h"
#include <cstdlib>
#include "logger.hh"
#include "common.hh"
#include "configDefault.hh"
#include <sstream>

ConfigManager *ConfigManager::mInstance = nullptr;

#define CONFIG(table, member, key, type)                                       \
    this->member = table->get_as<type> (key).value_or (this->member);

/*******************************************************/
void
BaseConfig::Read (std::shared_ptr<cpptoml::table> table)
{
    if (!table)
        return;

    CONFIG (table, enabled, "Enabled", bool);
}

/*******************************************************/
bool
DoesFileExist (const std::string &file)
{
    FILE *f = Rainbomizer::Common::GetRainbomizerFile (file, "r");
    if (f)
        {
            fclose (f);
            return true;
        }
    return false;
}

/*******************************************************/
void
ConfigManager::WriteDefaultConfig (const std::string &file)
{
    FILE *f = Rainbomizer::Common::GetRainbomizerFile (file.c_str (), "wb");
    if (f)
        {
            fwrite (configDefault, sizeof (configDefault) - 1, 1, f);
            fclose (f);
        }
}

/*******************************************************/
std::shared_ptr<cpptoml::table>
ConfigManager::ParseDefaultConfig ()
{
    // Read the default config file
    auto stream = std::istringstream (configDefault);

    cpptoml::parser p{stream};
    return p.parse ();
}

/*******************************************************/
ConfigManager::ConfigManager (const std::string &file)
{
    std::shared_ptr<cpptoml::table> config;
    config = ParseDefaultConfig ();
    try
        {
            config = cpptoml::parse_file (
                Rainbomizer::Common::GetRainbomizerFileName (file));
        }
    catch (const std::exception &e)
        {
            Rainbomizer::Logger::LogMessage (e.what ());

            if (!DoesFileExist (file))
                WriteDefaultConfig (file);
        }

    mConfigs.traffic.Read (config->get_table ("TrafficRandomizer"));
    mConfigs.colours.Read (config->get_table ("ColourRandomizer"));
    mConfigs.cheat.Read (config->get_table ("CheatRandomizer"));
    mConfigs.weapon.Read (config->get_table ("WeaponRandomizer"));
    mConfigs.parkedCar.Read (config->get_table ("ParkedCarRandomizer"));
    mConfigs.sounds.Read (config->get_table ("SoundsRandomizer"));
    mConfigs.scriptVehicle.Read (config->get_table ("ScriptVehicleRandomizer"));
    mConfigs.missions.Read (config->get_table ("MissionRandomizer"));
    mConfigs.cutscenes.Read (config->get_table ("CutsceneRandomizer"));
    mConfigs.weaponStats.Read (config->get_table ("WeaponStatsRandomizer"));
    mConfigs.objects.Read (config->get_table ("ObjectRandomizer"));
}

/*******************************************************/
void
ConfigManager::DestroyInstance ()
{
    if (ConfigManager::mInstance)
        delete ConfigManager::mInstance;
}

/*******************************************************/
ConfigManager *
ConfigManager::GetInstance ()
{
    if (!ConfigManager::mInstance)
        {
            ConfigManager::mInstance = new ConfigManager ("config.toml");
            atexit (&ConfigManager::DestroyInstance);
        }
    return ConfigManager::mInstance;
}

/*******************************************************/
const Configs &
ConfigManager::GetConfigs ()
{
    return GetInstance ()->mConfigs;
}
