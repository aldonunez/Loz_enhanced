/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "RegisterMenu.h"
#include "Graphics.h"
#include "Input.h"
#include "ItemObj.h"
#include "Profile.h"
#include "SaveFolder.h"
#include "Sound.h"
#include "SoundId.h"
#include "World.h"


const static uint8_t RegisterStr[] = 
{
    0x6A, 0x6A, 0x6A, 0x6A,
    0x1B, 0x0E, 0x10, 0x12, 0x1C, 0x1D, 0x0E, 0x1B, 0x24, 0x22, 0x18, 0x1E, 0x1B, 0x24, 0x17, 0x0A, 
    0x16, 0x0E,
    0x6A, 0x6A, 0x6A,
};

const static uint8_t RegisterEndStr[] = 
{ 0x1B, 0x0E, 0x10, 0x12, 0x1C, 0x1D, 0x0E, 0x1B, 0x24, 0x24, 0x24, 0x24, 0x0E, 0x17, 0x0D };

const static uint8_t CharSetStrBlank[] = 
{
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 
    0x60, 0x60, 0x60, 0x60, 0x60
};

const static uint8_t CharSetStr0[] = 
{
    0x0A, 0x60, 0x0B, 0x60, 0x0C, 0x60, 0x0D, 0x60, 0x0E, 0x60, 0x0F, 0x60, 0x10, 0x60, 0x11, 0x60, 
    0x12, 0x60, 0x13, 0x60, 0x14
};

const static uint8_t CharSetStr1[] = 
{
    0x15, 0x60, 0x16, 0x60, 0x17, 0x60, 0x18, 0x60, 0x19, 0x60, 0x1A, 0x60, 0x1B, 0x60, 0x1C, 0x60,
    0x1D, 0x60, 0x1E, 0x60, 0x1F
};

const static uint8_t CharSetStr2[] = 
{
    0x20, 0x60, 0x21, 0x60, 0x22, 0x60, 0x23, 0x60, 0x62, 0x60, 0x63, 0x60, 0x28, 0x60, 0x29, 0x60,
    0x2A, 0x60, 0x2B, 0x60, 0x2C
};

const static uint8_t CharSetStr3[] = 
{
    0x00, 0x60, 0x01, 0x60, 0x02, 0x60, 0x03, 0x60, 0x04, 0x60, 0x05, 0x60, 0x06, 0x60, 0x07, 0x60,
    0x08, 0x60, 0x09, 0x60, 0x24
};

const static uint8_t* CharSetStrs[] =
{
    CharSetStr0,
    CharSetStrBlank,
    CharSetStr1,
    CharSetStrBlank,
    CharSetStr2,
    CharSetStrBlank,
    CharSetStr3,
};

const static uint8_t Quest2Name[] = { 0x23, 0x0E, 0x15, 0x0D, 0x0A, 0x24, 0x24, 0x24 };


RegisterMenu::RegisterMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
    :   summaries( summaries ),
        selectedIndex( -1 ),
        namePos( 0 ),
        charPosCol( 0 ),
        charPosRow( 0 )
{
    // So that characters are transparent.
    Graphics::SetColor( 0, 0, 0x00000000 );
    Graphics::UpdatePalettes();

    for ( int i = 0; i < MaxProfiles; i++ )
    {
        origActive[i] = summaries->Summaries[i].IsActive();
    }

    SelectNext();
}

void RegisterMenu::SelectNext()
{
    do
    {
        selectedIndex++;
        if ( selectedIndex >= 4 )
            selectedIndex = 0;
    } while ( selectedIndex < 3 && origActive[selectedIndex] );
}

void RegisterMenu::MoveNextNamePosition()
{
    namePos++;
    if ( namePos >= MaxNameLength )
        namePos = 0;
}

void RegisterMenu::AddCharToName( char ch )
{
    ProfileSummary& summary = summaries->Summaries[selectedIndex];
    if ( summary.NameLength == 0 )
    {
        memset( summary.Name, Char_Space, MaxNameLength );
        summary.NameLength = MaxNameLength;
        summary.HeartContainers = DefaultHearts;
    }
    summary.Name[namePos] = ch;
    MoveNextNamePosition();
}

char RegisterMenu::GetSelectedChar()
{
    char ch = CharSetStrs[charPosRow][charPosCol];
    return ch;
}

void RegisterMenu::MoveCharSetCursorH( int dir )
{
    int fullSize = _countof( CharSetStr0 ) * _countof( CharSetStrs );

    for ( int i = 0; i < fullSize; i++ )
    {
        charPosCol += dir;

        if ( charPosCol < 0 )
        {
            charPosCol = _countof( CharSetStr0 ) - 1;
            MoveCharSetCursorV( -1, false );
        }
        else if ( charPosCol >= _countof( CharSetStr0 ) )
        {
            charPosCol = 0;
            MoveCharSetCursorV( 1, false );
        }

        if ( GetSelectedChar() != 0x60 )
            break;
    }
}

void RegisterMenu::MoveCharSetCursorV( int dir, bool skip )
{
    for ( int i = 0; i < _countof( CharSetStrs ); i++ )
    {
        charPosRow += dir;

        if ( charPosRow < 0 )
        {
            charPosRow = _countof( CharSetStrs ) - 1;
        }
        else if ( charPosRow >= _countof( CharSetStrs ) )
        {
            charPosRow = 0;
        }

        if ( GetSelectedChar() != 0x60 || !skip )
            break;
    }
}

void RegisterMenu::CommitFiles()
{
    for ( int i = 0; i < MaxProfiles; i++ )
    {
        if ( !origActive[i] && summaries->Summaries[i].IsActive() )
        {
            ProfileSummary& summary = summaries->Summaries[i];
            Profile profile = { 0 };

            if ( (summary.NameLength == _countof( Quest2Name )) 
                && memcmp( summary.Name, Quest2Name, summary.NameLength ) == 0 )
            {
                summary.Quest = 1;
            }

            profile.NameLength = summary.NameLength;
            memcpy( profile.Name, summary.Name, summary.NameLength );
            profile.Quest = summary.Quest;
            profile.Items[ItemSlot_HeartContainers] = summary.HeartContainers;
            profile.Items[ItemSlot_MaxBombs] = DefaultBombs;
            // Leave deaths set 0.
            SaveFolder::WriteProfile( i, profile );
        }
    }
}

void RegisterMenu::Update()
{
    if ( Input::IsKeyPressing( SelectKey ) )
    {
        SelectNext();
        namePos = 0;
        Sound::PlayEffect( SEffect_cursor );
    }
    else if ( Input::IsKeyPressing( MenuKey ) )
    {
        if ( selectedIndex == 3 )
        {
            CommitFiles();
            World::ChooseFile( summaries );
        }
    }
    else if ( Input::IsKeyPressing( WeaponKey ) )
    {
        if ( selectedIndex < 3 )
        {
            AddCharToName( GetSelectedChar() );
            Sound::PlayEffect( SEffect_put_bomb );
        }
    }
    else if ( Input::IsKeyPressing( ItemKey ) )
    {
        if ( selectedIndex < 3 )
            MoveNextNamePosition();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        MoveCharSetCursorH( 1 );
        Sound::PlayEffect( SEffect_cursor );
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        MoveCharSetCursorH( -1 );
        Sound::PlayEffect( SEffect_cursor );
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        MoveCharSetCursorV( 1 );
        Sound::PlayEffect( SEffect_cursor );
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        MoveCharSetCursorV( -1 );
        Sound::PlayEffect( SEffect_cursor );
    }
}

void RegisterMenu::Draw()
{
    Graphics::Begin();

    al_clear_to_color( al_map_rgb( 0, 0, 0 ) );

    int x, y;

    if ( selectedIndex < 3 )
    {
        bool showCursor = (GetFrameCounter() >> 3) & 1;
        if ( showCursor )
        {
            x = 0x70 + (namePos * 8);
            y = 0x30 + (selectedIndex * 24);
            DrawChar( 0x25, x, y, 7 );

            x = 0x30 + (charPosCol * 8);
            y = 0x88 + (charPosRow * 8);
            DrawChar( 0x25, x, y, 7 );
        }
    }

    DrawBox( 0x28, 0x80, 0xB8, 0x48 );
    DrawString( RegisterStr, sizeof RegisterStr, 0x20, 0x18, 0 );
    DrawString( RegisterEndStr, sizeof RegisterEndStr, 0x50, 0x78, 0 );

    y = 0x88;
    for ( int i = 0; i < _countof( CharSetStrs ); i++, y += 8 )
    {
        DrawString( CharSetStrs[i], sizeof CharSetStr0, 0x30, y, 0 );
    }

    y = 0x30;
    for ( int i = 0; i < 3; i++ )
    {
        DrawString( summaries->Summaries[i].Name, summaries->Summaries[i].NameLength, 0x70, y, 0 );
        DrawFileIcon( 0x50, y, 0 );
        y += 24;
    }

    if ( selectedIndex < 3 )
        y = 0x30 + selectedIndex * 24 + 4;
    else
        y = 0x78 + (selectedIndex - 3) * 16;
    DrawChar( Char_FullHeart, 0x44, y, 7 );

    Graphics::End();
}
