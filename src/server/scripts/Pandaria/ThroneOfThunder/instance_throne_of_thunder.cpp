/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
#include "Player.h"
#include "CreatureGroups.h"
#include "InstanceScript.h"
#include "throne_of_thunder.h"

#define MAX_ENCOUNTER 13

/* Throne of Thunder encounters:
0 - Jin'rokh the Breaker
1 - Horridon
2 - Council of Elders
3 - Tortos
4 - Megaera
5 - Ji-Kun
6 - Durumu the Forgotten
7 - Primordius
8 - Dark Animus
9 - Iron Qon
10 - Twin Consorts
11 - Lei Shen
12 - Ra-den
*/

ObjectData const creatureData[] =
{
    { NPC_LEI_SHEN, DATA_LEI_SHEN },
    { 0, 0 } // END
};

class instance_throne_of_thunder : public InstanceMapScript
{
    public:
		instance_throne_of_thunder() : InstanceMapScript(TOTScriptName, 1098) { }

		struct instance_throne_of_thunder_InstanceScript : public InstanceScript
        {
			instance_throne_of_thunder_InstanceScript(Map* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(MAX_ENCOUNTER);
                //LoadObjectData(creatureData, nullptr);
            }

            void OnGameObjectCreate(GameObject* go) override
            {
                switch (go->GetEntry())
                {
                    default:
                        break;
                }
            }

            void OnCreatureCreate(Creature* creature) override
            {
                switch (creature->GetEntry())
                {
                    default:
                        break;
                }

                InstanceScript::OnCreatureCreate(creature);
            }

            bool SetBossState(uint32 type, EncounterState state) override
            {

                return InstanceScript::SetBossState(type, state);
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    default:
                        break;
                }

                return 0;
            }

            void SetData(uint32 type, uint32 data) override
            {
                switch (type)
                {
                    default:
                        break;
                }
            }
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
			return new instance_throne_of_thunder_InstanceScript(map);
        }
};

void AddSC_instance_throne_of_thunder()
{
	new instance_throne_of_thunder();
}
