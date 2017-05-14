/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Graphics.h"


// Y determines the palette, X determines the color in the palette.
// But, it looks like the minimum height of a bitmap is 16.
const int PaletteBmpWidth = Util::Max( PaletteLength, 16 );
const int PaletteBmpHeight = Util::Max( PaletteCount, 16 );

enum
{
    TileWidth   = 8,
    TileHeight  = 8,
};


ALLEGRO_BITMAP* tileSheets[Sheet_Max];
ALLEGRO_BITMAP* paletteBmp;
ALLEGRO_SHADER* tileShader;
unsigned char*  paletteBuf;
int             paletteBufSize;
int             paletteStride;
int             systemPalette[SysPaletteLength];
int             grayscalePalette[SysPaletteLength];
int*            activeSystemPalette = systemPalette;
uint8_t         palettes[PaletteCount][PaletteLength];

float           viewScale;
float           viewOffsetX;
float           viewOffsetY;

int             savedClipX;
int             savedClipY;
int             savedClipWidth;
int             savedClipHeight;

Util::Table<SpriteAnim> animSpecs[Sheet_Max];


static bool ChooseShaderSource( 
    ALLEGRO_SHADER* shader, 
    const char** vsource, 
    const char** psource )
{
    ALLEGRO_SHADER_PLATFORM platform = al_get_shader_platform( shader );
    if ( platform == ALLEGRO_SHADER_HLSL )
    {
        *vsource = "tileShaderVertex.hlsl";
        *psource = "tileShaderPixel.hlsl";
    }
    else if ( platform == ALLEGRO_SHADER_GLSL )
    {
        *vsource = "tileShaderVertex.glsl";
        *psource = "tileShaderPixel.glsl";
    }
    else
    {
        *vsource = nullptr;
        *psource = nullptr;
        return false;
    }

   return true;
}

static bool AllocatePaletteBuffer()
{
    int format = al_get_bitmap_format( paletteBmp );
    ALLEGRO_LOCKED_REGION*  region = al_lock_bitmap( paletteBmp, format, ALLEGRO_LOCK_WRITEONLY );
    if ( region == nullptr )
        return false;
    int stride = region->pitch;
    al_unlock_bitmap( paletteBmp );

    if ( stride < 0 )
        stride = -stride;

    paletteBufSize = stride * PaletteBmpHeight;
    paletteStride = stride;

    paletteBuf = new unsigned char[paletteBufSize];
    if ( paletteBuf == nullptr )
        return false;

    memset( paletteBuf, 0, paletteBufSize );
    return true;
}

bool Graphics::Init()
{
    const char* vsource = nullptr;
    const char* psource = nullptr;

    paletteBmp = al_create_bitmap( PaletteBmpWidth, PaletteBmpHeight );
    if ( paletteBmp == nullptr )
        return false;

    tileShader = al_create_shader( ALLEGRO_SHADER_AUTO );
    if ( tileShader == nullptr )
        return false;

    if ( !ChooseShaderSource( tileShader, &vsource, &psource ) )
        return false;

    if ( !al_attach_shader_source_file( tileShader, ALLEGRO_VERTEX_SHADER, vsource ) )
    {
        _RPT1( _CRT_WARN, "%s", al_get_shader_log( tileShader ) );
        return false;
    }
    if ( !al_attach_shader_source_file( tileShader, ALLEGRO_PIXEL_SHADER, psource ) )
    {
        _RPT1( _CRT_WARN, "%s", al_get_shader_log( tileShader ) );
        return false;
    }
    if ( !al_build_shader( tileShader ) )
    {
        _RPT1( _CRT_WARN, "%s", al_get_shader_log( tileShader ) );
        return false;
    }

    if ( !al_use_shader( tileShader ) )
        return false;

    if ( !AllocatePaletteBuffer() )
        return false;

    return true;
}

void Graphics::LoadTileSheet( int slot, const char* path )
{
    if ( tileSheets[slot] != nullptr )
    {
        al_destroy_bitmap( tileSheets[slot] );
        tileSheets[slot] = nullptr;
    }

    tileSheets[slot] = al_load_bitmap( path );
    assert( tileSheets[slot] != nullptr );

    if ( tileSheets[slot] == nullptr )
    {
        tileSheets[slot] = al_create_bitmap( 1, 1 );
    }
}

void Graphics::LoadTileSheet( int slot, const char* imagePath, const char* animPath )
{
    LoadTileSheet( slot, imagePath );
    Util::LoadResource( animPath, &animSpecs[slot] );
}

const SpriteAnim* Graphics::GetAnimation( int slot, int animIndex )
{
    return animSpecs[slot].GetItem( animIndex );
}

void Graphics::LoadSystemPalette( const int* colorsArgb8 )
{
    memcpy( systemPalette, colorsArgb8, sizeof systemPalette );

    for ( int i = 0; i < SysPaletteLength; i++ )
    {
        grayscalePalette[i] = systemPalette[i & 0x30];
    }
}

ALLEGRO_COLOR Graphics::GetSystemColor( int sysColor )
{
    int argb8 = activeSystemPalette[sysColor];
    return al_map_rgba( 
        (argb8 >> 16) & 0xFF,
        (argb8 >>  8) & 0xFF,
        (argb8 >>  0) & 0xFF,
        (argb8 >> 24) & 0xFF
        );
}

// TODO: this method has to consider the picture format
void Graphics::SetColor( int paletteIndex, int colorIndex, int colorArgb8 )
{
    int y = paletteIndex;
    int x = colorIndex;

    unsigned char* line = paletteBuf + y * paletteStride;
    ((int*) line)[x] = colorArgb8;
}

void Graphics::SetPalette( int paletteIndex, const int* colorsArgb8 )
{
    int y = paletteIndex;
    unsigned char* line = paletteBuf + y * paletteStride;

    for ( int x = 0; x < PaletteLength; x++ )
    {
        ((int*) line)[x] = colorsArgb8[x];
    }
}

void Graphics::SetColorIndexed( int paletteIndex, int colorIndex, int sysColor )
{
    int colorArgb8 = 0;
    if ( colorIndex != 0 )
        colorArgb8 = activeSystemPalette[sysColor];
    SetColor( paletteIndex, colorIndex, colorArgb8 );
    palettes[paletteIndex][colorIndex] = sysColor;
}

void Graphics::SetPaletteIndexed( int paletteIndex, const uint8_t* sysColors )
{
    int colorsArgb8[4] = 
    {
        0,
        activeSystemPalette[sysColors[1]],
        activeSystemPalette[sysColors[2]],
        activeSystemPalette[sysColors[3]],
    };
    SetPalette( paletteIndex, colorsArgb8 );
    memcpy( palettes[paletteIndex], sysColors, PaletteLength );
}

void Graphics::UpdatePalettes()
{
    int format = al_get_bitmap_format( paletteBmp );
    ALLEGRO_LOCKED_REGION*  region = al_lock_bitmap( paletteBmp, format, ALLEGRO_LOCK_WRITEONLY );
    assert( region != nullptr );

    unsigned char* base = (unsigned char*) region->data;
    if ( region->pitch < 0 )
        base += region->pitch * (PaletteBmpHeight - 1);

    memcpy( base, paletteBuf, paletteBufSize );

    al_unlock_bitmap( paletteBmp );
}

void Graphics::SwitchSystemPalette( int* newSystemPalette )
{
    if ( activeSystemPalette == newSystemPalette )
        return;

    activeSystemPalette = newSystemPalette;

    for ( int i = 0; i < PaletteCount; i++ )
    {
        const uint8_t* sysColors = palettes[i];
        int colorsArgb8[4] = 
        {
            0,
            activeSystemPalette[sysColors[1]],
            activeSystemPalette[sysColors[2]],
            activeSystemPalette[sysColors[3]],
        };
        SetPalette( i, colorsArgb8 );
    }
    UpdatePalettes();
}

void Graphics::EnableGrayscale()
{
    SwitchSystemPalette( grayscalePalette );
}

void Graphics::DisableGrayscale()
{
    SwitchSystemPalette( systemPalette );
}

void Graphics::Begin()
{
    bool bRet;

    bRet = al_set_shader_sampler( "palTex", paletteBmp, 1 );
    assert( bRet );

    al_hold_bitmap_drawing( true );
}

void Graphics::End()
{
    al_hold_bitmap_drawing( false );
}

void Graphics::DrawBitmap(
    ALLEGRO_BITMAP* bitmap,
    int srcX, 
    int srcY,
    int width,
    int height,
    int destX,
    int destY,
    int palette,
    int flags
    )
{
    float palRed = palette / (float) PaletteBmpHeight;
    ALLEGRO_COLOR tint = al_map_rgba_f( palRed, 0, 0, 1 );

    al_draw_tinted_bitmap_region(
        bitmap,
        tint,
        srcX,
        srcY,
        width,
        height,
        destX,
        destY,
        flags );
}

void Graphics::DrawSpriteTile( 
    int slot, 
    int srcX, 
    int srcY, 
    int width, 
    int height, 
    int destX, 
    int destY, 
    int palette,
    int flags
    )
{
    DrawTile(
        slot,
        srcX,
        srcY,
        width,
        height,
        destX,
        destY + 1,
        palette,
        flags );
}

void Graphics::DrawTile( 
    int slot, 
    int srcX, 
    int srcY, 
    int width, 
    int height, 
    int destX, 
    int destY, 
    int palette,
    int flags
    )
{
    assert( slot < Sheet_Max );

    float palRed = palette / (float) PaletteBmpHeight;
    ALLEGRO_COLOR tint = al_map_rgba_f( palRed, 0, 0, 1 );

    al_draw_tinted_bitmap_region(
        tileSheets[slot],
        tint,
        srcX,
        srcY,
        width,
        height,
        destX,
        destY,
        flags );
}

void Graphics::DrawStripSprite16x16(
    int slot,
    int firstTile,
    int destX,
    int destY,
    int palette
)
{
    static const uint8_t offsetsX[4] = { 0, 0, 8, 8 };
    static const uint8_t offsetsY[4] = { 0, 8, 0, 8 };
    int tileRef = firstTile;

    for ( int i = 0; i < 4; i++ )
    {
        int srcX = (tileRef & 0x0F) * TileWidth;
        int srcY = ((tileRef & 0xF0) >> 4) * TileHeight;
        tileRef++;

        DrawTile(
            slot,
            srcX,
            srcY,
            TileWidth,
            TileHeight,
            destX + offsetsX[i],
            destY + offsetsY[i],
            palette,
            0 );
    }
}

void Graphics::SetViewParams( float scale, float x, float y )
{
    viewScale = scale;
    viewOffsetX = x;
    viewOffsetY = y;
}

void Graphics::SetClip( int x, int y, int width, int height )
{
    al_get_clipping_rectangle( 
        &savedClipX,
        &savedClipY,
        &savedClipWidth,
        &savedClipHeight );

    int y2 = y + height;

    if ( y2 < 0 )
    {
        height = 0;
        y = 0;
    }
    else if ( y > StdViewHeight )
    {
        height = 0;
        y = StdViewHeight;
    }
    else
    {
        if ( y < 0 )
        {
            height += y;
            y = 0;
        }

        if ( y2 > StdViewHeight )
        {
            height = StdViewHeight - y;
        }
    }

    int clipX = viewOffsetX + x * viewScale;
    int clipY = viewOffsetY + y * viewScale;
    int clipWidth = width * viewScale;
    int clipHeight = height * viewScale;

    al_set_clipping_rectangle( 
        clipX, 
        clipY, 
        clipWidth, 
        clipHeight );
}

void Graphics::ResetClip()
{
    al_set_clipping_rectangle( 
        savedClipX,
        savedClipY,
        savedClipWidth,
        savedClipHeight );
}
