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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ulduar.h"

enum Yells
{
    SAY_AGGRO                                   = 0,
    SAY_SPECIAL_1                               = 1,
    SAY_SPECIAL_2                               = 2,
    SAY_SPECIAL_3                               = 3,
    SAY_JUMPDOWN                                = 4,
    SAY_SLAY                                    = 5,
    SAY_BERSERK                                 = 6,
    SAY_WIPE                                    = 7,
    SAY_DEATH                                   = 8,
    SAY_END_NORMAL_1                            = 9,
    SAY_END_NORMAL_2                            = 10,
    SAY_END_NORMAL_3                            = 11,
    SAY_END_HARD_1                              = 12,
    SAY_END_HARD_2                              = 13,
    SAY_END_HARD_3                              = 14
};

class boss_thorim : public CreatureScript
{
    public:
        boss_thorim() : CreatureScript("boss_thorim") { }

        struct boss_thorimAI : public BossAI
        {
            boss_thorimAI(Creature* creature) : BossAI(creature, BOSS_THORIM)
            {
            }

            void Reset() override
            {
                _Reset();
            }

            void EnterEvadeMode(EvadeReason why) override
            {
                Talk(SAY_WIPE);
                _EnterEvadeMode(why);
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);
                _JustDied();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
                _EnterCombat();
            }

            void UpdateAI(uint32 /*diff*/) override
            {
                if (!UpdateVictim())
                    return;
                //SPELLS @todo

                //
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<boss_thorimAI>(creature);
        }
};

struct npc_thorim_trashAI : public ScriptedAI
{
    npc_thorim_trashAI(Creature* creature) : ScriptedAI(creature)
    {
        _instance = creature->GetInstanceScript();
        for (uint8 i = 0; i < ThorimTrashCount; ++i)
            if (me->GetEntry() == StaticThorimTrashInfo[i].Entry)
                _info = &StaticThorimTrashInfo[i];

        ASSERT(_info);
    }

    struct AIHelper
    {
        /// returns heal amount of the given spell including hots
        static uint32 GetTotalHeal(SpellInfo const* spellInfo, Unit const* caster)
        {
            uint32 heal = 0;
            for (SpellEffectInfo const* effect : spellInfo->GetEffects())
            {
                if (effect->IsEffect(SPELL_EFFECT_HEAL))
                    heal += effect->CalcValue(caster);

                if (effect->IsEffect(SPELL_EFFECT_APPLY_AURA) && effect->IsAura(SPELL_AURA_PERIODIC_HEAL))
                    heal += spellInfo->GetMaxTicks() * effect->CalcValue(caster);
            }
            return heal;
        }

        /// returns remaining heal amount on given target
        static uint32 GetRemainingHealOn(Unit* target)
        {
            uint32 heal = 0;
            Unit::AuraEffectList const& auras = target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL);
            for (AuraEffect const* aurEff : auras)
                heal += aurEff->GetAmount() * aurEff->GetRemainingTicks();

            return heal;
        }

        class MostHPMissingInRange
        {
            public:
                MostHPMissingInRange(Unit const* referer, float range, uint32 hp, uint32 exclAura = 0, bool exclSelf = false)
                    : _referer(referer), _range(range), _hp(hp), _exclAura(exclAura), _exclSelf(exclSelf) { }

                bool operator()(Unit* u)
                {
                    if (_exclSelf && u == _referer)
                        return false;

                    if (_exclAura && u->HasAura(_exclAura))
                        return false;

                    if ((u->GetHealth() + GetRemainingHealOn(u) + _hp) > u->GetMaxHealth())
                        return false;

                    uint32 missingHP = u->GetMaxHealth() - u->GetHealth();
                    if (u->IsAlive() && _referer->IsFriendlyTo(u) && _referer->IsWithinDistInMap(u, _range) && missingHP > _hp)
                    {
                        _hp = missingHP;
                        return true;
                    }

                    return false;
                }

            private:
                Unit const* _referer;
                float _range;
                uint32 _hp;
                uint32 _exclAura;
                bool _exclSelf;
        };

        static Unit* GetUnitWithMostMissingHp(SpellInfo const* spellInfo, Unit* caster)
        {
            // use positive range, it's a healing spell
            float const range = spellInfo->GetMaxRange(true);
            uint32 const heal = GetTotalHeal(spellInfo, caster);

            Unit* target = nullptr;
            Trinity::MostHPMissingInRange checker(caster, range, heal);
            Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(caster, target, checker);
            Cell::VisitGridObjects(caster, searcher, 60.0f);

            return target;
        }

        static Unit* GetHealTarget(SpellInfo const* spellInfo, Unit* caster)
        {
            Unit* healTarget = nullptr;
            if (!spellInfo->HasAttribute(SPELL_ATTR1_CANT_TARGET_SELF) && !roll_chance_f(caster->GetHealthPct()) && ((caster->GetHealth() + GetRemainingHealOn(caster) + GetTotalHeal(spellInfo, caster)) <= caster->GetMaxHealth()))
                healTarget = caster;
            else
                healTarget = GetUnitWithMostMissingHp(spellInfo, caster);

            return healTarget;
        }
    };

    bool UseAbility(uint32 spellId)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, GetDifficulty());
        if (!spellInfo)
            return false;

        Unit* target = nullptr;
        if (AIHelper::GetTotalHeal(spellInfo, me))
            target = AIHelper::GetHealTarget(spellInfo, me);
        else
            target = me->GetVictim();

        if (!target)
            return false;

        if (_info->Type == MERCENARY_SOLDIER)
        {
            bool allowMove = true;
            if (me->IsInRange(target, spellInfo->GetMinRange(), spellInfo->GetMaxRange()))
                allowMove = false;

            if (IsCombatMovementAllowed() != allowMove)
            {
                SetCombatMovement(allowMove);

                // need relaunch movement
                ScriptedAI::AttackStart(target);

                // give some time to allow reposition, try again in a second
                if (allowMove)
                    return false;
            }
        }

        DoCast(target, spellId);
        return true;
    }

    void UpdateAI(uint32 diff) final override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            ExecuteEvent(eventId);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        if (_info->Type == DARK_RUNE_ACOLYTE)
            DoSpellAttackIfReady(SPELL_HOLY_SMITE);
        else
            DoMeleeAttackIfReady();
    }

    virtual void ExecuteEvent(uint32 eventId) = 0;

protected:
    InstanceScript* _instance;
    EventMap _events;

    ThorimTrashInfo const* _info = nullptr;
};

class npc_thorim_pre_phase : public CreatureScript
{
    public:
        npc_thorim_pre_phase() : CreatureScript("npc_thorim_pre_phase") { }

        struct npc_thorim_pre_phaseAI : public npc_thorim_trashAI
        {
            npc_thorim_pre_phaseAI(Creature* creature) : npc_thorim_trashAI(creature)
            {
                me->setActive(true); // prevent grid unload
            }

            void Reset() override
            {
                _events.Reset();
                if (_info->PrimaryAbility)
                    _events.ScheduleEvent(EVENT_PRIMARY_ABILITY, urand(3000, 6000));
                if (_info->SecondaryAbility)
                    _events.ScheduleEvent(EVENT_SECONDARY_ABILITY, _info->SecondaryAbility == SPELL_SHOOT ? 2000 : urand(12000, 15000));
                if (_info->ThirdAbility)
                    _events.ScheduleEvent(EVENT_THIRD_ABILITY, urand(6000, 8000));
                if (_info->Type == MERCENARY_SOLDIER)
                    SetCombatMovement(false);
            }

            void JustDied(Unit* /*victim*/) override
            {
                if (Creature* thorim = _instance->GetCreature(BOSS_THORIM))
                    thorim->AI()->DoAction(ACTION_INCREASE_PREADDS_COUNT);
            }

            bool ShouldSparWith(Unit const* target) const override
            {
                return !target->GetAffectingPlayer();
            }

            void DamageTaken(Unit* attacker, uint32& damage) override
            {
                // nullify spell damage
                if (!attacker->GetAffectingPlayer())
                    damage = 0;
            }

            void ExecuteEvent(uint32 eventId) override
            {
                switch (eventId)
                {
                    case EVENT_PRIMARY_ABILITY:
                        if (UseAbility(_info->PrimaryAbility))
                            _events.ScheduleEvent(eventId, urand(15000, 20000));
                        else
                            _events.ScheduleEvent(eventId, 1000);
                        break;
                    case EVENT_SECONDARY_ABILITY:
                        if (UseAbility(_info->SecondaryAbility))
                            _events.ScheduleEvent(eventId, _info->SecondaryAbility == SPELL_SHOOT ? 2000 : urand(4000, 8000));
                        else
                            _events.ScheduleEvent(eventId, 1000);
                        break;
                    case EVENT_THIRD_ABILITY:
                        if (UseAbility(_info->ThirdAbility))
                            _events.ScheduleEvent(eventId, urand(6000, 8000));
                        else
                            _events.ScheduleEvent(eventId, 1000);
                        break;
                    default:
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_thorim_pre_phaseAI>(creature);
        }
};

class npc_thorim_arena_phase : public CreatureScript
{
    public:
        npc_thorim_arena_phase() : CreatureScript("npc_thorim_arena_phase") { }

        struct npc_thorim_arena_phaseAI : public npc_thorim_trashAI
        {
            npc_thorim_arena_phaseAI(Creature* creature) : npc_thorim_trashAI(creature)
            {
                switch (_info->Type)
                {
                    case DARK_RUNE_CHAMPION:
                    case DARK_RUNE_WARBRINGER:
                    case DARK_RUNE_COMMONER:
                    case DARK_RUNE_EVOKER:
                        _isInArena = true;
                        break;
                    case DARK_RUNE_ACOLYTE:
                    {
                        _isInArena = (_info->Entry == NPC_DARK_RUNE_ACOLYTE_PRE);
                        SetBoundary(&ArenaBoundaries, !_isInArena);
                        break;
                    }
                    default:
                        _isInArena = false;
                        break;
                }
            }

            bool CanAIAttack(Unit const* who) const override
            {
                // don't try to attack players in balcony
                if (_isInArena && HeightPositionCheck(true)(who))
                    return false;

                return CheckBoundary(who);
            }

            void Reset() override
            {
                _events.Reset();
                if (_info->PrimaryAbility)
                    _events.ScheduleEvent(EVENT_PRIMARY_ABILITY, urand(3000, 6000));
                if (_info->SecondaryAbility)
                    _events.ScheduleEvent(EVENT_SECONDARY_ABILITY, urand(7000, 9000));
                if (_info->ThirdAbility)
                    _events.ScheduleEvent(EVENT_THIRD_ABILITY, urand(6000, 8000));
                if (_info->Type == DARK_RUNE_CHAMPION)
                    _events.ScheduleEvent(EVENT_ABILITY_CHARGE, 8000);
            }

            void EnterCombat(Unit* /*who*/) override
            {
                if (_info->Type == DARK_RUNE_WARBRINGER)
                    DoCast(me, SPELL_AURA_OF_CELERITY);

                if (!_isInArena)
                    if (Creature* colossus = _instance->GetCreature(DATA_RUNIC_COLOSSUS))
                        colossus->AI()->DoAction(ACTION_ACTIVATE_RUNIC_SMASH);
            }

            void EnterEvadeMode(EvadeReason why) override
            {
                if (why != EVADE_REASON_NO_HOSTILES && why != EVADE_REASON_BOUNDARY)
                    return;

                // this should only happen if theres no alive player in the arena -> summon orb
                if (Creature* thorim = _instance->GetCreature(BOSS_THORIM))
                    thorim->AI()->DoAction(ACTION_BERSERK);
                ScriptedAI::EnterEvadeMode(why);
            }

            void ExecuteEvent(uint32 eventId) override
            {
                switch (eventId)
                {
                    case EVENT_PRIMARY_ABILITY:
                        if (UseAbility(_info->PrimaryAbility))
                            _events.Repeat(3000, 6000);
                        else
                            _events.Repeat(1000);
                        break;
                    case EVENT_SECONDARY_ABILITY:
                        if (UseAbility(_info->SecondaryAbility))
                            _events.Repeat(12000, 16000);
                        else
                            _events.Repeat(1000);
                        break;
                    case EVENT_THIRD_ABILITY:
                        if (UseAbility(_info->ThirdAbility))
                            _events.Repeat(6000, 8000);
                        else
                            _events.Repeat(1000);
                        break;
                    case EVENT_ABILITY_CHARGE:
                    {
                        Unit* referer = me;
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [referer](Unit* unit){ return unit->GetTypeId() == TYPEID_PLAYER && unit->IsInRange(referer, 8.0f, 25.0f); }))
                            DoCast(target, SPELL_CHARGE);
                        _events.ScheduleEvent(eventId, 12000);
                        break;
                    }
                    default:
                        break;
                }
            }

        private:
            bool _isInArena;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_thorim_arena_phaseAI>(creature);
        }
};

struct npc_thorim_minibossAI : public ScriptedAI
{
    npc_thorim_minibossAI(Creature* creature) : ScriptedAI(creature), _summons(me)
    {
        _instance = creature->GetInstanceScript();

        SetBoundary(&ArenaBoundaries, true);
    }

    bool CanAIAttack(Unit const* who) const final override
    {
        return CheckBoundary(who);
    }

    void JustSummoned(Creature* summon) final override
    {
        _summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature* summon) final override
    {
        _summons.Despawn(summon);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_ACTIVATE_ADDS)
        {
            for (ObjectGuid const& guid : _summons)
                if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                    summon->SetImmuneToPC(false);
        }
    }

protected:
    InstanceScript* _instance;
    EventMap _events;
    SummonList _summons;
};

class npc_runic_colossus : public CreatureScript
{
    public:
        npc_runic_colossus() : CreatureScript("npc_runic_colossus") { }

        struct npc_runic_colossusAI : public npc_thorim_minibossAI
        {
            npc_runic_colossusAI(Creature* creature) : npc_thorim_minibossAI(creature)
            {
                Initialize();
            }

            void Initialize()
            {
                _runicActive = false;
            }

            void Reset() override
            {
                Initialize();
                _events.Reset();

                // close the Runic Door
                _instance->HandleGameObject(_instance->GetGuidData(DATA_RUNIC_DOOR), false);

                // Spawn trashes
                _summons.DespawnAll();
                for (SummonLocation const& s : ColossusAddLocations)
                    me->SummonCreature(s.entry, s.pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
            }

            void MoveInLineOfSight(Unit* /*who*/) override
            {
                // don't enter combat
            }

            void DoAction(int32 action) override
            {
                npc_thorim_minibossAI::DoAction(action);

                if (_runicActive)
                    return;

                if (action == ACTION_ACTIVATE_RUNIC_SMASH)
                {
                    _runicActive = true;
                    _events.ScheduleEvent(EVENT_RUNIC_SMASH, 7000);
                }
            }

            void JustDied(Unit* /*victim*/) override
            {
                // open the Runic Door
                _instance->HandleGameObject(_instance->GetGuidData(DATA_RUNIC_DOOR), true);

                if (Creature* thorim = _instance->GetCreature(BOSS_THORIM))
                    thorim->AI()->Talk(SAY_SPECIAL);

                if (Creature* giant = _instance->GetCreature(DATA_RUNE_GIANT))
                {
                    giant->SetImmuneToPC(false);
                    giant->AI()->DoAction(ACTION_ACTIVATE_ADDS);
                }
            }

            void EnterCombat(Unit* /*who*/) override
            {
                DoZoneInCombat();
                _events.Reset();
                _events.ScheduleEvent(EVENT_RUNIC_BARRIER, urand(12000, 15000));
                _events.ScheduleEvent(EVENT_SMASH, urand(15000, 18000));
                _events.ScheduleEvent(EVENT_RUNIC_CHARGE, urand(20000, 24000));
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RUNIC_BARRIER:
                            Talk(EMOTE_RUNIC_BARRIER);
                            DoCastAOE(SPELL_RUNIC_BARRIER);
                            _events.Repeat(35000, 45000);
                            break;
                        case EVENT_SMASH:
                            DoCastAOE(SPELL_SMASH);
                            _events.Repeat(15000, 18000);
                            break;
                        case EVENT_RUNIC_CHARGE:
                        {
                            Unit* referer = me;
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [referer](Unit* unit){ return unit->GetTypeId() == TYPEID_PLAYER && unit->IsInRange(referer, 8.0f, 40.0f); }))
                                DoCast(target, SPELL_RUNIC_CHARGE);
                            _events.Repeat(20000);
                            break;
                        }
                        case EVENT_RUNIC_SMASH:
                            DoCast(me, RAND(SPELL_RUNIC_SMASH_LEFT, SPELL_RUNIC_SMASH_RIGHT));
                            _events.Repeat(6000);
                            break;
                        default:
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }

        private:
            bool _runicActive;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_runic_colossusAI>(creature);
        }
};

class npc_ancient_rune_giant : public CreatureScript
{
    public:
        npc_ancient_rune_giant() : CreatureScript("npc_ancient_rune_giant") { }

        struct npc_ancient_rune_giantAI : public npc_thorim_minibossAI
        {
            npc_ancient_rune_giantAI(Creature* creature) : npc_thorim_minibossAI(creature) { }

            void Reset() override
            {
                _events.Reset();

                // close the Stone Door
                _instance->HandleGameObject(_instance->GetGuidData(DATA_STONE_DOOR), false);

                // Spawn trashes
                _summons.DespawnAll();
                for (SummonLocation const& s : GiantAddLocations)
                    me->SummonCreature(s.entry, s.pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
            }

            void EnterCombat(Unit* /*who*/) override
            {
                DoZoneInCombat();
                _events.Reset();
                _events.ScheduleEvent(EVENT_RUNIC_FORTIFICATION, 1);
                _events.ScheduleEvent(EVENT_STOMP, urand(10000, 12000));
                _events.ScheduleEvent(EVENT_RUNE_DETONATION, 25000);
            }

            void JustDied(Unit* /*victim*/) override
            {
                // opem the Stone Door
                _instance->HandleGameObject(_instance->GetGuidData(DATA_STONE_DOOR), true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RUNIC_FORTIFICATION:
                            Talk(EMOTE_RUNIC_MIGHT);
                            DoCastAOE(SPELL_RUNIC_FORTIFICATION);
                            break;
                        case EVENT_STOMP:
                            DoCastAOE(SPELL_STOMP);
                            _events.Repeat(10000, 12000);
                            break;
                        case EVENT_RUNE_DETONATION:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                                DoCast(target, SPELL_RUNE_DETONATION);
                            _events.Repeat(10000, 12000);
                            break;
                        default:
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_ancient_rune_giantAI>(creature);
        }
};

class npc_sif : public CreatureScript
{
    public:
        npc_sif() : CreatureScript("npc_sif") { }

        struct npc_sifAI : public ScriptedAI
        {
            npc_sifAI(Creature* creature) : ScriptedAI(creature)
            {
                SetCombatMovement(false);
                _instance = creature->GetInstanceScript();
            }

            void Reset() override
            {
                _events.Reset();
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spellInfo) override
            {
                if (spellInfo->Id == SPELL_STORMHAMMER_SIF)
                {
                    me->InterruptSpell(CURRENT_GENERIC_SPELL);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                }
            }

            void DoAction(int32 action) override
            {
                if (action == ACTION_START_HARD_MODE)
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 250.0f);
                    Talk(SAY_SIF_EVENT);
                    _events.Reset();
                    _events.ScheduleEvent(EVENT_FROSTBOLT, 2000);
                    _events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, 15000);
                    _events.ScheduleEvent(EVENT_BLINK, urand(20000, 25000));
                    _events.ScheduleEvent(EVENT_BLIZZARD, 30000);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BLINK:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_BLINK);
                            _events.ScheduleEvent(EVENT_FROST_NOVA, 0);
                            _events.Repeat(20000, 25000);
                            return;
                        case EVENT_FROST_NOVA:
                            DoCastAOE(SPELL_FROSTNOVA);
                            return;
                        case EVENT_FROSTBOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_FROSTBOLT);
                            _events.Repeat(2000);
                            return;
                        case EVENT_FROSTBOLT_VOLLEY:
                            DoCastAOE(SPELL_FROSTBOLT_VOLLEY);
                            _events.Repeat(15000, 20000);
                            return;
                        case EVENT_BLIZZARD:
                            DoCastAOE(SPELL_BLIZZARD);
                            _events.Repeat(35000, 45000);
                            return;
                        default:
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }

                // no melee attack
            }

        private:
            EventMap _events;
            InstanceScript* _instance;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_sifAI>(creature);
        }
};

// 62576 - Blizzard
// 62602 - Blizzard
class spell_thorim_blizzard_effect : public SpellScriptLoader
{
    public:
        spell_thorim_blizzard_effect() : SpellScriptLoader("spell_thorim_blizzard_effect") { }

        class spell_thorim_blizzard_effect_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_thorim_blizzard_effect_AuraScript);

            bool CheckAreaTarget(Unit* target)
            {
                /// @todo: fix this for all dynobj auras
                if (target != GetOwner())
                {
                    // check if not stacking aura already on target
                    // this one prevents overriding auras periodically by 2 near area aura owners
                    Unit::AuraApplicationMap const& auraMap = target->GetAppliedAuras();
                    for (Unit::AuraApplicationMap::const_iterator iter = auraMap.begin(); iter != auraMap.end(); ++iter)
                    {
                        Aura const* aura = iter->second->GetBase();
                        if (GetId() == aura->GetId() && GetOwner() != aura->GetOwner() /*!GetAura()->CanStackWith(aura)*/)
                            return false;
                    }
                }

                return true;
            }

            void Register() override
            {
                DoCheckAreaTarget += AuraCheckAreaTargetFn(spell_thorim_blizzard_effect_AuraScript::CheckAreaTarget);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_thorim_blizzard_effect_AuraScript();
        }
};

// 62580, 62604 - Frostbolt Volley
class spell_thorim_frostbolt_volley : public SpellScriptLoader
{
    public:
        spell_thorim_frostbolt_volley() : SpellScriptLoader("spell_thorim_frostbolt_volley") { }

        class spell_thorim_frostbolt_volley_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_frostbolt_volley_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if([](WorldObject* target)
                    {
                        return target->GetTypeId() != TYPEID_PLAYER && (target->GetTypeId() != TYPEID_UNIT || !target->ToUnit()->IsPet());
                    });
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_thorim_frostbolt_volley_SpellScript::FilterTargets, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_frostbolt_volley_SpellScript();
        }
};

// 62016 - Charge Orb
class spell_thorim_charge_orb : public SpellScriptLoader
{
    public:
        spell_thorim_charge_orb() : SpellScriptLoader("spell_thorim_charge_orb") { }

        class spell_thorim_charge_orb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_charge_orb_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_LIGHTNING_PILLAR_1 });
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(HeightPositionCheck(false));

                if (targets.empty())
                    return;

                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                targets.clear();
                targets.push_back(target);
            }

            void HandleScript()
            {
                if (Unit* target = GetHitUnit())
                    target->CastSpell(nullptr, SPELL_LIGHTNING_PILLAR_1, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_thorim_charge_orb_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                AfterHit += SpellHitFn(spell_thorim_charge_orb_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_charge_orb_SpellScript();
        }
};

// 62466 - Lightning Charge
class spell_thorim_lightning_charge : public SpellScriptLoader
{
    public:
        spell_thorim_lightning_charge() : SpellScriptLoader("spell_thorim_lightning_charge") { }

        class spell_thorim_lightning_charge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_lightning_charge_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_LIGHTNING_CHARGE });
            }

            void HandleFocus()
            {
                /// @workaround: focus target is not working because spell is triggered and instant
                if (Creature* creature = GetCaster()->ToCreature())
                    creature->FocusTarget(GetSpell(), GetExplTargetWorldObject());
            }

            void HandleCharge()
            {
                GetCaster()->CastSpell(GetCaster(), SPELL_LIGHTNING_CHARGE);
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_thorim_lightning_charge_SpellScript::HandleFocus);
                AfterCast += SpellCastFn(spell_thorim_lightning_charge_SpellScript::HandleCharge);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_lightning_charge_SpellScript();
        }
};

// 61934 - Leap
class spell_thorim_arena_leap : public SpellScriptLoader
{
    public:
        spell_thorim_arena_leap() : SpellScriptLoader("spell_thorim_arena_leap") { }

        class spell_thorim_arena_leap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_arena_leap_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_UNIT;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (Position const* pos = GetHitDest())
                    GetCaster()->ToCreature()->SetHomePosition(*pos);
            }

            void Register() override
            {
                OnEffectLaunch += SpellEffectFn(spell_thorim_arena_leap_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_arena_leap_SpellScript();
        }
};

struct OutOfArenaCheck
{
    bool operator()(Position const* who) const
    {
        return !CreatureAI::IsInBounds(ArenaBoundaries, who);
    }
};

// 62042 - Stormhammer
class spell_thorim_stormhammer : public SpellScriptLoader
{
    public:
        spell_thorim_stormhammer() : SpellScriptLoader("spell_thorim_stormhammer") { }

        class spell_thorim_stormhammer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_stormhammer_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_STORMHAMMER_BOOMERANG, SPELL_DEAFENING_THUNDER });
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if([](WorldObject* target) -> bool { return HeightPositionCheck(true)(target) || OutOfArenaCheck()(target); });

                if (targets.empty())
                {
                    FinishCast(SPELL_FAILED_NO_VALID_TARGETS);
                    return;
                }

                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                targets.clear();
                targets.push_back(target);
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    target->CastSpell(target, SPELL_DEAFENING_THUNDER, true);
                    target->CastSpell(GetCaster(), SPELL_STORMHAMMER_BOOMERANG, true);
                }
            }

            void LoseHammer()
            {
                GetCaster()->SetVirtualItem(0, 0);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_thorim_stormhammer_SpellScript::FilterTargets, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ENEMY);
                AfterCast += SpellCastFn(spell_thorim_stormhammer_SpellScript::LoseHammer);
                OnEffectHitTarget += SpellEffectFn(spell_thorim_stormhammer_SpellScript::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_stormhammer_SpellScript();
        }
};

// 64767 - Stormhammer
class spell_thorim_stormhammer_sif : public SpellScriptLoader
{
    public:
        spell_thorim_stormhammer_sif() : SpellScriptLoader("spell_thorim_stormhammer_sif") { }

        class spell_thorim_stormhammer_sif_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_stormhammer_sif_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_STORMHAMMER_BOOMERANG, SPELL_SIF_TRANSFORM });
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    target->CastSpell(GetCaster(), SPELL_STORMHAMMER_BOOMERANG, true);
                    target->CastSpell(target, SPELL_SIF_TRANSFORM, true);
                }
            }

            void LoseHammer()
            {
                GetCaster()->SetVirtualItem(0, 0);
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_thorim_stormhammer_sif_SpellScript::LoseHammer);
                OnEffectHitTarget += SpellEffectFn(spell_thorim_stormhammer_sif_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_stormhammer_sif_SpellScript();
        }
};

// 64909 - Stormhammer
class spell_thorim_stormhammer_boomerang : public SpellScriptLoader
{
    public:
        spell_thorim_stormhammer_boomerang() : SpellScriptLoader("spell_thorim_stormhammer_boomerang") { }

        class spell_thorim_stormhammer_boomerang_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_stormhammer_boomerang_SpellScript);

            void RecoverHammer(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                    target->SetVirtualItem(0, THORIM_WEAPON_DISPLAY_ID);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_thorim_stormhammer_boomerang_SpellScript::RecoverHammer, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_stormhammer_boomerang_SpellScript();
        }
};

// 62057, 62058 - Runic Smash
class spell_thorim_runic_smash : public SpellScriptLoader
{
    public:
        spell_thorim_runic_smash() : SpellScriptLoader("spell_thorim_runic_smash") { }

        class spell_thorim_runic_smash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thorim_runic_smash_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_RUNIC_SMASH });
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                std::vector<Creature*> triggers;
                GetCaster()->GetCreatureListWithEntryInGrid(triggers, GetSpellInfo()->Id == SPELL_RUNIC_SMASH_LEFT ? NPC_GOLEM_LEFT_HAND_BUNNY : NPC_GOLEM_RIGHT_HAND_BUNNY, 150.0f);
                for (Creature* trigger : triggers)
                {
                    float dist = GetCaster()->GetExactDist(trigger);
                    trigger->m_Events.AddEvent(new RunicSmashExplosionEvent(trigger), trigger->m_Events.CalculateTime(uint64(dist * 30.f)));
                };
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_thorim_runic_smash_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thorim_runic_smash_SpellScript();
        }
};

class UpperOrbCheck
{
    public:
        UpperOrbCheck() : _check(true) { }

        bool operator() (Creature* target) const
        {
            return target->GetEntry() == NPC_THUNDER_ORB && _check(target);
        }

    private:
        HeightPositionCheck const _check;
};

// 62184 - Activate Lightning Orb Periodic
class spell_thorim_activate_lightning_orb_periodic : public SpellScriptLoader
{
    public:
        spell_thorim_activate_lightning_orb_periodic() : SpellScriptLoader("spell_thorim_activate_lightning_orb_periodic") { }

        class spell_thorim_activate_lightning_orb_periodic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_thorim_activate_lightning_orb_periodic_AuraScript);

            InstanceScript* instance = nullptr;

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();

                Unit* caster = GetCaster();
                std::vector<Creature*> triggers;

                UpperOrbCheck check;
                Trinity::CreatureListSearcher<UpperOrbCheck> searcher(caster, triggers, check);
                Cell::VisitGridObjects(caster, searcher, 100.f);

                if (!triggers.empty())
                {
                    Creature* target = Trinity::Containers::SelectRandomContainerElement(triggers);
                    if (Creature* thorim = instance->GetCreature(BOSS_THORIM))
                        thorim->AI()->SetGUID(target->GetGUID(), DATA_CHARGED_PILLAR);
                }
            }

            bool Load() override
            {
                if (Unit* caster = GetCaster())
                    instance = caster->GetInstanceScript();

                return instance != nullptr;
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_thorim_activate_lightning_orb_periodic_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_thorim_activate_lightning_orb_periodic_AuraScript();
        }
};

// 62331, 62418 - Impale
class spell_iron_ring_guard_impale : public SpellScriptLoader
{
    public:
        spell_iron_ring_guard_impale() : SpellScriptLoader("spell_iron_ring_guard_impale") { }

        class spell_iron_ring_guard_impale_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_iron_ring_guard_impale_AuraScript);

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (GetTarget()->HealthAbovePct(GetSpellInfo()->GetEffect(EFFECT_1)->CalcValue()))
                {
                    Remove(AURA_REMOVE_BY_ENEMY_SPELL);
                    PreventDefaultAction();
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_iron_ring_guard_impale_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_iron_ring_guard_impale_AuraScript();
        }
};

class condition_thorim_arena_leap : public ConditionScript
{
    public:
        condition_thorim_arena_leap() : ConditionScript("condition_thorim_arena_leap"), _check(false) { }

        bool OnConditionCheck(Condition const* condition, ConditionSourceInfo& sourceInfo) override
        {
            WorldObject* target = sourceInfo.mConditionTargets[condition->ConditionTarget];
            InstanceScript* instance = target->GetInstanceScript();

            if (!instance)
                return false;

            return _check(target);
        }

    private:
        HeightPositionCheck _check;
};

void AddSC_boss_thorim()
{
    new boss_thorim();
}
