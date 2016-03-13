/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class Sound
{
public:
    enum
    {
        MainSongStream,
        EventSongStream,
    };

    enum
    {
        AmbientInstance = 4,
    };

public:
    static bool Init();
    static void Uninit();

    static void Update();

    static void PlaySong( int id, int stream, bool loop );
    static void PushSong( int id );
    static void StopSongs();

    static void PlayEffect( int id, bool loop = false, int instance = -1 );
    static void StopEffect( int instance );
    static void StopEffects();

    static void StopAll();
    static void Pause();
    static void Unpause();
};
