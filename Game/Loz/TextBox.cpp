/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "TextBox.h"
#include "ItemObj.h"
#include "Sound.h"
#include "SoundId.h"


TextBox::TextBox( const uint8_t* str, int delay )
    :   left( StartX ),
        top( StartY ),
        height( 8 ),
        charDelay( delay ),
        charTimer( 0 ),
        drawingDialog( true ),
        startCharPtr( str ),
        curCharPtr( str )
{
}

void TextBox::Reset( const uint8_t* str )
{
    drawingDialog = true;
    charTimer = 0;
    startCharPtr = str;
    curCharPtr = startCharPtr;
}

bool TextBox::IsDone()
{
    return !drawingDialog;
}

int TextBox::GetHeight()
{
    return height;
}

int TextBox::GetX()
{
    return left;
}

int TextBox::GetY()
{
    return top;
}

void TextBox::SetX( int x )
{
    left = x;
}

void TextBox::SetY( int y )
{
    top = y;
}

void TextBox::Update()
{
    if ( !drawingDialog )
        return;

    if ( charTimer == 0 )
    {
        uint8_t attr;
        uint8_t ch;

        do
        {
            ch   = *curCharPtr & 0x3F;
            attr = *curCharPtr & 0xC0;
            if ( attr == 0xC0 )
                drawingDialog = false;
            else if ( attr != 0 )
                height += 8;

            curCharPtr++;
            if ( ch != Char_JustSpace )
                Sound::PlayEffect( SEffect_character );
        } while ( drawingDialog && ch == Char_JustSpace );
        charTimer = charDelay - 1;
    }
    else
    {
        charTimer--;
    }
}

void TextBox::Draw()
{
    int x = left;
    int y = top;

    for ( const uint8_t* charPtr = startCharPtr; charPtr != curCharPtr; charPtr++ )
    {
        uint8_t attr = *charPtr & 0xC0;
        uint8_t ch   = *charPtr & 0x3F;

        if ( ch != Char_JustSpace )
            DrawChar( ch, x, y, 0 );

        if ( attr == 0 )
        {
            x += 8;
        }
        else
        {
            x = StartX;
            y += 8;
        }
    }
}
