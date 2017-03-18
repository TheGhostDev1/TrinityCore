DELETE FROM `instance_template` WHERE `map`=1136;
INSERT INTO `instance_template` (`map`, `parent`, `script`, `allowMount`) VALUES
(1136, 870, 'instance_siege_of_orgrimmar', 1);

DELETE FROM `spell_target_position` WHERE `ID` IN (144884,144886,144881) AND `EffectIndex`=0;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `VerifiedBuild`) VALUES
(144884, 0, 1136, 1047.26, -5847.24, -318.856, 0),
(144886, 0, 1136, 1084.47, -5630.96, -423.455, 0),
(144881, 0, 1136, 1092.46, -5455.42, -354.904, 0);

DELETE FROM `areatrigger_scripts` WHERE `entry` IN(9374,9412,9416,9417,9418,9419,9420,9421,9422);
INSERT INTO `areatrigger_scripts` (`entry`, `ScriptName`) VALUES
(9374, 'at_soo_garrosh_intro'),
(9412, 'at_soo_garrosh_fire_pit'),
(9416, 'at_soo_garrosh_fire_pit'),
(9417, 'at_soo_garrosh_fire_pit'),
(9418, 'at_soo_garrosh_fire_pit'),
(9419, 'at_soo_garrosh_fire_pit'),
(9420, 'at_soo_garrosh_fire_pit'),
(9421, 'at_soo_garrosh_fire_pit'),
(9422, 'at_soo_garrosh_fire_pit');

DELETE FROM `areatrigger_template` WHERE `Id`=4997;
INSERT INTO `areatrigger_template` (`Id`, `Type`, `Flags`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `ScriptName`, `VerifiedBuild`) VALUES
(4997, 3, 4, 0, 0, 0, 0, 0, 0, 'at_entity_korkron_iron_star', 23420);

DELETE FROM `areatrigger_template_polygon_vertices` WHERE `AreaTriggerId`=4997;
INSERT INTO `areatrigger_template_polygon_vertices` (`AreaTriggerId`, `Idx`, `VerticeX`, `VerticeY`, `VerticeTargetX`, `VerticeTargetY`, `VerifiedBuild`) VALUES
(4997, 0, -17,  29, -17,  29, 23420),
(4997, 1, -17, -54, -17, -54, 23420),
(4997, 2,  17, -54,  17, -54, 23420),
(4997, 3,  17,  29,  17,  29, 23420);

UPDATE `creature_template` SET `InhabitType`=4, flags_extra=128 WHERE `entry`=72215;
UPDATE `creature_template` SET `unit_flags`=32832, `flags_extra`=1, `ScriptName`='boss_garrosh_hellscream' WHERE `entry`=71865;
UPDATE `creature_template` SET `InhabitType`=4 WHERE `entry`=71985;

UPDATE `creature_template` SET `ScriptName`='npc_garrosh_thrall' WHERE `entry`=73483;

UPDATE `gameobject_template` SET `ScriptName`='go_garrosh_hellscream_sanctum_door' WHERE `entry`=221792;

DELETE FROM `gameobject_template_addon` WHERE `entry` IN (221442, 221132, 221995);
INSERT INTO `gameobject_template_addon` (`entry`, `faction`, `flags`) VALUES
(221442, 1375, 0),
(221132, 0, 32),
(221995, 1375, 32);

UPDATE `creature_template` SET `ScriptName`='npc_heart_of_yshaarj_main_room' WHERE `entry`=72215;
UPDATE `creature_template` SET `ScriptName`='npc_korkron_iron_star' WHERE `entry`=71985;
UPDATE `creature_template` SET `ScriptName`='npc_garrosh_siege_engineer' WHERE `entry`=71984;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_garrosh_hellscream_desecrate',
 'spell_garrosh_weak_minded',
 'spell_power_iron_star',
 'spell_enter_realm_of_yshaarj',
 'spell_enter_realm_of_yshaarj_aoe',
 'spell_enter_realm_of_yshaarj_jump',
 'spell_garrosh_jump_to_ground',
 'spell_garrosh_attack_thrall',
 'spell_garrosh_realm_of_yshaarj');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(144745, 'spell_garrosh_hellscream_desecrate'),
(145331, 'spell_garrosh_weak_minded'),
(144616, 'spell_power_iron_star'),
(144866, 'spell_enter_realm_of_yshaarj'),
(144867, 'spell_enter_realm_of_yshaarj_aoe'),
(144868, 'spell_enter_realm_of_yshaarj_jump'),
(144870, 'spell_enter_realm_of_yshaarj_jump'),
(144956, 'spell_garrosh_jump_to_ground'),
(147831, 'spell_garrosh_attack_thrall'),
(144954, 'spell_garrosh_realm_of_yshaarj');

DELETE FROM `creature_text` WHERE `entry`=71865;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(71865, 0, 0, NULL, 14, 0, 100, 0, 0, 0, 74701, 3, 'Garrosh Hellscream - Intro 1'),
(71865, 1, 0, NULL, 14, 0, 100, 0, 0, 0, 74702, 3, 'Garrosh Hellscream - Intro 2'),
(71865, 2, 0, NULL, 14, 0, 100, 0, 0, 0, 74703, 3, 'Garrosh Hellscream - Intro 3'),
(71865, 3, 0, NULL, 14, 0, 100, 0, 0, 0, 74704, 3, 'Garrosh Hellscream - Intro 4'),
(71865, 4, 0, NULL, 14, 0, 100, 0, 0, 0, 74705, 3, 'Garrosh Hellscream - Intro 5'),
(71865, 5, 0, NULL, 14, 0, 100, 0, 0, 0, 75825, 3, 'Garrosh Hellscream - Enter Combat'),
(71865, 6, 0, NULL, 14, 0, 100, 0, 0, 0, 74717, 3, 'Garrosh Hellscream - Kill 1'),
(71865, 6, 1, NULL, 14, 0, 100, 0, 0, 0, 74718, 3, 'Garrosh Hellscream - Kill 2'),
(71865, 7, 0, NULL, 14, 0, 100, 0, 0, 0, 74719, 3, 'Garrosh Hellscream - Evade'),
(71865, 8, 0, NULL, 14, 0, 100, 0, 0, 0, 74716, 3, 'Garrosh Hellscream - Berserk'),
(71865, 9, 0, NULL, 14, 0, 100, 0, 0, 0, 75369, 3, 'Garrosh Hellscream - Death Normal'),
(71865, 10, 0, NULL, 14, 0, 100, 0, 0, 0, 75824, 3, 'Garrosh Hellscream - Death Heroic'),
(71865, 11, 0, NULL, 14, 0, 100, 0, 0, 0, 74709, 3, 'Garrosh Hellscream - Summon Warbringers (UNUSED'),
(71865, 12, 0, NULL, 14, 0, 100, 0, 0, 0, 74710, 3, 'Garrosh Hellscream - Summon Far Seers'),
(71865, 13, 0, NULL, 14, 0, 100, 0, 0, 0, 74713, 3, 'Garrosh Hellscream - Hellscreams Warsong 1'),
(71865, 13, 1, NULL, 14, 0, 100, 0, 0, 0, 74712, 3, 'Garrosh Hellscream - Hellscreams Warsong 2'),
(71865, 13, 2, NULL, 14, 0, 100, 0, 0, 0, 74711, 3, 'Garrosh Hellscream - Hellscreams Warsong 3'),
(71865, 14, 0, NULL, 14, 0, 100, 0, 0, 0, 74714, 3, 'Garrosh Hellscream - Whirling Corruption'),
(71865, 15, 0, NULL, 14, 0, 100, 0, 0, 0, 74715, 3, 'Garrosh Hellscream - Empowered Whirling Corruption'),
(71865, 16, 0, NULL, 14, 0, 100, 0, 0, 0, 74708, 3, 'Garrosh Hellscream - Kor\'kron Iron Star'),
(71865, 17, 0, NULL, 14, 0, 100, 0, 0, 0, 74610, 3, 'Garrosh Hellscream - Phase 2 1'),
(71865, 18, 0, NULL, 14, 0, 100, 0, 0, 0, 74616, 3, 'Garrosh Hellscream - Phase 2 2'),
(71865, 19, 0, NULL, 14, 0, 100, 0, 0, 0, 74720, 3, 'Garrosh Hellscream - Realm of Y\'shaarj 1'),
(71865, 19, 1, NULL, 14, 0, 100, 0, 0, 0, 74721, 3, 'Garrosh Hellscream - Realm of Y\'shaarj 2'),
(71865, 19, 2, NULL, 14, 0, 100, 0, 0, 0, 74722, 3, 'Garrosh Hellscream - Realm of Y\'shaarj 3'),
(71865, 19, 3, NULL, 14, 0, 100, 0, 0, 0, 74723, 3, 'Garrosh Hellscream - Realm of Y\'shaarj 4'),
(71865, 20, 0, NULL, 14, 0, 100, 0, 0, 0, 74724, 3, 'Garrosh Hellscream - Phase 3'),
(71865, 21, 0, NULL, 14, 0, 100, 0, 0, 0, 75820, 3, 'Garrosh Hellscream - Phase 4 (Heroic) 1'),
(71865, 22, 0, NULL, 14, 0, 100, 0, 0, 0, 75821, 3, 'Garrosh Hellscream - Phase 4 (Heroic) 2'),
(71865, 23, 0, NULL, 14, 0, 100, 0, 0, 0, 75823, 3, 'Garrosh Hellscream - Manifest Rage'),
(71865, 24, 0, NULL, 14, 0, 100, 0, 0, 0, 75822, 3, 'Garrosh Hellscream - Call Bombardment'),
(71865, 25, 0, NULL, 14, 0, 100, 0, 0, 0, 74706, 3, 'Garrosh Hellscream - Unk 1'),
(71865, 26, 0, NULL, 41, 0, 100, 0, 0, 0, 74586, 3, 'Garrosh Hellscream - (Emote) Siege Engineers'),
(71865, 27, 0, NULL, 41, 0, 100, 0, 0, 0, 75753, 3, 'Garrosh Hellscream - (Emote) Whirling Corruption'),
(71865, 28, 0, NULL, 41, 0, 100, 0, 0, 0, 75764, 3, 'Garrosh Hellscream - (Emote) Empowered Whirling Corruption'),
(71865, 29, 0, NULL, 42, 0, 100, 0, 0, 0, 74621, 3, 'Garrosh Hellscream - (Emote) Whirling Corruption Tip'),
(71865, 30, 0, NULL, 41, 0, 100, 0, 0, 0, 75762, 3, 'Garrosh Hellscream - (Emote) Manifest Rage'),
(71865, 31, 0, NULL, 41, 0, 100, 0, 0, 0, 75795, 3, 'Garrosh Hellscream - (Emote) Call Bombardment'),
(71865, 32, 0, NULL, 41, 0, 100, 0, 0, 0, 75799, 3, 'Garrosh Hellscream - (Emote) Unstable Iron Star'),
(71865, 33, 0, NULL, 41, 0, 100, 0, 0, 0, 75843, 3, 'Garrosh Hellscream - (Emote) Malice');

DELETE FROM `creature_text` WHERE `entry`=73483;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(73483, 0, 0, NULL, 14, 0, 100, 0, 0, 0, 74698, 3, 'Garrosh Hellscream (Thrall) - Intro 1'),
(73483, 1, 0, NULL, 14, 0, 100, 0, 0, 0, 74699, 3, 'Garrosh Hellscream (Thrall) - Intro 2'),
(73483, 2, 0, NULL, 14, 0, 100, 0, 0, 0, 74700, 3, 'Garrosh Hellscream (Thrall) - Intro 3');

DELETE FROM `creature_summon_groups` WHERE `summonerId`=71865;
INSERT INTO `creature_summon_groups` (`summonerId`, `summonerType`, `groupId`, `entry`, `position_x`, `position_y`, `position_z`, `orientation`, `summonType`, `summonTime`) VALUES
(71865, 0, 3, 71984, 1058.622, -5780.127, -304.8477, 0.964128, 8, 0),
(71865, 0, 3, 71984, 1087.634, -5499.806, -304.8515, 4.552298, 8, 0),
(71865, 0, 4, 71985, 1087.052, -5758.288, -317.5687, 1.459921, 8, 0),
(71865, 0, 4, 71985, 1059.924, -5520.203, -317.5687, 4.647993, 8, 0);

