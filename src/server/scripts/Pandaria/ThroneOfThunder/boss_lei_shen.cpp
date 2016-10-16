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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Player.h"
#include "Vehicle.h"
#include "throne_of_thunder.h"

enum Spells
{
	SPELL_ANCHOR_HERE			= 45313, //SetHomePosition(Here)
	SPELL_COSMETIC_VISUAL		= 138275,
	SPELL_COSMETIC_TELEPORT_W	= 139104,
	SPELL_COSMETIC_TELEPORT_N	= 139103,
	SPELL_COSMETIC_TELEPORT_E	= 139102,
	SPELL_COSMETIC_TELEPORT_S	= 139105,

	SPELL_IN_WEST_QUADRANT		= 139009,
	SPELL_IN_NORTH_QUADRANT		= 139006,
	SPELL_IN_EAST_QUADRANT		= 139007,
	SPELL_IN_SOUTH_QUADRANT		= 139008,

    // Other
    SPELL_DISPLACEMENT_TO_LEI_SHEN = 139110,
};

enum AreaTriggers
{
	AREATRIGGER_NORTH_QUADRANT	= 8910,
	AREATRIGGER_EAST_QUADRANT	= 8911,
	AREATRIGGER_SOUTH_QUADRANT	= 8912,
	AREATRIGGER_WEST_QUADRANT	= 8913,
};

enum Texts
{
};

enum Events
{
    EVENT_NONE,
	
	EVENT_IDLE_COSMETIC_CHANNEL,
	EVENT_IDLE_COSMETIC_TELEPORT,
};

enum Points
{
    POINT_NONE,
};

int const LeiShenIntro[4][2] = {
	{ NPC_BOUNCING_BOLT_CONDUIT,	SPELL_COSMETIC_TELEPORT_N },
	{ NPC_STATIC_SHOCK_CONDUIT,		SPELL_COSMETIC_TELEPORT_E },
	{ NPC_DIFFUSION_CHAIN_CONDUIT,	SPELL_COSMETIC_TELEPORT_S },
	{ NPC_OVERCHARGE_CONDUIT,		SPELL_COSMETIC_TELEPORT_W }
};

//int const QuadrantAuras[4] = { SPELL_IN_WEST_QUADRANT, SPELL_IN_NORTH_QUADRANT, SPELL_IN_EAST_QUADRANT, SPELL_IN_SOUTH_QUADRANT};
uint32 QuadrantAuras[4][2] = {
	{ AREATRIGGER_WEST_QUADRANT,	SPELL_IN_WEST_QUADRANT },
	{ AREATRIGGER_NORTH_QUADRANT,	SPELL_IN_NORTH_QUADRANT },
	{ AREATRIGGER_EAST_QUADRANT,	SPELL_IN_EAST_QUADRANT },
	{ AREATRIGGER_SOUTH_QUADRANT,	SPELL_IN_SOUTH_QUADRANT }
};

struct PathInfo
{
    uint32 Size;
    G3D::Vector3 Path[8];
};

uint32 const DisplacementPathSize = 4;
PathInfo const DisplacementPaths[DisplacementPathSize] =
{
    { 8, 
        { 
            { 5899.261f, 4099.082f, 202.5637f },
            { 5873.366f, 4088.847f, 205.9732f },
            { 5825.932f, 4089.73f, 194.5963f },
            { 5810.307f, 4036.47f, 187.8653f },
            { 5719.497f, 3980.462f, 187.8653f },
            { 5639.718f, 4020.283f, 187.8653f },
            { 5685.288f, 4086.411f, 187.8653f },
            { 5711.103f, 4096.278f, 159.1069f }
        }
    },

    { 6, 
        {
            { 5895.271f, 4096.196f, 202.4029f },
            { 5853.417f, 4088.592f, 216.8568f },
            { 5817.982f, 4103.582f, 199.3384f },
            { 5721.491f, 4151.757f, 199.3384f },
            { 5677.582f, 4131.644f, 199.3384f },
            { 5711.02f, 4094.822f, 157.6325f }
        }
    },

    { 6,
        {
            { 5901.929f, 4101.393f, 202.2954f },
            { 5884.275f, 4082.887f, 208.4285f },
            { 5822.514f, 4083.903f, 196.7104f },
            { 5760.973f, 4047.318f, 196.7104f },
            { 5695.247f, 4084.318f, 196.7104f },
            { 5710.495f, 4094.162f, 157.1986f }
        }
    },

    { 6, 
        {
            { 5896.792f, 4100.8f, 202.5638f },
            { 5898.243f, 4098.249f, 207.4689f },
            { 5861.318f, 4101.68f, 207.4689f },
            { 5832.268f, 4079.525f, 190.8674f },
            { 5764.341f, 4111.771f, 190.8674f },
            { 5710.285f, 4094.418f, 156.9281f } 
        }
    }
};

enum Phase
{
    PHASE_NONE,
    PHASE_ONE,
    PHASE_INTERMISSION_ONE,
    PHASE_TWO,
    PHASE_INTERMISSION_TWO,
    PHASE_THREE,

    PHASE_MASK_NONE             = 1 << PHASE_NONE,
    PHASE_MASK_ONE              = 1 << PHASE_ONE,
    PHASE_MASK_INTERMISSION_ONE = 1 << PHASE_INTERMISSION_ONE,
    PHASE_MASK_TWO              = 1 << PHASE_TWO,
    PHASE_MASK_INTERMISSION_TWO = 1 << PHASE_INTERMISSION_TWO,
    PHASE_MASK_THREE            = 1 << PHASE_THREE,
};

Position const ThunderKingDiscCollisionPosition = { 5710.495f, 4094.242f, 156.7227 };

class boss_lei_shen : public CreatureScript
{
    public:
		boss_lei_shen() : CreatureScript("boss_lei_shen") { }

		struct boss_lei_shenAI : public BossAI
        {
			boss_lei_shenAI(Creature* creature) : BossAI(creature, DATA_LEI_SHEN) { }

			void InitializeAI() override
			{
				if (!me->isDead())
				{
					_intro = true;
					JustRespawned();
				}
				else
					_intro = false;
			}

			void JustRespawned() override
			{
				Reset();

				events.ScheduleEvent(EVENT_IDLE_COSMETIC_CHANNEL, 1 * IN_MILLISECONDS);
			}

			void Reset() override
			{
				_Reset();
			}

			void EnterCombat(Unit* who) override
			{
				events.Reset();

				// Schedule EnterCombat
			}

            void PreparePhase(Phase phase)
            {
                switch (phase)
                {
                    case PHASE_ONE:
                        scheduler.Schedule(Seconds(1), [this](TaskContext task)
                        break;
                    case PHASE_INTERMISSION_ONE:
                        break;
                    case PHASE_TWO:
                        break;
                    case PHASE_INTERMISSION_TWO:
                        break;
                    case PHASE_THREE:
                        break;
                    default:
                        break;
                }
            }

			void SpellHitTarget(Unit* target, SpellInfo const* spell) override
			{
				if (target != me)
					return;

                if (spell->Id == SPELL_COSMETIC_TELEPORT_W || spell->Id == SPELL_COSMETIC_TELEPORT_N ||
                    spell->Id == SPELL_COSMETIC_TELEPORT_E || spell->Id == SPELL_COSMETIC_TELEPORT_S)
                {
                    me->SetFacingToObject(FindNearestConduit());
                    DoCastAOE(SPELL_ANCHOR_HERE, true);
                    events.ScheduleEvent(EVENT_IDLE_COSMETIC_CHANNEL, 1 * IN_MILLISECONDS);
                }
			}

			void UpdateAI(uint32 diff) override
			{
				events.Update(diff);

				while (uint32 eventId = events.ExecuteEvent())
				{
					switch (eventId)
					{
						case EVENT_IDLE_COSMETIC_TELEPORT:
						{

							Creature* conduit = FindNearestConduit();
							if (conduit == nullptr)
								return;

							for (uint32 i = 0; i < 4; i++)
							{
								if (LeiShenIntro[i][0] == conduit->GetEntry())
								{
									DoCastAOE(LeiShenIntro[i][1]);
									break;
								}
							}
						}
							break;
						case EVENT_IDLE_COSMETIC_CHANNEL:
						{
							Creature* conduit = FindNearestConduit();
							if (conduit == nullptr)
								break;

							DoCast(conduit, SPELL_COSMETIC_VISUAL);
							events.ScheduleEvent(EVENT_IDLE_COSMETIC_TELEPORT, 12 * IN_MILLISECONDS);
						}
							break;
						default:
							break;
					}
				}

				DoMeleeAttackIfReady();
			}

		private:
			bool _intro;
            TaskScheduler scheduler;

			Creature* FindNearestConduit()
			{
				std::list<Creature*> conduitList;
				me->GetCreatureListWithEntryInGrid(conduitList, NPC_BOUNCING_BOLT_CONDUIT,		150.0f);
				me->GetCreatureListWithEntryInGrid(conduitList, NPC_DIFFUSION_CHAIN_CONDUIT,	150.0f);
				me->GetCreatureListWithEntryInGrid(conduitList, NPC_OVERCHARGE_CONDUIT,			150.0f);
				me->GetCreatureListWithEntryInGrid(conduitList, NPC_STATIC_SHOCK_CONDUIT,		150.0f);
				conduitList.sort(Trinity::ObjectDistanceOrderPred(me));

				return conduitList.front();
			}

        };

        CreatureAI* GetAI(Creature* creature) const override
        {
			return new boss_lei_shenAI(creature);
			//return GetInstanceAI<boss_lei_shenAI>(creature);
        }
};

class go_displacement_pad_to_lei_shen : public GameObjectScript
{
    public:

    go_displacement_pad_to_lei_shen()
        : GameObjectScript("go_displacement_pad_to_lei_shen")
    {
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 /*action*/) override
    {
        return true;

        player->AddAura(SPELL_DISPLACEMENT_TO_LEI_SHEN, player);
        DisplacePlayer(player, DisplacementPaths[urand(0, DisplacementPathSize)]);

        return true;
    }

    private:
    void DisplacePlayer(Player* player, PathInfo path)
    {
        Movement::PointsArray _path(path.Path, path.Path + path.Size);

        Movement::MoveSplineInit init(player);
        init.MovebyPath(_path);
        init.SetFly();
        init.SetVelocity(50.0f);
        /*init.SetUncompressed();*/
        init.Launch();
    }
};

// http://www.wowhead.com/spell=136913/overwhelming-power
class spell_lei_shen_overwhelming_power : public SpellScriptLoader
{
public:
	spell_lei_shen_overwhelming_power() : SpellScriptLoader("spell_lei_shen_overwhelming_power") { }

	class spell_lei_shen_overwhelming_power_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_lei_shen_overwhelming_power_SpellScript);

		void SelectTarget(std::list<WorldObject*>& targets)
		{
			targets.remove_if([this](WorldObject* target)
			{
				return target->GetEntry() != NPC_OVERWHELMING_POWER;
			});
		}

		void Register() override
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lei_shen_overwhelming_power_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
		}
	};

	SpellScript* GetSpellScript() const override
	{
		return new spell_lei_shen_overwhelming_power_SpellScript();
	}
};

class at_lei_shen_quadrant : public AreaTriggerScript
{
	public:
		at_lei_shen_quadrant() : AreaTriggerScript("at_lei_shen_quadrant") { }

		bool OnTrigger(Player* player, AreaTriggerEntry const* at, bool entered) override
		{
            switch (at->ID)
            {
                case AREATRIGGER_WEST_QUADRANT:
                    if (entered)
                        player->CastSpell(player, SPELL_IN_WEST_QUADRANT, true);
                    else
                        player->RemoveAurasDueToSpell(SPELL_IN_WEST_QUADRANT);
                    break;
                case AREATRIGGER_NORTH_QUADRANT:
                    if (entered)
                        player->CastSpell(player, SPELL_IN_NORTH_QUADRANT, true);
                    else
                        player->RemoveAurasDueToSpell(SPELL_IN_NORTH_QUADRANT);
                    break;
                case AREATRIGGER_EAST_QUADRANT:
                    if (entered)
                        player->CastSpell(player, SPELL_IN_EAST_QUADRANT, true);
                    else
                        player->RemoveAurasDueToSpell(SPELL_IN_EAST_QUADRANT);
                    break;
                case AREATRIGGER_SOUTH_QUADRANT:
                    if (entered)
                        player->CastSpell(player, SPELL_IN_SOUTH_QUADRANT, true);
                    else
                        player->RemoveAurasDueToSpell(SPELL_IN_SOUTH_QUADRANT);
                    break;
                default:
                    break;
            }

			return true;
		}
};

void AddSC_boss_lei_shen()
{
    new boss_lei_shen();

    new go_displacement_pad_to_lei_shen();

	new spell_lei_shen_overwhelming_power();

	new at_lei_shen_quadrant();
}
