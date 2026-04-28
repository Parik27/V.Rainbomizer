#pragma once

#include <string>
#include <memory>
#include <vector>
#include "config_Macros.hh"

namespace cpptoml {
class table;
} // namespace cpptoml

/*******************************************************/
class ConfigManager
{
    std::shared_ptr<cpptoml::table> m_pConfig;
    std::shared_ptr<cpptoml::table> m_pDefaultConfig;

    ConfigManager (){};

    void WriteDefaultConfig (const std::string &file);
    std::shared_ptr<cpptoml::table> ParseDefaultConfig ();

    bool GetIsEnabled (const std::string &name);

    template <typename T>
    void ReadValue (const std::string &tableName, const std::string &key,
                    T &out, bool tmp = false);


public:
    /// Returns the static instance for ConfigManager.
    static ConfigManager *GetInstance ();

    /// Initialises
    ConfigManager (const std::string &file = "config.toml");

    template <typename... Args>
    static bool
    ReadConfig (const std::string &table, Args... params)
    {
        if (!GetInstance ()->GetIsEnabled (table))
            return false;

        (GetInstance ()->ReadValue (table, params.first, *params.second), ...);
        return true;
    }

    static bool ReadGlobalBool (const std::string &name, bool default_value = true);
};
