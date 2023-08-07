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

#include "Conversation.h"
#include "MotionMaster.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

enum MardumSpellData
{
    SPELL_START_DEMON_HUNTER_PLAY_SCENE = 193525
};

enum MardumQuestData
{
    QUEST_DEMON_HUNTER_INTRO_TRACKER    = 40076
};

enum MardumConversationData
{
    CONVO_DEMONHUNTER_INTRO_START   = 705
};

class scene_demonhunter_intro : public SceneScript
{
public:
    scene_demonhunter_intro() : SceneScript("scene_demonhunter_intro") { }

    void OnSceneStart(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        Conversation::CreateConversation(CONVO_DEMONHUNTER_INTRO_START, player, *player, player->GetGUID(), nullptr);
    }

    void OnSceneComplete(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        PhasingHandler::OnConditionChange(player);
    }
};

// 196030 - Serverside Spell
class spell_demon_hunter_intro_aura : public AuraScript
{
    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(nullptr, SPELL_START_DEMON_HUNTER_PLAY_SCENE, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_demon_hunter_intro_aura::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

enum TheInvasionBeginsQuestData
{
    QUEST_THE_INVASION_BEGINS       = 40077,

    CONVO_THE_INVASION_BEGINS       = 922,

    CONVO_LINE_TRIGGER_FACING       = 2529,
    CONVO_LINE_START_PATH           = 2288,

    CONVO_ACROR_IDX_KAYN            = 1,
    CONVO_ACTOR_IDX_KORVAS          = 2,

    NPC_KAYN_SUNFURY                = 93011,
    NPC_JAYCE_DARKWEAVER            = 98228,
    NPC_ALLARI_THE_SOULEATER        = 98227,
    NPC_CYANA_NIGHTGLAIVE           = 98290,
    NPC_KORVAS_BLOODTHORN           = 98292,
    NPC_SEVIS_BRIGHTFLAME           = 99918,

    SPELL_THE_INVASION_BEGINS       = 187379,
    SPELL_TRACK_TARGET_IN_CHANNEL   = 175799,
    SPELL_DEMON_HUNTER_GLIDE_STATE  = 199303
};

enum TheInvasionsBeginsWaypointData
{
    PATH_KAYN_INVASIONS_BEGINS      = 7858240,
    PATH_JAYCE_INVANSIONS_BEGINS    = 7440880,
    PATH_ALLARI_INVASIONS_BEGINS    = 7858160,
    PATH_CYANA_INVASIONS_BEGINS     = 7863360,
    PATH_KORVAS_INVASIONS_BEGINS    = 7993440,
    PATH_SEVIS_INVASIONS_BEGINS     = 7863200
};

enum TheInvasionBeginsEventData
{
    EVENT_ILLIDARI_FACE_PLAYERS     = 1,
    EVENT_ILLIDARI_START_PATH
};

// 93011 - Kayn Sunfury
struct npc_kayn_sunfury_invasion_begins : public ScriptedAI
{
    npc_kayn_sunfury_invasion_begins(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_THE_INVASION_BEGINS)
        {
            PhasingHandler::OnConditionChange(player);
            //player->CastSpell(nullptr, SPELL_THE_INVASION_BEGINS, false);
            Conversation::CreateConversation(CONVO_THE_INVASION_BEGINS, player, *player, player->GetGUID(), nullptr, false);
        }
    }
};

// 922 - The Invasion Begins
class conversation_the_invasion_begins : public ConversationScript
{
public:
    conversation_the_invasion_begins() : ConversationScript("conversation_the_invasion_begins") { }

    void OnConversationCreate(Conversation* conversation, Unit* creator) override
    {
        Creature* kaynObject = GetClosestCreatureWithOptions(creator, 10.0f, { .CreatureId = NPC_KAYN_SUNFURY, .IgnorePhases = true });
        Creature* jayceObject = GetClosestCreatureWithOptions(creator, 10.0f, { .CreatureId = NPC_JAYCE_DARKWEAVER, .IgnorePhases = true });
        Creature* allariaObject = GetClosestCreatureWithOptions(creator, 10.0f, { .CreatureId = NPC_ALLARI_THE_SOULEATER, .IgnorePhases = true });
        Creature* cyanaObject = GetClosestCreatureWithOptions(creator, 10.0f, { .CreatureId = NPC_CYANA_NIGHTGLAIVE, .IgnorePhases = true });
        Creature* korvasObject = GetClosestCreatureWithOptions(creator, 10.0f, { .CreatureId = NPC_KORVAS_BLOODTHORN, .IgnorePhases = true });
        Creature* sevisObject = GetClosestCreatureWithOptions(creator, 10.0f, { .CreatureId = NPC_SEVIS_BRIGHTFLAME, .IgnorePhases = true });
        if (!kaynObject || !jayceObject || !allariaObject || !cyanaObject || !korvasObject || !sevisObject)
            return;

        TempSummon* kaynClone = kaynObject->SummonPersonalClone(kaynObject->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, creator->ToPlayer());
        TempSummon* jayceClone = jayceObject->SummonPersonalClone(jayceObject->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, creator->ToPlayer());
        TempSummon* allariaClone = allariaObject->SummonPersonalClone(allariaObject->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, creator->ToPlayer());
        TempSummon* cyanaClone = cyanaObject->SummonPersonalClone(cyanaObject->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, creator->ToPlayer());
        TempSummon* korvasClone = korvasObject->SummonPersonalClone(korvasObject->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, creator->ToPlayer());
        TempSummon* sevisClone = sevisObject->SummonPersonalClone(sevisObject->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, creator->ToPlayer());
        if (!kaynClone || !jayceClone || !allariaClone || !cyanaClone || !korvasClone || !sevisClone)
            return;

        _jayceGUID = jayceClone->GetGUID();
        _allariGUID = allariaClone->GetGUID();
        _cyanaGUID = cyanaClone->GetGUID();
        _sevisGUID = sevisClone->GetGUID();
        kaynClone->RemoveNpcFlag(NPCFlags(UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));

        conversation->AddActor(CONVO_THE_INVASION_BEGINS, CONVO_ACROR_IDX_KAYN, kaynClone->GetGUID());
        conversation->AddActor(CONVO_THE_INVASION_BEGINS, CONVO_ACTOR_IDX_KORVAS, korvasClone->GetGUID());
        conversation->Start();
    }

    void OnConversationStart(Conversation* conversation) override
    {
        LocaleConstant privateOwnerLocale = conversation->GetPrivateObjectOwnerLocale();

        if (Milliseconds const* illidariFacingLineStarted = conversation->GetLineStartTime(privateOwnerLocale, CONVO_LINE_TRIGGER_FACING))
            _events.ScheduleEvent(EVENT_ILLIDARI_FACE_PLAYERS, *illidariFacingLineStarted);

        if (Milliseconds const* illidariStartPathLineStarted = conversation->GetLineStartTime(privateOwnerLocale, CONVO_LINE_START_PATH))
            _events.ScheduleEvent(EVENT_ILLIDARI_START_PATH, *illidariStartPathLineStarted);
    }

    void OnConversationUpdate(Conversation* conversation, uint32 diff) override
    {
        _events.Update(diff);

        switch (_events.ExecuteEvent())
        {
        case EVENT_ILLIDARI_FACE_PLAYERS:
        {
            Unit* privateObjectOwner = ObjectAccessor::GetUnit(*conversation, conversation->GetPrivateObjectOwner());
            if (!privateObjectOwner)
                break;

            Creature* kaynClone = conversation->GetActorCreature(CONVO_ACROR_IDX_KAYN);
            if (!kaynClone)
                break;

            Creature* korvasClone = conversation->GetActorCreature(CONVO_ACTOR_IDX_KORVAS);
            if (!korvasClone)
                break;

            Creature* jayceClone = ObjectAccessor::GetCreature(*conversation, _jayceGUID);
            if (!jayceClone)
                break;

            Creature* allariClone = ObjectAccessor::GetCreature(*conversation, _allariGUID);
            if (!allariClone)
                break;

            Creature* cyanaClone = ObjectAccessor::GetCreature(*conversation, _cyanaGUID);
            if (!cyanaClone)
                break;

            Creature* sevisClone = ObjectAccessor::GetCreature(*conversation, _sevisGUID);
            if (!sevisClone)
                break;

            kaynClone->CastSpell(privateObjectOwner, SPELL_TRACK_TARGET_IN_CHANNEL, false);
            korvasClone->CastSpell(privateObjectOwner, SPELL_TRACK_TARGET_IN_CHANNEL, false);
            jayceClone->CastSpell(privateObjectOwner, SPELL_TRACK_TARGET_IN_CHANNEL, false);
            allariClone->CastSpell(privateObjectOwner, SPELL_TRACK_TARGET_IN_CHANNEL, false);
            cyanaClone->CastSpell(privateObjectOwner, SPELL_TRACK_TARGET_IN_CHANNEL, false);
            sevisClone->CastSpell(privateObjectOwner, SPELL_TRACK_TARGET_IN_CHANNEL, false);
            break;
        }
        case EVENT_ILLIDARI_START_PATH:
        {
            Creature* kaynClone = conversation->GetActorCreature(CONVO_ACROR_IDX_KAYN);
            if (!kaynClone)
                break;

            Creature* korvasClone = conversation->GetActorCreature(CONVO_ACTOR_IDX_KORVAS);
            if (!korvasClone)
                break;

            Creature* jayceClone = ObjectAccessor::GetCreature(*conversation, _jayceGUID);
            if (!jayceClone)
                break;

            Creature* allariClone = ObjectAccessor::GetCreature(*conversation, _allariGUID);
            if (!allariClone)
                break;

            Creature* cyanaClone = ObjectAccessor::GetCreature(*conversation, _cyanaGUID);
            if (!cyanaClone)
                break;

            Creature* sevisClone = ObjectAccessor::GetCreature(*conversation, _sevisGUID);
            if (!sevisClone)
                break;

            kaynClone->GetMotionMaster()->MovePath(PATH_KAYN_INVASIONS_BEGINS, false);
            korvasClone->GetMotionMaster()->MovePath(PATH_KORVAS_INVASIONS_BEGINS, false);
            jayceClone->GetMotionMaster()->MovePath(PATH_JAYCE_INVANSIONS_BEGINS, false);
            allariClone->GetMotionMaster()->MovePath(PATH_ALLARI_INVASIONS_BEGINS, false);
            cyanaClone->GetMotionMaster()->MovePath(PATH_CYANA_INVASIONS_BEGINS, false);
            sevisClone->GetMotionMaster()->MovePath(PATH_SEVIS_INVASIONS_BEGINS, false);
            break;
        }
        default:
            break;
        }
    }

private:
    ObjectGuid _jayceGUID;
    ObjectGuid _allariGUID;
    ObjectGuid _cyanaGUID;
    ObjectGuid _sevisGUID;
    EventMap _events;
};

void AddSC_zone_mardum()
{
    // Creature
    RegisterCreatureAI(npc_kayn_sunfury_invasion_begins);

    // Conversation
    new conversation_the_invasion_begins();

    // Scene
    new scene_demonhunter_intro();

    // Spells
    RegisterSpellScript(spell_demon_hunter_intro_aura);
};
