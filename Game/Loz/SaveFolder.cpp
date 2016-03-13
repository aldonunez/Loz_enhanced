/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "SaveFolder.h"


enum OpenMode
{
    Open_Read,
    Open_Write,
};

const char* SaveFileNamePattern = "z1_%d.sav";


// The save should probably have a version and checksum
// for compatibility and integrity testing

static errno_t OpenFile( FILE** file, int slot, OpenMode mode )
{
    ALLEGRO_PATH* path = nullptr;
    const char* pathStr = nullptr;
    char fileName[32] = "";
    errno_t err = 0;

    path = al_get_standard_path( ALLEGRO_USER_DATA_PATH );
    if ( path == nullptr )
        return EPERM;

    pathStr = al_path_cstr( path, ALLEGRO_NATIVE_PATH_SEP );
    _mkdir( pathStr );

    sprintf_s( fileName, SaveFileNamePattern, slot );
    al_set_path_filename( path, fileName );

    pathStr = al_path_cstr( path, ALLEGRO_NATIVE_PATH_SEP );

    if ( mode == Open_Write )
    {
        err = fopen_s( file, pathStr, "r+b" );
        if ( err == ENOENT )
            err = fopen_s( file, pathStr, "wb" );
    }
    else
    {
        err = fopen_s( file, pathStr, "rb" );
    }

    al_destroy_path( path );

    return err;
}


void SaveFolder::ReadSummaries( ProfileSummarySnapshot& summaries )
{
    for ( int i = 0; i < MaxProfiles; i++ )
    {
        ProfileSummary& summary = summaries.Summaries[i];
        Profile profile;
        FILE* file = nullptr;
        errno_t err = 0;
        size_t lenRead = 0;

        err = OpenFile( &file, i, Open_Read );
        if ( err == 0 )
        {
            lenRead = fread( &profile, sizeof profile, 1, file );
            fclose( file );
        }

        if ( lenRead < 1 )
        {
            memset( &summaries.Summaries[i], 0, sizeof ProfileSummary );
        }
        else
        {
            summary.Deaths = profile.Deaths;
            summary.HeartContainers = profile.Items[ItemSlot_HeartContainers];
            summary.Quest = profile.Quest;
            summary.NameLength = profile.NameLength;
            memcpy( summary.Name, profile.Name, profile.NameLength );
        }
    }
}

bool SaveFolder::ReadProfile( int index, Profile& profile )
{
    assert( index < MaxProfiles );

    FILE* file = nullptr;
    errno_t err = 0;
    size_t lenRead = 0;

    err = OpenFile( &file, index, Open_Read );
    if ( err == 0 )
    {
        lenRead = fread( &profile, sizeof( Profile ), 1, file );
        fclose( file );
    }

    if ( lenRead < 1 )
    {
        memset( &profile, 0, sizeof( Profile ) );
        return false;
    }

    return true;
}

bool SaveFolder::WriteProfile( int index, const Profile& profile )
{
    assert( index < MaxProfiles );

    FILE* file = nullptr;
    errno_t err = 0;
    size_t lenWritten = 0;

    err = OpenFile( &file, index, Open_Write );
    if ( err != 0 )
        return false;

    lenWritten = fwrite( &profile, sizeof( Profile ), 1, file );
    fclose( file );

    if ( lenWritten != 1 )
        return false;

    return true;
}
