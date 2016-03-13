/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Credits.h"
#include "Graphics.h"
#include "ItemObj.h"
#include "UWBossAnim.h"
#include "World.h"
#include "Profile.h"


struct Line
{
    uint8_t Length;
    uint8_t Col;
    uint8_t Text[1];
};


Credits::Credits()
    :   fraction( 0 ),
        tileOffset( 0 ),
        top( StartY ),
        windowTop( StartY ),
        windowTopLine( 0 ),
        windowBottomLine( 1 ),
        windowFirstMappedLine( 0 ),
        madePlayerLine( false )
{
    Util::LoadResource( "credits.tab", &textTable );
    Util::LoadList( "creditsLinesBmp.dat", lineBmp, AllLineBytes );

    // It's a little cleaner to come up with these two line bitmaps than to use
    // the one from the original game and change line numbers and bit positions on the fly.
    if ( World::GetProfile().Quest == 0 )
    {
        const static uint8_t quest1LineBmp[AllLineBytes] = 
        { 0x46, 0x10, 0x90, 0x84, 0x90, 0xC0, 0x05, 0x20, 0x30 };
        memcpy( lineBmp, quest1LineBmp, sizeof quest1LineBmp );
    }
    else
    {
        const static uint8_t quest2LineBmp[AllLineBytes] = 
        { 0x46, 0x10, 0x90, 0x84, 0x90, 0xC0, 0, 0, 0x12, 0x50, 0x54, 0x00 };
        memcpy( lineBmp, quest2LineBmp, sizeof quest2LineBmp );
    }

    SetPilePalette();
    Graphics::SetColorIndexed( 1, 1, 0x16 );
    Graphics::SetColorIndexed( 2, 1, 0x22 );
    Graphics::SetColorIndexed( 3, 1, 0x2A );
    Graphics::UpdatePalettes();
}

bool Credits::IsDone()
{
    return windowTopLine == GetTopLineAtEnd();
}

int  Credits::GetTop()
{
    return top;
}

int Credits::GetTopLineAtEnd()
{
    if ( World::GetProfile().Quest == 0 )
        return 46;
    else
        return 61;
}

void Credits::Update()
{
    if ( windowTopLine == GetTopLineAtEnd() )
        return;

    fraction++;
    if ( fraction == 2 )
    {
        fraction = 0;
        tileOffset++;
        windowTop--;
        top--;

        if ( tileOffset >= 8 )
        {
            tileOffset -= 8;

            if ( windowTop < 0 )
            {
                windowTop += 8;
                if ( windowTopLine < (AllLines - 1) )
                {
                    int byte = windowTopLine / 8;
                    int bit  = windowTopLine % 8;
                    int show = lineBmp[byte] & (0x80 >> bit);
                    if ( show )
                        windowFirstMappedLine++;
                    windowTopLine++;
                }
            }

            if ( windowBottomLine < AllLines )
                windowBottomLine++;
        }
    }
}

void Credits::MakePlayerLine( const Line* line )
{
    memcpy( playerLine, line->Text, line->Length );
    memcpy( playerLine, World::GetProfile().Name, World::GetProfile().NameLength );
    NumberToStringR( World::GetProfile().Deaths, NumberSign_None, &playerLine[10], 3 );
    madePlayerLine = true;
}

static void DrawHorizWallLine( int x, int y, int length )
{
    for ( int i = 0; i < length; i++ )
    {
        DrawChar( 0xFA, x, y, 0 );
        x += 8;
    }
}

void Credits::Draw()
{
    int mappedLine = windowFirstMappedLine;
    int y = windowTop;

    for ( int i = windowTopLine; i < windowBottomLine; i++ )
    {
        if ( (mappedLine >= (int) textTable.GetLength())
            || (World::GetProfile().Quest == 0 && mappedLine >= 0x10) )
            break;

        int byte = i / 8;
        int bit  = i % 8;
        int show = lineBmp[byte] & (0x80 >> bit);
        int pal = 0;

        if ( i > 1 && i < 44 )
        {
            DrawChar( 0xFA, 24, y, 0 );
            DrawChar( 0xFA, 224, y, 0 );
            pal = ((i + 6) / 7) % 3 + 1;
        }
        else if ( World::GetProfile().Quest == 1 )
        {
            if ( mappedLine == 13 )
                pal = 1;
            else if ( mappedLine == 18 )
                pal = 2;
        }
        if ( show )
        {
            int effMappedLine = mappedLine;
            if ( World::GetProfile().Quest == 1 && mappedLine >= 12 )
                effMappedLine += 4;
            const Line* line = (Line*) textTable.GetItem( effMappedLine );
            const uint8_t* text = line->Text;
            int x = line->Col * 8;
            if ( World::GetProfile().Quest == 1 && mappedLine == 13 )
            {
                if ( !madePlayerLine )
                    MakePlayerLine( line );
                text = playerLine;
            }
            DrawString( text, line->Length, x, y, pal );
            mappedLine++;
        }
        if ( i == 1 )
        {
            DrawHorizWallLine( 24, y, 10 );
            DrawHorizWallLine( 160, y, 9 );
        }
        else if ( i == 44 )
        {
            DrawHorizWallLine( 24, y, 26 );
        }
        y += 8;
    }

    if ( IsDone() )
    {
        y = 0x80;
        DrawItem( Item_PowerTriforce, 0x78, y, 0 );

        SpriteImage pile;
        pile.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B3_Pile );
        pile.Draw( Sheet_Boss, 0x78, y + 0, 7 );
    }
}
