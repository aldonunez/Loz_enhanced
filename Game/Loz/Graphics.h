/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum TileSheetSlot
{
    Sheet_Background,
    Sheet_PlayerAndItems,
    Sheet_Npcs,
    Sheet_Boss,
    Sheet_Font,

    Sheet_Max
};


struct SpriteFrame
{
    uint8_t     x;
    uint8_t     y;
    uint8_t     flags;
};

struct SpriteAnim
{
    uint8_t     length;
    uint8_t     width;
    uint8_t     height;
    SpriteFrame frames[1];
};


class Graphics
{
public:
    static bool Init();

    static void LoadTileSheet( int slot, const char* path );
    static void LoadTileSheet( int slot, const char* imagePath, const char* animPath );

    static void Begin();
    static void End();
    static void DrawSpriteTile( 
        int slot, 
        int srcX, 
        int srcY, 
        int width, 
        int height, 
        int destX, 
        int destY, 
        int palette,
        int flags
        );
    static void DrawTile( 
        int slot, 
        int srcX, 
        int srcY, 
        int width, 
        int height, 
        int destX, 
        int destY, 
        int palette,
        int flags
        );
    static void DrawBitmap(
        ALLEGRO_BITMAP* bitmap,
        int srcX, 
        int srcY,
        int width,
        int height,
        int destX,
        int destY,
        int palette,
        int flags
        );
    static void DrawStripSprite16x16(
        int slot,
        int firstTile,
        int destX,
        int destY,
        int palette
        );

    static void LoadSystemPalette( const int* colorsArgb8 );
    static ALLEGRO_COLOR GetSystemColor( int sysColor );
    static void SetColor( int paletteIndex, int colorIndex, int colorArgb8 );
    static void SetColorIndexed( int paletteIndex, int colorIndex, int sysColor );
    static void SetPaletteIndexed( int paletteIndex, const uint8_t* sysColors );
    static void UpdatePalettes();

    static void EnableGrayscale();
    static void DisableGrayscale();

    static void SetViewParams( float scale, float x, float y );
    static void SetClip( int x, int y, int width, int height );
    static void ResetClip();

    static const SpriteAnim* GetAnimation( int slot, int animIndex );

private:
    static void SetPalette( int paletteIndex, const int* colorsArgb8 );
    static void SwitchSystemPalette( int* newSystemPalette );
};
