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

Enabled	  = true

#######################################################
# Traffic Randomizer - Randomizes cars that spawn in traffic

[TrafficRandomizer]

Enabled = true

#######################################################
# Colour Randomizer - Randomizes the colours of various in-game elements, including cars, the HUD, and the markers. 
[ColourRandomizer]

Enabled = true

#######################################################
# Weapon Randomizer - Randomizes the weapons that are dropped/wielded by the player/enemies.
[WeaponRandomizer]

Enabled = true

#######################################################
# Cutscene Randomizer - Randomizes the models used in motion-captured cutscenes
[CutsceneRandomizer]

Enabled = true

#######################################################
# Script Vehicle Randomizer - Randomizes the vehicles you are given in missions.
[ScriptVehicleRandomizer]

Enabled = true

LogSpawnedVehicles = false # Logs all the spawned script vehicles

)";
