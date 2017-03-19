/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#include "ObjectMgr.h"
#include "Player.h"

#ifndef DEF_SIEGE_OF_ORGRIMMAR_H_
#define DEF_SIEGE_OF_ORGRIMMAR_H_

#define DataHeader "SOO"

enum Encounter
{
    BOSS_IMMERSEUS,
    BOSS_FALLEN_PROTECTORS,
    BOSS_NORUSHEN,
    BOSS_SHA_OF_PRIDE,
    BOSS_GALAKRAS,
    BOSS_IRON_JUGGERNAUT,
    BOSS_KORKRON_DARK_SHAMAN,
    BOSS_GENERAL_NAZGRIM,
    BOSS_MALKOROK,
    BOSS_SPOILS_OF_PANDARIA,
    BOSS_THOK_THE_BLOODTHIRSTY,
    BOSS_SIEGECRAFTER_BLACKFUSE,
    BOSS_PARAGONS_OF_THE_KLAXXI,
    BOSS_GARROSH_HELLSCREAM,

    MAX_BOSS_DATA
};

enum InstanceData
{
    DATA_INSTANCE_PROGRESS                      = 1000,
    DATA_GARROSH_THRALL                         = 1001,
    DATA_GARROSH_HELLSCREAM_SANCTUM_BREACH      = 1002,
    DATA_GARROSH_HELLSCREAM_INTRO               = 1003,
    DATA_GARROSH_HELLSCREAM                     = 1004,
    DATA_HEART_OF_YSHAARJ_CREATURE_MAIN_ROOM    = 1005,
    DATA_HEART_OF_YSHAARJ_GAMEOBJECT_MAIN_ROOM  = 1006,
    DATA_HEART_OF_YSHAARJ_STATE                 = 1007,
    DATA_HEART_OF_YSHAARJ_UNIT_ENTER            = 1008,
    DATA_HEART_OF_YSHAARJ_UNIT_LEAVE            = 1009,
    DATA_HEART_OF_YSHAARJ_UNIT_BOOT             = 1010
};

enum InstanceActions
{
    ACTION_GARROSH_INTRO                    = 1001,
};

enum Creatures
{
    NPC_GARROSH_THRALL                      = 73483,
    NPC_GARROSH_HELLSCREAM                  = 71865,
    NPC_HEART_OF_YSHAARJ_MAIN_ROOM          = 72215,

    NPC_SIEGE_ENGINEER                      = 71984,
    NPC_KORKRON_WARBRINGER                  = 71979,
    NPC_FARSEER_WOLF_RIDER                  = 71983,

    NPC_KORKRON_IRON_STAR                   = 71985,
};

enum GameObjects
{
    GO_HEART_OF_YSHAARJ                     = 221995,
    GO_GARROSH_HELLSCREAM_ENTRANCE_1        = 221438,
    GO_GARROSH_HELLSCREAM_ENTRANCE_2        = 221441,
};

enum InstanceSpells
{
    SPELL_HEARTBEAT_SOUND                       = 148574,
    SPELL_TELEPORT_TEMPLE_OF_THE_JADE_SERPENT   = 144881,
    SPELL_TELEPORT_TEMPLE_OF_THE_RED_CRANE      = 144884,
    SPELL_TELEPORT_TERRACE_OF_ENDLESS_SPRING    = 144886,
};

enum SharedWorldSafeLocations
{
    WSL_ENTRANCE                            = 4572,
    WSL_PRE_GOLDEN_LOTUS                    = 4570,
    WSL_PRE_SHA                             = 4571,
    WSL_PRE_GALAKRAS                        = 4576,
    WSL_PRE_DARK_SHAMAN                     = 4577,
    WSL_THE_DRAG                            = 4624,
    WSL_PRE_MALKOROK                        = 4574,
    WSL_THE_UNDERHOLD                       = 4578,
    WSL_PRE_GARROSH                         = 4579
};

enum SoOInstanceProgress
{
    INSTANCE_PROGRESS_NONE,
    INSTANCE_PROGRESS_REACHED_NORUSHEN,
    INSTANCE_PROGRESS_ORGRIMMAR_GATES,
    INSTANCE_PROGRESS_REACHED_GAMON,
    INSTANCE_PROGRESS_REACHED_UNDERHOLD
};

enum SiegeOfOrgrimmarWorldStateIds
{
    WORLD_STATE_SOO_HEART_OF_YSHAARJ_ACTIVE = 8537
};

enum HeartOfYshaarjId
{
    HEART_OF_YSHAARJ_TEMPLE_OF_THE_JADE_SERPENT     = 0,
    HEART_OF_YSHAARJ_TEMPLE_OF_THE_RED_CRANE        = 1,
    HEART_OF_YSHAARJ_TERRACE_OF_THE_ENDLESS_SPRING  = 2,
};

uint32 const HeartOfYshaarjLocations[3] =
{
    HEART_OF_YSHAARJ_TEMPLE_OF_THE_JADE_SERPENT,
    HEART_OF_YSHAARJ_TEMPLE_OF_THE_RED_CRANE,
    HEART_OF_YSHAARJ_TERRACE_OF_THE_ENDLESS_SPRING
};

uint32 const HeartOfYshaarjTeleportSpells[4] =
{
    SPELL_TELEPORT_TEMPLE_OF_THE_JADE_SERPENT,
    SPELL_TELEPORT_TEMPLE_OF_THE_RED_CRANE,
    SPELL_TELEPORT_TERRACE_OF_ENDLESS_SPRING
};

class RangedClassOrSpecCheck
{
public:
    bool operator()(WorldObject* object) const
    {
        Player* player = object->ToPlayer();
        if (!player)
            return true;

        switch (player->getClass())
        {
            case CLASS_WARRIOR:
            case CLASS_ROGUE:
            case CLASS_DEATH_KNIGHT:
            case CLASS_DEMON_HUNTER:
            default:
                return false;
            case CLASS_MAGE:
            case CLASS_WARLOCK:
            case CLASS_PRIEST:
            case CLASS_HUNTER:
                return true;
            case CLASS_PALADIN:
                return player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == TALENT_SPEC_PALADIN_HOLY;
            case CLASS_SHAMAN:
                return player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == TALENT_SPEC_SHAMAN_RESTORATION;
            case CLASS_MONK:
                return player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == TALENT_SPEC_MONK_MISTWEAVER;
            case CLASS_DRUID:
                return player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == TALENT_SPEC_DRUID_RESTORATION || player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == TALENT_SPEC_DRUID_BALANCE;
        }

        return true;
    }
};

#endif //DEF_SIEGE_OF_ORGRIMMAR_H_
