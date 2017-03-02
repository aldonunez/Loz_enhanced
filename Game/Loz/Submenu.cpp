/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Submenu.h"
#include "Graphics.h"
#include "Input.h"
#include "ItemObj.h"
#include "PlayerItemsAnim.h"
#include "Profile.h"
#include "Sound.h"
#include "SoundId.h"
#include "World.h"


enum
{
    ArrowBowUISlot      = 2,

    CurItemX            = 0x40,
    CurItemY            = 0x28,

    PassiveItemX        = 0x80,
    PassiveItemY        = 0x10,

    ActiveItemX         = 0x80,
    ActiveItemY         = 0x28,
    ActiveItemStrideX   = 0x18,
    ActiveItemStrideY   = 0x10,

    ActiveMapX          = 0x80,
    ActiveMapY          = 0x58,
};


struct PassiveItemSpec
{
    uint8_t ItemSlot;
    uint8_t X;
};

struct TileInst
{
    uint8_t Id;
    uint8_t X;
    uint8_t Y;
    uint8_t Palette;
};

static const uint8_t equippedUISlots[] = 
{
    0,          // Sword
    1,          // Bombs
    2,          // Arrow
    0,          // Bow
    3,          // Candle
    4,          // Recorder
    5,          // Food
    6,          // Potion
    7,          // Rod
    0,          // Raft
    0,          // Book
    0,          // Ring
    0,          // Ladder
    0,          // MagicKey
    0,          // Bracelet
    6,          // Letter
    0,          // Compass
    0,          // Map
    0,          // Compass9
    0,          // Map9
    0,          // Clock
    0,          // Rupees
    0,          // Keys
    0,          // HeartContainers
    0,          // PartialHeart
    0,          // TriforcePieces
    0,          // PowerTriforce
    0,          // Boomerang
};

static const TileInst uiTiles[] = 
{
    // INVENTORY
    { 0x12, 0x20, 0x10, 1 },
    { 0x17, 0x28, 0x10, 1 },
    { 0x1F, 0x30, 0x10, 1 },
    { 0x0E, 0x38, 0x10, 1 },
    { 0x17, 0x40, 0x10, 1 },
    { 0x1D, 0x48, 0x10, 1 },
    { 0x18, 0x50, 0x10, 1 },
    { 0x1B, 0x58, 0x10, 1 },
    { 0x22, 0x60, 0x10, 1 },

    // Item B Box
    { 0x69, 0x38, 0x20, 0 },
    { 0x6A, 0x40, 0x20, 0 },
    { 0x6A, 0x48, 0x20, 0 },
    { 0x6B, 0x50, 0x20, 0 },
    { 0x6C, 0x38, 0x28, 0 },
    { 0x6C, 0x50, 0x28, 0 },
    { 0x6C, 0x38, 0x30, 0 },
    { 0x6C, 0x50, 0x30, 0 },
    { 0x6E, 0x38, 0x38, 0 },
    { 0x6A, 0x40, 0x38, 0 },
    { 0x6A, 0x48, 0x38, 0 },
    { 0x6D, 0x50, 0x38, 0 },

    // USE B BUTTON FOR THIS
    { 0x1E, 0x10, 0x40, 0 },
    { 0x1C, 0x18, 0x40, 0 },
    { 0x0E, 0x20, 0x40, 0 },

    { 0x0B, 0x30, 0x40, 0 },

    { 0x0B, 0x40, 0x40, 0 },
    { 0x1E, 0x48, 0x40, 0 },
    { 0x1D, 0x50, 0x40, 0 },
    { 0x1D, 0x58, 0x40, 0 },
    { 0x18, 0x60, 0x40, 0 },
    { 0x17, 0x68, 0x40, 0 },

    { 0x0F, 0x20, 0x48, 0 },
    { 0x18, 0x28, 0x48, 0 },
    { 0x1B, 0x30, 0x48, 0 },

    { 0x1D, 0x40, 0x48, 0 },
    { 0x11, 0x48, 0x48, 0 },
    { 0x12, 0x50, 0x48, 0 },
    { 0x1C, 0x58, 0x48, 0 },

    // Inventory Box
    { 0x69, 0x78, 0x20, 0 },
    { 0x6A, 0x80, 0x20, 0 },
    { 0x6A, 0x88, 0x20, 0 },
    { 0x6A, 0x90, 0x20, 0 },
    { 0x6A, 0x98, 0x20, 0 },
    { 0x6A, 0xA0, 0x20, 0 },
    { 0x6A, 0xA8, 0x20, 0 },
    { 0x6A, 0xB0, 0x20, 0 },
    { 0x6A, 0xB8, 0x20, 0 },

    { 0x6A, 0xC0, 0x20, 0 },
    { 0x6A, 0xC8, 0x20, 0 },
    { 0x6A, 0xD0, 0x20, 0 },

    { 0x6B, 0xD8, 0x20, 0 },

    { 0x6C, 0x78, 0x28, 0 },
    { 0x6C, 0xD8, 0x28, 0 },

    { 0x6C, 0x78, 0x30, 0 },
    { 0x6C, 0xD8, 0x30, 0 },

    { 0x6C, 0x78, 0x38, 0 },
    { 0x6C, 0xD8, 0x38, 0 },

    { 0x6C, 0x78, 0x40, 0 },
    { 0x6C, 0xD8, 0x40, 0 },

    //---
    { 0x6E, 0x78, 0x48, 0 },
    { 0x6A, 0x80, 0x48, 0 },
    { 0x6A, 0x88, 0x48, 0 },
    { 0x6A, 0x90, 0x48, 0 },
    { 0x6A, 0x98, 0x48, 0 },
    { 0x6A, 0xA0, 0x48, 0 },
    { 0x6A, 0xA8, 0x48, 0 },
    { 0x6A, 0xB0, 0x48, 0 },
    { 0x6A, 0xB8, 0x48, 0 },

    { 0x6A, 0xC0, 0x48, 0 },
    { 0x6A, 0xC8, 0x48, 0 },
    { 0x6A, 0xD0, 0x48, 0 },

    { 0x6D, 0xD8, 0x48, 0 },
};

static const PassiveItemSpec passiveItems[] = 
{
    { ItemSlot_Raft,     PassiveItemX },
    { ItemSlot_Book,     PassiveItemX + 0x18 },
    { ItemSlot_Ring,     PassiveItemX + 0x24 },
    { ItemSlot_Ladder,   PassiveItemX + 0x30 },
    { ItemSlot_MagicKey, PassiveItemX + 0x44 },
    { ItemSlot_Bracelet, PassiveItemX + 0x50 },
};


Submenu::Submenu()
    :   enabled( false ),
        activated( false ),
        activeUISlot( 0 )
{
}

static int GetItemIdForUISlot( int uiSlot, int& itemSlot )
{
    static int slots[Submenu::ActiveItems] = 
    {
        ItemSlot_Boomerang,
        ItemSlot_Bombs,
        ItemSlot_Arrow,
        ItemSlot_Candle,

        ItemSlot_Recorder,
        ItemSlot_Food,
        ItemSlot_Letter,
        ItemSlot_Rod,
    };

    Profile&    profile = World::GetProfile();

    itemSlot = slots[uiSlot];

    if ( itemSlot == ItemSlot_Arrow )
    {
        if (   profile.Items[ItemSlot_Arrow] != 0 
            && profile.Items[ItemSlot_Bow] != 0 )
        {
            int arrowId = ItemValueToItemId( ItemSlot_Arrow );
            return arrowId;
        }
    }
    else
    {
        if ( itemSlot == ItemSlot_Letter )
        {
            if ( profile.Items[ItemSlot_Potion] != 0 )
                itemSlot = ItemSlot_Potion;
        }

        int itemValue = profile.Items[itemSlot];
        if ( itemValue != 0 )
        {
            int itemId = ItemValueToItemId( itemSlot, itemValue );
            return itemId;
        }
    }

    itemSlot = 0;
    return Item_None;
}

void Submenu::Enable()
{
    for ( int i = 0; i < ActiveItems; i++ )
    {
        activeItems[i] = GetItemIdForUISlot( i, activeSlots[i] );
    }

    cursor.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Cursor );

    Profile& profile = World::GetProfile();

    activeUISlot = equippedUISlots[profile.SelectedItem];
    enabled = true;
}

void Submenu::Disable()
{
    enabled = false;
}

void Submenu::Activate()
{
    activated = true;
}

void Submenu::Deactivate()
{
    activated = false;
}

void Submenu::Update()
{
    if ( !activated )
        return;

    int dir = 0;

    if ( Input::IsButtonPressing( InputButtons::Left ) )
        dir = -1;
    else if ( Input::IsButtonPressing( InputButtons::Right ) )
        dir = 1;
    else
        return;

    Sound::PlayEffect( SEffect_cursor );

    for ( int i = 0; i < ActiveItems; i++ )
    {
        activeUISlot += dir;

        if ( activeUISlot < 0 )
            activeUISlot += ActiveItems;
        else if ( activeUISlot >= ActiveItems )
            activeUISlot -= ActiveItems;

        if ( activeItems[activeUISlot] != Item_None )
            break;
    }

    Profile& profile = World::GetProfile();

    profile.SelectedItem = activeSlots[activeUISlot];
}

void Submenu::Draw( int bottom )
{
    if ( !enabled )
        return;

    int top = bottom - Height;

    Graphics::SetClip( 0, 0, Width, bottom );

    DrawBackground( top );
    DrawPassiveInventory( top );
    DrawActiveInventory( top );
    DrawCurrentSelection( top );

    if ( World::IsOverworld() )
        DrawTriforce( top );
    else
        DrawMap( top );

    Graphics::ResetClip();
}

void Submenu::DrawBackground( int top )
{
    al_clear_to_color( al_map_rgb( 0, 0, 0 ) );

    for ( int i = 0; i < _countof( uiTiles ); i++ )
    {
        const TileInst& tileInst = uiTiles[i];
        DrawChar( tileInst.Id, tileInst.X, tileInst.Y + top, tileInst.Palette );
    }
}

void Submenu::DrawActiveInventory( int top )
{
    Profile& profile = World::GetProfile();
    int x = ActiveItemX;
    int y = ActiveItemY + top;

    for ( int i = 0; i < ActiveItems; i++ )
    {
        int itemSlot = activeSlots[i];
        int itemId = activeItems[i];

        if ( i == ArrowBowUISlot )
        {
            if ( profile.Items[ItemSlot_Arrow] != 0 )
            {
                itemId = ItemValueToItemId( ItemSlot_Arrow );
                DrawItemNarrow( itemId, x, y );
            }
            if ( profile.Items[ItemSlot_Bow] != 0 )
                DrawItemNarrow( Item_Bow, x + 8, y );
        }
        else if ( itemId != Item_None )
        {
            DrawItemWide( itemId, x, y );
        }

        x += ActiveItemStrideX;
        if ( (i % 4) == 3 )
        {
            x = ActiveItemX;
            y += ActiveItemStrideY;
        }
    }

    x = ActiveItemX + (activeUISlot % 4) * ActiveItemStrideX;
    y = ActiveItemY + (activeUISlot / 4) * ActiveItemStrideY + top;

    int cursorPals[] = { BlueFgPalette, RedFgPalette };
    int cursorPal = cursorPals[ (GetFrameCounter() >> 3) & 1 ];
    cursor.Draw( Sheet_PlayerAndItems, x, y, cursorPal );
}

void Submenu::DrawPassiveInventory( int top )
{
    Profile& profile = World::GetProfile();

    for ( int i = 0; i < PassiveItems; i++ )
    {
        int slot = passiveItems[i].ItemSlot;
        int value = profile.Items[slot];

        if ( value != 0 )
        {
            int itemId = ItemValueToItemId( slot, value );
            DrawItem( itemId, passiveItems[i].X, PassiveItemY + top, 0 );
        }
    }
}

void Submenu::DrawCurrentSelection( int top )
{
    Profile& profile = World::GetProfile();
    int curSlot = profile.SelectedItem;

    if ( curSlot != 0 )
    {
        int itemId = ItemValueToItemId( curSlot );
        DrawItemWide( itemId, CurItemX, CurItemY + top );
    }
}

struct TriforcePieceSpec
{
    uint8_t X;
    uint8_t Y;
    uint8_t OffTiles[2][2];
    uint8_t OnTiles[2][2];
};

void Submenu::DrawTriforce( int top )
{
    static const uint8_t Triforce[] = { 0x1D, 0x1B, 0x12, 0x0F, 0x18, 0x1B, 0x0C, 0x0E };

    static const TriforcePieceSpec pieceSpecs[] = 
    {
        { 0x70, 0x70, { { 0xED, 0xE9 }, { 0xE9, 0x24 } }, { { 0xED, 0xE7 }, { 0xE7, 0xF5 } } },
        { 0x80, 0x70, { { 0xEA, 0xEE }, { 0x24, 0xEA } }, { { 0xE8, 0xEE }, { 0xF5, 0xE8 } } },
        { 0x60, 0x80, { { 0xED, 0xE9 }, { 0xE9, 0x24 } }, { { 0xED, 0xE7 }, { 0xE7, 0xF5 } } },
        { 0x90, 0x80, { { 0xEA, 0xEE }, { 0x24, 0xEA } }, { { 0xE8, 0xEE }, { 0xF5, 0xE8 } } },
        { 0x70, 0x80, { { 0x24, 0x24 }, { 0x24, 0x24 } }, { { 0xE5, 0xF5 }, { 0x24, 0xE5 } } },
        { 0x70, 0x80, { { 0x24, 0x24 }, { 0x24, 0x24 } }, { { 0xE8, 0x24 }, { 0xF5, 0xE8 } } },
        { 0x80, 0x80, { { 0x24, 0x24 }, { 0x24, 0x24 } }, { { 0xF5, 0xE6 }, { 0xE6, 0x24 } } },
        { 0x80, 0x80, { { 0x24, 0x24 }, { 0x24, 0x24 } }, { { 0x24, 0xE7 }, { 0xE7, 0xF5 } } },
    };

    DrawChar( 0xED, 0x78, 0x68 + top, 1 );
    DrawChar( 0xEE, 0x80, 0x68 + top, 1 );

    DrawChar( 0xED, 0x68, 0x78 + top, 1 );
    DrawChar( 0xEE, 0x90, 0x78 + top, 1 );

    DrawChar( 0xED, 0x58, 0x88 + top, 1 );
    DrawChar( 0xEE, 0xA0, 0x88 + top, 1 );

    DrawChar( 0xEB, 0x50, 0x90 + top, 1 );
    DrawChar( 0xEF, 0x58, 0x90 + top, 1 );
    DrawChar( 0xF0, 0xA0, 0x90 + top, 1 );
    DrawChar( 0xEC, 0xA8, 0x90 + top, 1 );

    DrawString( Triforce, _countof( Triforce ), 0x60, 0xA0 + top, 1 );

    int x = 0x60;
    for ( int i = 0; i < 8; i++, x += 8 )
    {
        DrawChar( 0xF1, x, 0x90 + top, 1 );
    }

    uint pieces = World::GetItem( ItemSlot_TriforcePieces );
    uint piece = pieces;

    for ( int i = 0; i < 8; i++, piece >>= 1 )
    {
        const uint8_t* tiles = nullptr;
        uint have = piece & 1;

        if ( have )
        {
            tiles = (uint8_t*) pieceSpecs[i].OnTiles;
        }
        else
        {
            tiles = (uint8_t*) pieceSpecs[i].OffTiles;
        }

        for ( int r = 0; r < 2; r++ )
        {
            for ( int c = 0; c < 2; c++, tiles++ )
            {
                int x = pieceSpecs[i].X + (c * 8);
                int y = pieceSpecs[i].Y + (r * 8) + top;
                DrawChar( *tiles, x, y, 1 );
            }
        }
    }

    if ( (pieces & 0x30) == 0x30 )
    {
        DrawChar( 0xF5, 0x70, 0x80 + top, 1 );
        DrawChar( 0xF5, 0x78, 0x88 + top, 1 );
    }

    if ( (pieces & 0xC0) == 0xC0 )
    {
        DrawChar( 0xF5, 0x88, 0x80 + top, 1 );
        DrawChar( 0xF5, 0x80, 0x88 + top, 1 );
    }
}

void Submenu::DrawMap( int top )
{
    static const uint8_t Map[] = { 0x16, 0x0A, 0x19 };
    static const uint8_t Compass[] = { 0x0C, 0x18, 0x16, 0x19, 0x0A, 0x1C, 0x1C };
    static const uint8_t TopMapLine[] = 
    { 0xF5, 0xF5, 0xFD, 0xF5, 0xF5, 0xFD, 0xF5, 0xF5, 0xFD, 0xF5, 0xF5, 0xF5, 0xFD, 0xF5, 0xF5, 0xF5 };
    static const uint8_t BottomMapLine[] = 
    { 0xF5, 0xFE, 0xF5, 0xF5, 0xF5, 0xFE, 0xF5, 0xF5, 0xF5, 0xF5, 0xFE, 0xF5, 0xF5, 0xF5, 0xFE, 0xF5 };

    DrawString( Map, _countof( Map ), 0x28, 0x58 + top, 1 );
    DrawString( Compass, _countof( Compass ), 0x18, 0x80 + top, 1 );
    DrawString( TopMapLine, _countof( TopMapLine ), 0x60, 0x50 + top, 1 );
    DrawString( BottomMapLine, _countof( BottomMapLine ), 0x60, 0x98 + top, 1 );

    int y = 0x58 + top;
    for ( int r = 0; r < 8; r++, y += 8 )
    {
        int x = 0;
        for ( int c = 0; c < 4; c++, x += 8 )
        {
            DrawChar( 0xF5, 0x60 + x, y, 1 );
            DrawChar( 0xF5, 0xC0 + x, y, 1 );
        }
    }

    const World::LevelInfoBlock* levelInfo = World::GetLevelInfo();
    bool hasMap = World::HasCurrentMap();
    bool hasCompass = World::HasCurrentCompass();

    if ( hasMap )
        DrawItemNarrow( Item_Map, 0x30, 0x68 + top );
    if ( hasCompass )
        DrawItemNarrow( Item_Compass, 0x30, 0x90 + top );

    int x = ActiveMapX;
    for ( int c = 0; c < 8; c++, x += 8 )
    {
        uint mapMaskByte = levelInfo->DrawnMap[c + 4];
        y = ActiveMapY + top;
        for ( int r = 0; r < 8; r++, y += 8, mapMaskByte <<= 1 )
        {
            int roomId = (r << 4) | (c - levelInfo->DrawnMapOffset + 0x10 + 4) & 0xF;
            UWRoomFlags& roomFlags = World::GetUWRoomFlags( roomId );
            uint8_t tile = 0xF5;
            if ( (mapMaskByte & 0x80) == 0x80 && roomFlags.GetVisitState() )
            {
                uint doorSum = 0;
                uint doorBit = 8;
                for ( ; doorBit != 0; doorBit >>= 1 )
                {
                    DoorType doorType = World::GetDoorType( roomId, (Direction) doorBit );
                    if ( doorType == DoorType_Open )
                        doorSum |= doorBit;
                    else if ( roomFlags.GetDoorState( doorBit )
                        && (doorType == DoorType_Bombable 
                        ||  doorType == DoorType_Key 
                        ||  doorType == DoorType_Key2) )
                        doorSum |= doorBit;
                }
                tile = 0xD0 + doorSum;
            }
            DrawChar( tile, x, y, 1 );
        }
    }

    int curRoomId = World::GetRoomId();
    int playerRow = (curRoomId >> 4) & 0xF;
    int playerCol = curRoomId & 0xF;
    playerCol = (playerCol + levelInfo->DrawnMapOffset) & 0xF;
    playerCol -= 4;

    y = ActiveMapY + top + playerRow * 8 + 3;
    x = ActiveMapX + playerCol * 8 + 2;
    DrawChar( 0xE0, x, y, PlayerPalette );
}
