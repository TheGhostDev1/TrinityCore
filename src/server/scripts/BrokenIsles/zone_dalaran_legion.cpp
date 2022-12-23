/*
* This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "Player.h"
#include "Group.h"

enum BlinkOfAnEye
{
    QUEST_BLINK_OF_AN_EYE = 44663,
    MAP_BROKEN_ISLES = 1220,
    KILL_CREDIT_TELEPORT_DALARAN = 114506,
    GOSSIP_OPTION_DALARAN_TELE = 0,
    SCENE_DALARAN_TELEPORT = 1149
};

struct npc_archmage_khadgar_dalaran_legion : public ScriptedAI
{
public:
    npc_archmage_khadgar_dalaran_legion(Creature* creature) : ScriptedAI(creature) { }

        bool GossipHello(Player* player)
        {
            if (me->IsQuestGiver())
                player->PrepareQuestMenu(me->GetGUID());

            if (player->GetQuestStatus(QUEST_BLINK_OF_AN_EYE) == QUEST_STATUS_INCOMPLETE)
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_OPTION_DALARAN_TELE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0);

            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId)
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            if (action == GOSSIP_ACTION_INFO_DEF + 0)
            {
                if (player->GetQuestStatus(QUEST_BLINK_OF_AN_EYE) == QUEST_STATUS_INCOMPLETE)
                {
                    CloseGossipMenuFor(player);
                    player->AddMovieDelayedAction(SCENE_DALARAN_TELEPORT, [player]
                        {
                            player->TeleportTo(MAP_BROKEN_ISLES, -827.82f, 4369.25f, 738.64f, 1.893364f);
                        });
                    player->SendMovieStart(SCENE_DALARAN_TELEPORT);
                    player->KilledMonsterCredit(KILL_CREDIT_TELEPORT_DALARAN);
                    return true;
                }
                return true;
            }
            return true;
        }
};

void AddSC_dalaran_legion()
{
    RegisterCreatureAI(npc_archmage_khadgar_dalaran_legion);
}
