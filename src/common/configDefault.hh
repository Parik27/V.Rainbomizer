const char configDefault[] = R"(
########################################################

# Rainbomizer for V
# Copyright (C) 2020-2021 - Parik

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.	If not, see <https://www.gnu.org/licenses/>.
#
#######################################################

# General Configuration
[Randomizers]

RainbomizerNews = true
RainbomizerCredits = true

TrafficRandomizer = true
DispatchRandomizer = true
ColourRandomizer = true
WeaponRandomizer = true
WeaponStatsRandomizer = true
CutSceneRandomizer = true
ScriptVehicleRandomizer = true
VoiceLineRandomizer = true
SfxRandomizer = true
WeatherRandomizer = true
MissionRandomizer = true
PedRandomizer = true
HandlingRandomizer = false
PlayerSwitchSceneRandomizer = false

#######################################################
[ColourRandomizer]

RandomizeHudColours = true
RandomizeCarColours = true

#######################################################
[ScriptVehicleRandomizer]

LogSpawnedVehicles = false # Logs all the spawned script vehicles

#######################################################
[WeatherRandomizer]

RandomizeWeather = true # Randomize the weather (properties like sun, rain, etc.)
RandomizeTimecycle = true # Randomize the appearance of the sky/ground.

TunableFile = "Timecyc/Default.txt"

RandomizeEveryFade = true

#######################################################
[VoiceLineRandomizer]

IncludeDLCLines = true # Includes DLC lines in randomization

#######################################################
[MissionRandomizer]

Seed = ""
ForceSeedOnSaves = false

ForcedMission = "" # All missions will start this instead of a random mission.

EnableFastSkips = false # Mission skips will be enabled after failing the mission once. Useful for testing

#######################################################
[PedRandomizer]

RandomizePlayer = true
RandomizePeds = true
RandomizeCutscenePeds = true # only if UseCutsceneModelsFile is enabled

RandomizeSpecialAbility = true
IncludeUnusedAbilities = false # sprint boost, aimbot and two abilities that don't work (+ any other R* may decide to add and not use in the future).

ForcedPed = ""
ForcedClipset = ""

EnableNSFWModels = false
UseCutsceneModelsFile = true
)";
