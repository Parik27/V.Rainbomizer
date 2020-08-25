#include <string>
#define CPPTOML_NO_RTTI

#include "config.hh"
#include "cpptoml/cpptoml.h"
#include <cstdlib>
#include "logger.hh"
#include "common.hh"
#include "configDefault.hh"
#include <sstream>
#include <string>

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
    m_pConfig = ParseDefaultConfig ();
    try
        {
            m_pConfig = cpptoml::parse_file (
                Rainbomizer::Common::GetRainbomizerFileName (file));
        }
    catch (const std::exception &e)
        {
            Rainbomizer::Logger::LogMessage (e.what ());

            if (!DoesFileExist (file))
                WriteDefaultConfig (file);
        }
}

/*******************************************************/
void
ConfigManager::DestroyInstance ()
{
    if (ConfigManager::sm_Instance)
        delete ConfigManager::sm_Instance;
}

/*******************************************************/
ConfigManager *
ConfigManager::GetInstance ()
{
    if (!ConfigManager::sm_Instance)
        {
            ConfigManager::sm_Instance = new ConfigManager ("config.toml");
            atexit (&ConfigManager::DestroyInstance);
        }
    return ConfigManager::sm_Instance;
}

/*******************************************************/
bool
ConfigManager::GetEnabledState (const std::string &name)
{
    // Finds "name" key in the main table. Also allows an "Enabled" key in the
    // table for the randomizer/whatever.

    // Example:
    // TrafficRandomizer = true
    // ColourRandomizer = false
    // [ColourRandomizer]
    // Enabled = true

    // Will be parsed as TrafficRandomizer and ColourRandomizer enabled.
    // Enabled key takes precedence over main table key.

    bool enabled = true;
    ReadValue ("Randomizers", name, enabled);
    ReadValue (name, "Enabled", enabled);

    return enabled;
}

template <typename T>
void
ConfigManager::ReadValue (const std::string &tableName, const std::string &key,
                          T &out)
{
    auto table = m_pConfig->get_table (tableName);
    if (table)
        out = table->get_as<T> (key).value_or (out);
}

#define READ_VALUE_ADD_TYPE(type)                                              \
    template void ConfigManager::ReadValue<type> (                             \
        const std::string &tableName, const std::string &key, type &out);

READ_VALUE_ADD_TYPE (bool)
READ_VALUE_ADD_TYPE (int)
READ_VALUE_ADD_TYPE (std::string)
