/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

enum
{
    MaxNameLength = 8,
    DefaultHearts = 3,
    DefaultBombs  = 8,
};

enum ItemSlot
{
    ItemSlot_Sword,
    ItemSlot_Bombs,
    ItemSlot_Arrow,
    ItemSlot_Bow,
    ItemSlot_Candle,
    ItemSlot_Recorder,
    ItemSlot_Food,
    ItemSlot_Potion,
    ItemSlot_Rod,
    ItemSlot_Raft,
    ItemSlot_Book,
    ItemSlot_Ring,
    ItemSlot_Ladder,
    ItemSlot_MagicKey,
    ItemSlot_Bracelet,
    ItemSlot_Letter,
    ItemSlot_Compass,
    ItemSlot_Map,
    ItemSlot_Compass9,
    ItemSlot_Map9,
    ItemSlot_Clock,
    ItemSlot_Rupees,
    ItemSlot_Keys,
    ItemSlot_HeartContainers,
    ItemSlot_PartialHeart_Unused,
    ItemSlot_TriforcePieces,
    ItemSlot_PowerTriforce,
    ItemSlot_Boomerang,
    ItemSlot_MagicShield,
    ItemSlot_MaxBombs,
    ItemSlot_RupeesToAdd,
    ItemSlot_RupeesToSubtract,

    ItemSlot_MaxItems
};

struct OWRoomFlags
{
    enum
    {
        ItemState       = 0x10,
        ShortcutState   = 0x20,
        SecretState     = 0x80,

        CountMask       = 7,
        CountShift      = 0,
    };

    uint8_t Data;

    bool GetItemState()
    {
        return Data & ItemState;
    }

    void SetItemState()
    {
        Data |= ItemState;
    }

    bool GetShortcutState()
    {
        return Data & ShortcutState;
    }

    void SetShortcutState()
    {
        Data |= ShortcutState;
    }

    bool GetSecretState()
    {
        return Data & SecretState;
    }

    void SetSecretState()
    {
        Data |= SecretState;
    }

    int GetObjCount()
    {
        return (Data & CountMask) >> CountShift;
    }

    void SetObjCount( int count )
    {
        Data = (Data & ~CountMask) | (count << CountShift);
    }
};

struct UWRoomFlags
{
    enum
    {
        ItemState       = 0x10,
        VisitState      = 0x20,

        CountMask       = 0xC0,
        CountShift      = 6,
    };

    uint8_t Data;

    bool GetItemState()
    {
        return Data & ItemState;
    }

    void SetItemState()
    {
        Data |= ItemState;
    }

    bool GetVisitState()
    {
        return Data & VisitState;
    }

    void SetVisitState()
    {
        Data |= VisitState;
    }

    bool GetDoorState( int dir )
    {
        return (Data & dir) != 0;
    }

    void SetDoorState( int dir )
    {
        Data |= dir;
    }

    int GetObjCount()
    {
        return (Data & CountMask) >> CountShift;
    }

    void SetObjCount( int count )
    {
        Data = (Data & ~CountMask) | (count << CountShift);
    }
};


struct Profile
{
    uint8_t     NameLength;
    uint8_t     Name[MaxNameLength];
    uint8_t     Quest;
    uint8_t     Deaths;
    uint8_t     SelectedItem;
    uint16_t    Hearts;
    uint8_t     Items[ItemSlot_MaxItems];
    OWRoomFlags OverworldFlags[LevelBlockRooms];
    UWRoomFlags LevelFlags1[LevelBlockRooms];
    UWRoomFlags LevelFlags2[LevelBlockRooms];

    uint GetMaxHeartsValue();
    static uint GetMaxHeartsValue( uint heartContainers );
};
