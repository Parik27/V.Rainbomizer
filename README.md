![Build](https://github.com/Parik27/V.Rainbomizer/workflows/Build/badge.svg)
[![Discord](https://img.shields.io/discord/681996979974570066.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/BYVBQw7)

# Rainbomizer: V 🌈

<a href="https://media.discordapp.net/attachments/821138024192278640/906978373333581864/20211107183741_1.jpg"> <img src="https://media.discordapp.net/attachments/821138024192278640/906978373333581864/20211107183741_1.jpg"></a>

A modification for Grand Theft Auto V that randomizes various aspects of the game from vehicles and sounds to missions and weapons.
<hr>
<p align="center">
  <a href="http://rainbomizer.com/"><strong>homepage</strong></a> • 
  <a href="https://github.com/Parik27/V.Rainbomizer/releases"><strong>download</strong></a> • 
  <a href="https://github.com/Parik27/V.Rainbomizer/issues"><strong>issues</strong></a> • 
  <a href="#installation"><strong>installation</strong></a> • 
  <a href="#features"><strong>features</strong></a> • 
  <a href="#contribute"><strong>contribute</strong></a> 
</p>
<hr>

<span id="features"></span>
## Features

New Features
- 💡 **(NEW) Light Randomizer** - Randomizes all light colours in the game.
- 🏥 **(NEW) Respawn Randomizer** - Randomizes where you respawn.
- 🎥 **(NEW) Switch Scene Randomizer** - Randomizes the scene played when you switch to a different character.

Features
- :helicopter: **Traffic Randomizer** - Randomizes cars that spawn in traffic. 
- :blue_car: **Mission Vehicle Randomizer** - Randomizes vehicles you are given in missions, making sure that the vehicle you're given is usable for that mission.
- :rainbow: **Colour Randomizer** - Randomizes colours of various in-game elements, including cars, the HUD, and more. 
- :page_with_curl: **Mission Randomizer** - Randomizes order of missions in-game. Upon completion of a mission, the game progresses as if you completed the original mission.
- :gun: **Weapon Randomizer** - Randomizes weapons given to the enemies.
- :sound: **Sounds Randomizer** - Randomizes dialogue and sound effects played in-game.
- :movie_camera: **Cutscene Randomizer** - Randomizes models used in motion-captured cutscenes.
- :walking: **Ped Randomizer** - Randomizes all ambient, mission, and cutscene peds.
- :video_game: **Player Randomizer** - Randomizes the player model into a different ped model.
- :star2: **Player Special Ability Randomizer** - With Player Randomizer enabled, this feature randomizes the player's special ability.
- :cloud_with_rain: **Timecycle and Weather Randomizer** - Randomizes the look of the game by randomizing weather elements and sky colour.
- :tractor: **Handling Randomizer** - Randomizes the handling of all vehicles.
- :rocket: **Weapon Stats Randomizer** - Randomizes weapon stats such as fire rate, range, etc. of all weapons.
- :oncoming_police_car: **Dispatch Randomizer** - Randomizes all dispatched police cars, ambulances, helicopters, and boats.

<span id="installation"></span>
## Installation

Rainbomizer requires an **ASI-Loader** to function. It works with **all** versions of the game. Additionally, you require **ScriptHook** for certain features.

1. Install **ScriptHook** (includes an ASI Loader) from [here](http://www.dev-c.com/gtav/scripthookv)
2. Download Rainbomizer from [Releases](https://github.com/Parik27/V.Rainbomizer/releases) and **extract the archive** to the root directory of GTA V. 

## Configuration

If you wish to change any of the mod's functionalities, for example to disable a specific randomizer, a configuration file (`config.toml`) has been provided with the mod. The config file is located in the `rainbomizer` folder in the game's root directory.

The default configuration file is in the main repository, `config.toml`, and is automatically created by the mod if it doesn't exist in the rainbomizer folder using pre-determined default values.

It can be opened and modified with a text editing program such as Notepad++. Any aspect of the mod can be modified to your liking using this file - if you make changes while the game is running, the game must be restarted for these to take effect.

- To enable or disable a certain randomizer, set the corresponding value to "true" or "false" from the list of randomizers at the top of the file.   
- To configure a certain randomizer, find its specific section in the file by searching for its name. If there isn't a specific section, the randomizer doesn't offer further customization. More information about how to configure specific randomizers is provided in the default config file.

## Credits 🌈

#### Lead Developers

- [Parik](https://github.com/Parik27) - Creation and implementation of the ASI.

#### Major Contributors

- [Fryterp23](https://www.twitch.tv/fryterp23) - Assisting with creation of external files and extensive testing.
- [123robot](https://www.twitch.tv/123robot) - Creation of external files, testing, and support with debugging.

#### Beta Testers

- [Gibstack](https://www.twitch.tv/gibstack)
- [Hugo_One](https://www.twitch.tv/hugo\_one)
- [mo_xi](https://www.twitch.tv/mo\_xi)
- [SpeedyFolf](https://www.twitch.tv/speedyfolf)

<span id="contribute"></span>
## Contribution

You can contribute to the project by reporting any crashes/issues you encounter in the [Issues](https://github.com/Parik27/V.Rainbomizer/issues) section of the repository.

Pull requests are welcome, but are required to follow the current style used in the project.

You can also keep up with or contribute to development as it happens by [joining the official Rainbomizer Discord!](https://discord.gg/BYVBQw7) You can give #suggestions, ask for #support, or view current development in #v.

The lib folder in the repository's base is intended for reuse in other projects and should not contain any Rainbomizer specific code.
