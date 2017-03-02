/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "StatusBar.h"
#include "Graphics.h"
#include "World.h"
#include "ItemObj.h"
#include "Profile.h"


enum
{
    StatusBarWidth = World::TileMapWidth,

    LevelNameX = 16,
    LevelNameY = 16,

    MiniMapX = 16,
    MiniMapY = 24,
    MiniMapColumnOffset = 4,

    OWMapTileWidth  = 4,
    OWMapTileHeight = 4,
    UWMapTileWidth  = 8,
    UWMapTileHeight = 4,

    CountersX       = 0x60,
    EquipmentY      = 0x20,
    HeartsX         = 0xB8,
    HeartsY         = 0x30,

    Tile_FullHeart  = 0xF2,
    Tile_HalfHeart  = 0x65,
    Tile_EmptyHeart = 0x66,
};

struct TileInst
{
    uint8_t Id;
    uint8_t X;
    uint8_t Y;
    uint8_t Palette;
};

static const TileInst uiTiles[] = 
{
    { 0xF7, 88, 24, 1 },
    { 0xF9, 88, 40, 1 },
    { 0x61, 88, 48, 0 },

    // Item A Box
    { 0x69, 120, 24, 0 },
    { 0x0B, 128, 24, 0 },
    { 0x6B, 136, 24, 0 },
    { 0x6C, 120, 32, 0 },
    { 0x6C, 136, 32, 0 },
    { 0x6C, 120, 40, 0 },
    { 0x6C, 136, 40, 0 },
    { 0x6E, 120, 48, 0 },
    { 0x6A, 128, 48, 0 },
    { 0x6D, 136, 48, 0 },

    // Item B Box
    { 0x69, 120+24, 24, 0 },
    { 0x0A, 128+24, 24, 0 },
    { 0x6B, 136+24, 24, 0 },
    { 0x6C, 120+24, 32, 0 },
    { 0x6C, 136+24, 32, 0 },
    { 0x6C, 120+24, 40, 0 },
    { 0x6C, 136+24, 40, 0 },
    { 0x6E, 120+24, 48, 0 },
    { 0x6A, 128+24, 48, 0 },
    { 0x6D, 136+24, 48, 0 },

    // -LIFE-
    { 0x62, 184, 24, 1 },
    { 0x15, 192, 24, 1 },
    { 0x12, 200, 24, 1 },
    { 0x0F, 208, 24, 1 },
    { 0x0E, 216, 24, 1 },
    { 0x62, 224, 24, 1 },
};


StatusBar::StatusBar()
    :   features( Feature_All )
{
}

void StatusBar::EnableFeatures( Features features, bool enable )
{
    if ( enable )
        this->features = (Features) (this->features | features);
    else
        this->features = (Features) (this->features ^ features);
}

void StatusBar::Draw( int baseY )
{
    ALLEGRO_COLOR backColor = al_map_rgb( 0, 0, 0 );
    Draw( baseY, backColor );
}

void StatusBar::Draw( int baseY, ALLEGRO_COLOR backColor )
{
    Graphics::SetClip( 0, baseY, StatusBarWidth, StatusBarHeight );

    al_clear_to_color( backColor );

    for ( int i = 0; i < _countof( uiTiles ); i++ )
    {
        const TileInst& tileInst = uiTiles[i];
        DrawTile( tileInst.Id, tileInst.X, tileInst.Y + baseY, tileInst.Palette );
    }

    DrawMiniMap( baseY );
    DrawItems( baseY );

    Graphics::ResetClip();
}

void StatusBar::DrawMiniMap( int baseY )
{
    int roomId = World::GetRoomId();
    int row = (roomId >> 4) & 0xF;
    int col = roomId & 0xF;
    int cursorX = MiniMapX;
    int cursorY = MiniMapY + baseY;
    bool showCursor = true;

    if ( World::IsOverworld() )
    {
        DrawOWMiniMap( baseY );

        cursorX += col * OWMapTileWidth;
        cursorY += row * OWMapTileHeight;
    }
    else
    {
        uint8_t levelStr[] = { 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x62, 0 };
        const World::LevelInfoBlock* levelInfo = World::GetLevelInfo();

        levelStr[6] = levelInfo->LevelNumber;
        DrawString( levelStr, _countof( levelStr ), LevelNameX, baseY + LevelNameY, 0 );
        DrawUWMiniMap( baseY );

        col = (col + levelInfo->DrawnMapOffset) & 0xF;
        col -= MiniMapColumnOffset;

        cursorX += col * UWMapTileWidth + 2;
        cursorY += row * UWMapTileHeight;

        if ( !World::IsUWMain( roomId ) )
            showCursor = false;

        if ( (features & Feature_MapCursors) != 0
            && World::HasCurrentCompass() )
        {
            int triforceRoomId = levelInfo->TriforceRoomId;
            int triforceRow = (triforceRoomId >> 4) & 0xF;
            int triforceCol = triforceRoomId & 0xF;
            col = (triforceCol + levelInfo->DrawnMapOffset) & 0xF;
            col -= MiniMapColumnOffset;
            int triforceX = MiniMapX + col * UWMapTileWidth + 2;
            int triforceY = MiniMapY + baseY + triforceRow * UWMapTileHeight;
            int palette = LevelFgPalette;

            if ( !World::GotItem( triforceRoomId ) )
            {
                if ( (GetFrameCounter() & 0x10) == 0 )
                    palette = RedFgPalette;
            }

            DrawTile( 0xE0, triforceX, triforceY, palette );
        }
    }

    if ( (features & Feature_MapCursors) != 0 )
    {
        if ( showCursor )
            DrawTile( 0xE0, cursorX, cursorY, PlayerPalette );
    }
}

void StatusBar::DrawTile( int tile, int x, int y, int palette )
{
    DrawChar( tile, x, y, palette );
}

void StatusBar::DrawOWMiniMap( int baseY )
{
    int y = MiniMapY + baseY;

    for ( int i = 0; i < 4; i++ )
    {
        int x = MiniMapX;
        for ( int j = 0; j < 8; j++ )
        {
            DrawTile( 0xF5, x, y, 0 );
            x += 8;
        }
        y += 8;
    }
}

void StatusBar::DrawUWMiniMap( int baseY )
{
    if ( !World::HasCurrentMap() )
        return;

    const World::LevelInfoBlock* levelInfo = World::GetLevelInfo();

    int x = MiniMapX;

    for ( int c = 0; c < 12; c++ )
    {
        int b = levelInfo->DrawnMap[c + MiniMapColumnOffset];
        int y = baseY + MiniMapY;

        for ( int r = 0; r < 8; r++ )
        {
            if ( (b & 0x80) != 0 )
            {
                Graphics::DrawTile( 
                    Sheet_Font,
                    0x7 * 8, 0x6 * 8,
                    8, 4,
                    x, y,
                    0, 0 );
            }
            b <<= 1;
            y += 4;
        }
        x += 8;
    }
}

static void DrawCount( int itemSlot, int x, int y )
{
    uint8_t     charBuf[4];
    int         count = World::GetItem( itemSlot );
    uint8_t*    strLeft = NumberToStringR( count, NumberSign_None, charBuf );

    if ( count < 100 )
    {
        strLeft--;
        *strLeft = Char_X;
    }

    int length = 4 - (strLeft - charBuf);
    DrawString( strLeft, length, x, y, 0 );
}

void StatusBar::DrawItems( int baseY )
{
    if ( (features & Feature_Counters) != 0 )
    {
        if ( World::GetItem( ItemSlot_MagicKey ) != 0 )
        {
            static const uint8_t XA[] = { Char_X, 0x0A };
            DrawString( XA, 2, CountersX, 0x28 + baseY, 0 );
        }
        else
        {
            DrawCount( ItemSlot_Keys, CountersX, 0x28 + baseY );
        }

        DrawCount( ItemSlot_Bombs, CountersX, 0x30 + baseY );
        DrawCount( ItemSlot_Rupees, CountersX, 0x18 + baseY );

        DrawHearts( baseY );
    }

    if ( (features & Feature_Equipment) != 0 )
    {
        DrawSword( baseY );
        DrawItemB( baseY );
    }
}

void StatusBar::DrawSword( int baseY )
{
    int swordValue = World::GetItem( ItemSlot_Sword );
    if ( swordValue == 0 )
        return;

    int itemId = ItemValueToItemId( ItemSlot_Sword, swordValue );

    ::DrawItemNarrow( itemId, 0x98, EquipmentY + baseY );
}

void StatusBar::DrawItemB( int baseY )
{
    Profile& profile = World::GetProfile();

    if ( profile.SelectedItem == 0 )
        return;

    uint8_t itemValue = profile.Items[profile.SelectedItem];

    if ( itemValue == 0 )
        return;

    uint8_t itemId = ItemValueToItemId( profile.SelectedItem, itemValue );

    ::DrawItemNarrow( itemId, 0x80, EquipmentY + baseY );
}

void StatusBar::DrawHearts( int baseY )
{
    unsigned int totalHearts = World::GetItem( ItemSlot_HeartContainers );
    unsigned int heartsValue = World::GetProfile().Hearts;
    int y = HeartsY + baseY;
    ::DrawHearts( heartsValue, totalHearts, HeartsX, y );
}
