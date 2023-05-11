/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

/*
Blasted_Lands
Quest support: 3628.
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Group.h"

enum DeathlyUsher
{
    SPELL_TELEPORT_SINGLE               = 12885,
    SPELL_TELEPORT_SINGLE_IN_GROUP      = 13142,
    SPELL_TELEPORT_GROUP                = 27686
};

/*#####
# spell_razelikh_teleport_group
#####*/

class spell_razelikh_teleport_group : public SpellScriptLoader
{
    public: spell_razelikh_teleport_group() : SpellScriptLoader("spell_razelikh_teleport_group") { }

        class spell_razelikh_teleport_group_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_razelikh_teleport_group_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                return ValidateSpellInfo({ SPELL_TELEPORT_SINGLE, SPELL_TELEPORT_SINGLE_IN_GROUP });
            }

            void HandleScriptEffect(SpellEffIndex /* effIndex */)
            {
                if (Player* player = GetHitPlayer())
                {
                    if (Group* group = player->GetGroup())
                    {
                        for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
                            if (Player* member = itr->GetSource())
                                if (member->IsWithinDistInMap(player, 20.0f) && !member->isDead())
                                    member->CastSpell(member, SPELL_TELEPORT_SINGLE_IN_GROUP, true);
                    }
                    else
                        player->CastSpell(player, SPELL_TELEPORT_SINGLE, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_razelikh_teleport_group_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_razelikh_teleport_group_SpellScript();
        }
};

enum DraenorIntro
{
    QUEST_THE_DARK_PORTAL     = 36881,
    QUEST_THE_DARK_PORTAL_2   = 34398,

    MAP_TANAAN_JUNGLE         = 1265,
    MAP_BLASTED_LANDS_PHASE   = 1190,
    MAP_EASTERN_KINGDOMS      = 0,

    CREDIT_SPEAK_WITH_KHADGAR = 78419,

    SCENE_ENTER_PORTAL        = 199,

    GOSSIP_MENU_DRAENOR_INTRO = 16863,
    GOSSIP_MENU_OPTION        = 0,

    SPELL_TIME_TRAVELLING     = 176111
};

// Need to update this with non hard coded gossip.
class npc_zidormi_blasted_lands : public CreatureScript
{
public:
    npc_zidormi_blasted_lands() : CreatureScript("npc_zidormi_blasted_lands") { }

    struct npc_zidormi_blasted_landsAI : public ScriptedAI
    {
        npc_zidormi_blasted_landsAI(Creature* creature) : ScriptedAI(creature) { }

        bool GossipHello(Player* player)
        {
            if (player->getLevel() < 90)
                return true;

            if (player->GetMapId() == MAP_BLASTED_LANDS_PHASE)
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I would like to visit the past", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0);
            }
            else if (player->GetMapId() == MAP_EASTERN_KINGDOMS)
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Return to the present", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            }

            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            return true;
        }

        bool GossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId)
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);

            if (action == GOSSIP_ACTION_INFO_DEF + 0)
            {
                CloseGossipMenuFor(player);

                player->CastSpell(player, SPELL_TIME_TRAVELLING, true);
                player->SeamlessTeleportToMap(MAP_EASTERN_KINGDOMS);
            }
            else if (action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                CloseGossipMenuFor(player);

                player->RemoveAurasDueToSpell(SPELL_TIME_TRAVELLING);
                player->SeamlessTeleportToMap(MAP_BLASTED_LANDS_PHASE);
            }

            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_zidormi_blasted_landsAI(creature);
    }
};

struct npc_archmage_khadgar_blastedlands : public ScriptedAI
{
public:
     npc_archmage_khadgar_blastedlands(Creature* creature) : ScriptedAI(creature) { }

         bool OnGossipHello(Player* player)
         {
             if (me->IsQuestGiver())
                 player->PrepareQuestMenu(me->GetGUID());

             if (player->GetQuestStatus(QUEST_THE_DARK_PORTAL) == QUEST_STATUS_INCOMPLETE)
             {
                 AddGossipItemFor(player, GOSSIP_MENU_DRAENOR_INTRO, GOSSIP_MENU_OPTION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0);
                 SendGossipMenuFor(player, GOSSIP_MENU_DRAENOR_INTRO, me->GetGUID());
             }
             else
                 SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

             return true;
         }

         bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId)
         {
             uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
             if (action == GOSSIP_ACTION_INFO_DEF + 0)
             {
                 if (player->GetQuestStatus(QUEST_THE_DARK_PORTAL) == QUEST_STATUS_INCOMPLETE)
                 {
                     CloseGossipMenuFor(player);
                     player->AddMovieDelayedAction(SCENE_ENTER_PORTAL, [player]
                     {
                        player->TeleportTo(MAP_TANAAN_JUNGLE, 4066.7370f, -2381.9917f, 94.858f, 2.90f);
                     });
                     player->SendMovieStart(SCENE_ENTER_PORTAL);
                     player->KilledMonsterCredit(CREDIT_SPEAK_WITH_KHADGAR);
                     return true;
                 }
                 else if (player->GetQuestStatus(QUEST_THE_DARK_PORTAL_2) == QUEST_STATUS_INCOMPLETE)
                 {
                     CloseGossipMenuFor(player);
                     player->AddMovieDelayedAction(SCENE_ENTER_PORTAL, [player]
                     {
                        player->TeleportTo(MAP_TANAAN_JUNGLE, 4066.7370f, -2381.9917f, 94.858f, 2.90f);
                     });
                     player->SendMovieStart(SCENE_ENTER_PORTAL);
                     player->KilledMonsterCredit(CREDIT_SPEAK_WITH_KHADGAR);
                     return true;
                 }
             }
             return true;
         }
 };

void AddSC_blasted_lands()
{
    new spell_razelikh_teleport_group();
    RegisterCreatureAI(npc_archmage_khadgar_blastedlands);
    new npc_zidormi_blasted_lands();
}
