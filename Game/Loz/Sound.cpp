/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Sound.h"
#include "SoundId.h"
#include "Util.h"
#include <allegro5\allegro_audio.h>


enum
{
    Instances       = 5,
    Streams         = 2,
    LoPriStreams    = Streams - 1,
    Songs           = Song_Max,
    Effects         = SEffect_Max,
    NoSound         = 0xFF,
};

enum SoundFlags
{
    SoundFlag_PlayIfQuietSlot       = 1,
};

struct EffectRequest
{
    uint8_t SoundId;
    bool    Loop;
};


struct SoundInfo
{
    // The loop beginning and end points in frames (1/60 seconds).
    int16_t Begin;
    int16_t End;
    int8_t  Slot;
    int8_t  Priority;
    int8_t  Flags;
    int8_t  Reserved;
    char    Filename[20];
};


static ALLEGRO_VOICE* defaultVoice;
static ALLEGRO_MIXER* defaultMixer;
static ALLEGRO_SAMPLE_INSTANCE* sampleInstances[Instances];
static ALLEGRO_SAMPLE* effectSamples[Effects];
static ALLEGRO_AUDIO_STREAM* streams[Streams];
static double savedPos[LoPriStreams];
static SoundInfo songs[Songs];
static SoundInfo effects[Effects];
static EffectRequest effectRequests[Instances];
static bool paused;
static bool pausedSongs[Streams];
static bool pausedEffects[Instances];
static int pausedEffectPos[Instances];


static void PlaySongInternal( int songId, int streamId, bool loop, bool play )
{
    al_destroy_audio_stream( streams[streamId] );

    streams[streamId] = al_load_audio_stream( songs[songId].Filename, 2, 2048 );
    if ( streams[streamId] == nullptr )
        return;

    if ( !al_attach_audio_stream_to_mixer( streams[streamId], defaultMixer ) )
    {
        al_destroy_audio_stream( streams[streamId] );
        streams[streamId] = nullptr;
        return;
    }

    ALLEGRO_PLAYMODE playMode = ALLEGRO_PLAYMODE_ONCE;

    if ( loop )
    {
        playMode = ALLEGRO_PLAYMODE_LOOP;

        if ( songs[songId].End >= 0 )
        {
            double beginSecs = 0;
            double endSecs = songs[songId].End * (1 / 60.0);

            if ( songs[songId].Begin >= 0 )
                beginSecs = songs[songId].Begin * (1 / 60.0);

            al_set_audio_stream_loop_secs( streams[streamId], beginSecs, endSecs );
        }
    }

    al_set_audio_stream_playmode( streams[streamId], playMode );
    al_set_audio_stream_playing( streams[streamId], play );
}

bool Sound::Init()
{
    defaultVoice = al_create_voice( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );
    if ( defaultVoice == nullptr )
        return false;

    defaultMixer = al_create_mixer( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );
    if ( defaultMixer == nullptr )
        return false;

    if ( !al_attach_mixer_to_voice( defaultMixer, defaultVoice ) )
        return false;

    for ( int i = 0; i < Instances; i++ )
    {
        effectRequests[i].SoundId = NoSound;
        effectRequests[i].Loop = false;

        sampleInstances[i] = al_create_sample_instance( nullptr );
        if ( sampleInstances[i] == nullptr )
            return false;

        if ( !al_attach_sample_instance_to_mixer( sampleInstances[i], defaultMixer ) )
            return false;
    }

    if ( !Util::LoadList<SoundInfo>( "Songs.dat", songs, Songs ) )
        return false;
    if ( !Util::LoadList<SoundInfo>( "Effects.dat", effects, Effects ) )
        return false;

    for ( int i = 0; i < Effects; i++ )
    {
        effectSamples[i] = al_load_sample( effects[i].Filename );
        if ( effectSamples[i] == nullptr )
            return false;
    }

    return true;
}

void Sound::Uninit()
{
    for ( int i = 0; i < SEffect_Max; i++ )
    {
        al_destroy_sample( effectSamples[i] );
    }

    for ( int i = 0; i < Streams; i++ )
    {
        al_destroy_audio_stream( streams[i] );
    }

    for ( int i = 0; i < Instances; i++ )
    {
        al_destroy_sample_instance( sampleInstances[i] );
    }

    al_destroy_mixer( defaultMixer );
    al_destroy_voice( defaultVoice );
}

static void UpdateSongs()
{
    if ( paused )
        return;

    if ( streams[Sound::EventSongStream] == nullptr
        || al_get_audio_stream_playing( streams[Sound::EventSongStream] ) )
        return;

    al_destroy_audio_stream( streams[Sound::EventSongStream] );
    streams[Sound::EventSongStream] = nullptr;

    for ( int i = 0; i < LoPriStreams; i++ )
    {
        if ( streams[i] != nullptr )
        {
            al_seek_audio_stream_secs( streams[i], savedPos[i] );
            al_set_audio_stream_playing( streams[i], true ); 
        }
    }
}

static void UpdateEffects()
{
    for ( int i = 0; i < Instances; i++ )
    {
        int id = effectRequests[i].SoundId;
        effectRequests[i].SoundId = NoSound;
        if ( id != NoSound )
        {
            ALLEGRO_SAMPLE_INSTANCE* instance = sampleInstances[i];

            if ( (effects[id].Flags & SoundFlag_PlayIfQuietSlot) == 0
                || !al_get_sample_instance_playing( instance ) )
            {
                al_set_sample( instance, effectSamples[id] );

                ALLEGRO_PLAYMODE playMode = ALLEGRO_PLAYMODE_ONCE;

                if ( effectRequests[i].Loop )
                    playMode = ALLEGRO_PLAYMODE_LOOP;

                al_set_sample_instance_playmode( instance, playMode );
                al_play_sample_instance( instance );
            }
        }
    }
}

void Sound::Update()
{
    UpdateSongs();
    UpdateEffects();
}

void Sound::PlaySong( int songId, int streamId, bool loop )
{
    if ( streamId < 0 || streamId >= LoPriStreams )
        return;
    if ( songId < 0 || songId >= _countof( songs ) )
        return;

    if ( streams[EventSongStream] == nullptr || !al_get_audio_stream_playing( streams[EventSongStream] ) )
    {
        PlaySongInternal( songId, streamId, loop, true );
        return;
    }

    PlaySongInternal( songId, streamId, loop, false );
    savedPos[streamId] = 0;
}

void Sound::PushSong( int songId )
{
    if ( songId < 0 || songId >= _countof( songs ) )
        return;

    for ( int i = 0; i < LoPriStreams; i++ )
    {
        if ( streams[i] != nullptr && al_get_audio_stream_playing( streams[i] ) )
        {
            savedPos[i] = al_get_audio_stream_position_secs( streams[i] );
            al_set_audio_stream_playing( streams[i], false );
        }
    }

    PlaySongInternal( songId, EventSongStream, false, true );
}

void Sound::StopSongs()
{
    for ( int i = 0; i < Streams; i++ )
    {
        if ( streams[i] == nullptr )
            continue;

        al_set_audio_stream_playing( streams[i], false );
        al_destroy_audio_stream( streams[i] );
        streams[i] = nullptr;
    }
}

void Sound::PlayEffect( int id, bool loop, int instance )
{
    if ( id < 0 || id >= _countof( effectSamples ) )
        return;
    if ( instance < -1 || instance >= Instances )
        return;

    int index;

    if ( instance == -1 )
        index = effects[id].Slot - 1;
    else
        index = instance;

    int prevId = effectRequests[index].SoundId;

    if ( (prevId == NoSound) || (effects[id].Priority < effects[prevId].Priority) )
    {
        effectRequests[index].SoundId = id;
        effectRequests[index].Loop = loop;
    }
}

void Sound::StopEffect( int instance )
{
    al_stop_sample_instance( sampleInstances[instance] );
    effectRequests[instance].SoundId = NoSound;
    effectRequests[instance].Loop = false;
}

void Sound::StopEffects()
{
    for ( int i = 0; i < Instances; i++ )
    {
        StopEffect( i );
    }
}

void Sound::StopAll()
{
    StopSongs();
    StopEffects();
}

void Sound::Pause()
{
    paused = true;

    for ( int i = Streams - 1; i >= 0; i-- )
    {
        if ( streams[i] != nullptr )
        {
            if ( al_get_audio_stream_playing( streams[i] ) )
            {
                pausedSongs[i] = true;
                al_set_audio_stream_playing( streams[i], false );
            }
            if ( i == EventSongStream )
                break;
        }
    }

    for ( int i = 0; i < Instances; i++ )
    {
        if ( al_get_sample_instance_playing( sampleInstances[i] ) )
        {
            pausedEffects[i] = true;
            pausedEffectPos[i] = al_get_sample_instance_position( sampleInstances[i] );
            al_set_sample_instance_playing( sampleInstances[i], false );
        }
    }
}

void Sound::Unpause()
{
    paused = false;

    for ( int i = Streams - 1; i >= 0; i-- )
    {
        if ( streams[i] != nullptr )
        {
            if ( pausedSongs[i] )
            {
                pausedSongs[i] = false;
                al_set_audio_stream_playing( streams[i], true );
            }
            if ( i == EventSongStream )
                break;
        }
    }

    for ( int i = 0; i < Instances; i++ )
    {
        if ( pausedEffects[i] )
        {
            pausedEffects[i] = false;
            al_set_sample_instance_position( sampleInstances[i], pausedEffectPos[i] );
            al_set_sample_instance_playing( sampleInstances[i], true );
        }
    }
}
