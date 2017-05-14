/*
   Copyright 2017 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum TileBehavior : uint8_t
{
    TileBehavior_GenericWalkable,
    TileBehavior_Sand,
    TileBehavior_SlowStairs,
    TileBehavior_Stairs,

    TileBehavior_Water,
    TileBehavior_GenericSolid,
    TileBehavior_Cave,
    TileBehavior_Ghost0,
    TileBehavior_Ghost1,
    TileBehavior_Ghost2,
    TileBehavior_Ghost3,
    TileBehavior_Ghost4,
    TileBehavior_Ghost5,
    TileBehavior_Ghost6,
    TileBehavior_Ghost7,
    TileBehavior_Ghost8,
    TileBehavior_Ghost9,
    TileBehavior_GhostA,
    TileBehavior_GhostB,
    TileBehavior_GhostC,
    TileBehavior_GhostD,
    TileBehavior_GhostE,
    TileBehavior_GhostF,
    TileBehavior_Armos0,
    TileBehavior_Armos1,
    TileBehavior_Armos2,
    TileBehavior_Armos3,
    TileBehavior_Armos4,
    TileBehavior_Armos5,
    TileBehavior_Armos6,
    TileBehavior_Armos7,
    TileBehavior_Armos8,
    TileBehavior_Armos9,
    TileBehavior_ArmosA,
    TileBehavior_ArmosB,
    TileBehavior_ArmosC,
    TileBehavior_ArmosD,
    TileBehavior_ArmosE,
    TileBehavior_ArmosF,
    TileBehavior_Wall,

    TileBehavior_Max,

    TileBehavior_FirstWalkable = TileBehavior_GenericWalkable,
    TileBehavior_FirstSolid = TileBehavior_Water,
};
