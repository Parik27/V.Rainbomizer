const char configDefault[] = R"(
########################################################

# Rainbomizer for V
# Copyright (C) 2020 - Parik

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

TrafficRandomizer = true
ColourRandomizer = true
WeaponRandomizer = true
CutSceneRandomizer = true
ScriptVehicleRandomizer = true
VoiceLineRandomizer = true
WeatherRandomizer = true
MissionRandomizer = true

#######################################################
[ScriptVehicleRandomizer]

LogSpawnedVehicles = false # Logs all the spawned script vehicles

#######################################################
[WeatherRandomizer]

RandomizeWeather = false # Randomize the weather (properties like sun, rain, etc.)
RandomizeTimecycle = true # Randomize the appearance of the sky/ground.

# CrazyMode = false # Not recommended, makes the game invisible.

#######################################################
[VoiceLineRandomizer]

IncludeDLCLines = true # Includes DLC lines in randomization

#######################################################
[MissionRandomizer]

Seed = ""
ForceSeedOnSaves = false

ForcedMission = "" # All missions will start this instead of a random mission.

)";