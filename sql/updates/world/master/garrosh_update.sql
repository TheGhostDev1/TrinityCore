DELETE FROM `instance_template` WHERE `map`=1136;
INSERT INTO `instance_template` (`map`, `parent`, `script`, `allowMount`) VALUES
(1136, 870, 'instance_siege_of_orgrimmar', 1);

DELETE FROM `spell_target_position` WHERE `ID` IN (144884,144886,144881) AND `EffectIndex`=0;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `VerifiedBuild`) VALUES
(144884, 0, 1136, 1047.26, -5847.24, -318.856, 0),
(144886, 0, 1136, 1084.47, -5630.96, -423.455, 0),
(144881, 0, 1136, 1092.46, -5455.42, -354.904, 0);

DELETE FROM `areatrigger_scripts` WHERE `entry` IN(9412,9416,9417,9418,9419,9420,9421,9422);
INSERT INTO `areatrigger_scripts` (`entry`, `ScriptName`) VALUES
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
