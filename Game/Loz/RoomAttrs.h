/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


struct RoomAttrs
{
    uint8_t UniqueRoomId;
    uint8_t PalettesAndMonsterCount;
    uint8_t MonsterListId;
    uint8_t Specific[4];

    int GetUniqueRoomId()
    {
        return UniqueRoomId & 0x7F;
    }

    int GetOuterPalette()
    {
        return PalettesAndMonsterCount & 0x03;
    }

    int GetInnerPalette()
    {
        return (PalettesAndMonsterCount >> 2) & 0x03;
    }

    int GetMonsterCount()
    {
        return (PalettesAndMonsterCount >> 4) & 0xF;
    }
};

struct OWRoomAttrs : RoomAttrs
{
    RSpot GetExitPosition()
    {
        return Specific[0];
    }

    int GetCaveId()
    {
        return Specific[1] & 0x3F;
    }

    int GetShortcutStairsIndex()
    {
        return (Specific[2] & 0x03);
    }

    bool HasZora()
    {
        return (Specific[2] & 0x04) != 0;
    }

    bool MonstersEnter()
    {
        return (Specific[2] & 0x08) != 0;
    }

    bool HasAmbientSound()
    {
        return (Specific[2] & 0x10) != 0;
    }
};

struct UWRoomAttrs : public RoomAttrs
{
    int GetDoor( int dirOrd )
    {
        switch ( dirOrd )
        {
        case 0: return Specific[1] & 7;
        case 1: return (Specific[1] >> 3) & 7;
        case 2: return Specific[0] & 7;
        case 3: return (Specific[0] >> 3) & 7;
        default: return 1;
        }
    }

    int GetLeftCellarExitRoomId()
    {
        return Specific[0];
    }

    int GetRightCellarExitRoomId()
    {
        return Specific[1];
    }

    int GetItemId()
    {
        int itemId = Specific[2] & 0x1F;
        if ( itemId == 3 )
            itemId = 0x3F;
        return itemId;
    }

    int GetItemPositionIndex()
    {
        return (Specific[2] >> 5) & 3;
    }

    int GetSecret()
    {
        return Specific[3] & 7;
    }

    bool HasBlock()
    {
        return (Specific[3] & 0x08) != 0;
    }

    bool IsDark()
    {
        return (Specific[3] & 0x10) != 0;
    }

    int GetAmbientSound()
    {
        return (Specific[3] >> 5) & 3;
    }
};

