/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DEF_THRONE_OF_THUNDER_H
#define DEF_THRONE_OF_THUNDER_H

#define TOTScriptName "instance_throne_of_thunder"
#define DataHeader "TOT"

enum DataTypes
{
    // Encounter States/Boss GUIDs
    DATA_JINROKH_THE_BREAKER,
    DATA_HORRIDON,
    DATA_COUNCIL_OF_ELDERS,
    DATA_TORTOS,
    DATA_MEGAERA,
    DATA_JIKUN,
    DATA_DURUMU_THE_FORGOTTEN,
    DATA_PRIMORDIUS,
    DATA_DARK_ANIMUS,
    DATA_IRON_QON,
    DATA_TWIN_CONSORTS,
    DATA_LEI_SHEN,
    DATA_RADEN
};

enum Creatures
{
    NPC_LEI_SHEN                = 68397,

    NPC_BOUNCING_BOLT_CONDUIT   = 68698,
    NPC_STATIC_SHOCK_CONDUIT    = 68398,
    NPC_DIFFUSION_CHAIN_CONDUIT = 68696,
    NPC_OVERCHARGE_CONDUIT      = 68697,

    NPC_OVERWHELMING_POWER      = 69645,
};

template<class AI>
AI* GetThroneOfThunderAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, TOTScriptName);
}

#endif // DEF_THRONE_OF_THUNDER_H
