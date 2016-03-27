/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "EliminateMenu.h"
#include "Graphics.h"
#include "Input.h"
#include "ItemObj.h"
#include "Profile.h"
#include "SaveFolder.h"
#include "Sound.h"
#include "SoundId.h"
#include "World.h"


const static uint8_t EliminateStr[] = 
{
    0x6A, 0x6A, 0x6A, 0x6A,
    0x0E, 0x15, 0x12, 0x16, 0x12, 0x17, 0x0A, 0x1D, 0x12, 0x18, 0x17, 0x24, 0x24, 0x16, 0x18, 0x0D, 0x0E,
    0x6A, 0x6A, 0x6A, 0x6A
};

const static uint8_t EliminationEndStr[] = 
{
    0x0E, 0x15, 0x12, 0x16, 0x12, 0x17, 0x0A, 0x1D, 0x12, 0x18, 0x17, 0x24, 0x0E, 0x17, 0x0D,
};


EliminateMenu::EliminateMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
    :   summaries( summaries ),
        selectedIndex( -1 )
{
    SelectNext();
}

void EliminateMenu::SelectNext()
{
    do
    {
        selectedIndex++;
        if ( selectedIndex >= 4 )
            selectedIndex = 0;
    } while ( selectedIndex < 3 && !summaries->Summaries[selectedIndex].IsActive() );
}

void EliminateMenu::DeleteCurrentProfile()
{
    Profile profile = { 0 };
    SaveFolder::WriteProfile( selectedIndex, profile );
    memset( &summaries->Summaries[selectedIndex], 0, sizeof ProfileSummary );
    Sound::PlayEffect( SEffect_player_hit );
}

void EliminateMenu::Update()
{
    if ( Input::IsButtonPressing( InputButtons::Select ) )
    {
        SelectNext();
        Sound::PlayEffect( SEffect_cursor );
    }
    else if ( Input::IsButtonPressing( InputButtons::Start ) )
    {
        if ( selectedIndex < 3 )
            DeleteCurrentProfile();
        else if ( selectedIndex == 3 )
            World::ChooseFile( summaries );
    }
}

void EliminateMenu::Draw()
{
    Graphics::Begin();

    al_clear_to_color( al_map_rgb( 0, 0, 0 ) );

    DrawString( EliminateStr, sizeof EliminateStr, 0x20, 0x18, 0 );
    DrawString( EliminationEndStr, sizeof EliminationEndStr, 0x50, 0x78, 0 );

    int y = 0x30;
    for ( int i = 0; i < 3; i++ )
    {
        ProfileSummary& summary = summaries->Summaries[i];
        if ( summary.IsActive() )
        {
            DrawString( summary.Name, summary.NameLength, 0x70, y, 0 );
            DrawFileIcon( 0x50, y, summary.Quest );
        }
        y += 24;
    }

    if ( selectedIndex < 3 )
        y = 0x30 + selectedIndex * 24 + 4;
    else
        y = 0x78 + (selectedIndex - 3) * 16;
    DrawChar( Char_FullHeart, 0x44, y, 7 );

    Graphics::End();
}
