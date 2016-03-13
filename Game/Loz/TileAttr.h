/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum TileAction
{
    TileAction_None,
    TileAction_Push,
    TileAction_Bomb,
    TileAction_Burn,
    TileAction_Headstone,
    TileAction_Ladder,
    TileAction_Raft,
    TileAction_Cave,
    TileAction_Stairs,
    TileAction_Ghost,
    TileAction_Armos,
    TileAction_Block,
};


class TileAttr
{
public:
    static int GetAction( uint8_t t )
    {
        return (t & 0xF0) >> 4;
    }

    static bool IsQuadrantBlocked( uint8_t t, int row, int col )
    {
        uint8_t walkBit = 1;
        walkBit <<= col * 2;
        walkBit <<= row;
        return t & walkBit;
    }

    static bool IsTileBlocked( uint8_t t )
    {
        return (t & 0x0F) != 0;
    }
};
