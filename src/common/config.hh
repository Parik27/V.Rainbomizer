#pragma once

#include <string>
#include <memory>
#include <vector>

namespace cpptoml {
class table;
} // namespace cpptoml

/*******************************************************/
struct BaseConfig
{
    bool enabled = true;

    void Read (std::shared_ptr<cpptoml::table> table);
};

/*******************************************************/
struct TrafficConfig : public BaseConfig
{
};

/*******************************************************/
struct ColourConfig : public BaseConfig
{
};

/*******************************************************/
struct CheatConfig : public BaseConfig
{
};

/*******************************************************/
struct WeaponConfig : public BaseConfig
{
};

/*******************************************************/
struct ParkedCarConfig : public BaseConfig
{
};

/*******************************************************/
struct SoundsConfig : public BaseConfig
{
};

/*******************************************************/
struct ScriptVehicleConfig : public BaseConfig
{
};

/*******************************************************/
struct MissionConfig : public BaseConfig
{
};

/*******************************************************/
struct CutsceneConfig : public BaseConfig
{
};

/******************************************************/
struct WeaponStatsConfig : public BaseConfig
{
};

struct ObjectsConfig : public BaseConfig
{
};

/*******************************************************/
struct Configs
{
    TrafficConfig       traffic;
    ColourConfig        colours;
    CheatConfig         cheat;
    WeaponConfig        weapon;
    ParkedCarConfig     parkedCar;
    SoundsConfig        sounds;
    ScriptVehicleConfig scriptVehicle;
    MissionConfig       missions;
    CutsceneConfig      cutscenes;
    WeaponStatsConfig   weaponStats;
    ObjectsConfig       objects;
};

/*******************************************************/
class ConfigManager
{
    static ConfigManager *mInstance;

    Configs mConfigs;

    ConfigManager (){};
    static void DestroyInstance ();

    void WriteDefaultConfig (const std::string &file);
    std::shared_ptr<cpptoml::table> ParseDefaultConfig ();

public:
    /// Returns the static instance for ConfigManager.
    static ConfigManager *GetInstance ();

    /// Initialises
    ConfigManager (const std::string &file = "config.toml");

    /// Gets the config
    static const Configs &GetConfigs ();
};
