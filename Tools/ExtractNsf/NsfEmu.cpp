/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

// This is the main DLL file.

#include "stdafx.h"
#include "NsfEmu.h"
#include <msclr/marshal.h>
#include "Game_Music_Emu\gme\Nsf_Emu.h"


namespace ExtractNsf {

    void CheckBlarggError( blargg_err_t err )
    {
        if ( err == NULL )
            return;

        String^ message = gcnew String( err );
        throw gcnew Exception( message );
    }

    NsfEmu::NsfEmu()
    {
        emu = new Nsf_Emu();
        emu->ignore_silence();
    }

    NsfEmu::~NsfEmu()
    {
        this->!NsfEmu();
    }

    NsfEmu::!NsfEmu()
    {
        delete emu;
        emu = NULL;
    }

    int NsfEmu::SampleRate::get()
    {
        return emu->sample_rate();
    }

    void NsfEmu::SampleRate::set( int value )
    {
        CheckBlarggError( emu->set_sample_rate( value ) );
    }

    void NsfEmu::Gain::set( double value )
    {
        emu->set_gain( value );
    }

    int NsfEmu::TrackCount::get()
    {
        return emu->track_count();
    }

    int NsfEmu::CurrentTrack::get()
    {
        return emu->current_track();
    }

    String^ NsfEmu::Warning::get()
    {
        String^ str = gcnew String( emu->warning() );
        return str;
    }

    int NsfEmu::Tell::get()
    {
        return emu->tell();
    }

    void NsfEmu::LoadFile( String^ path )
    {
        msclr::interop::marshal_context context;
        const char* nativePath = context.marshal_as<const char*>( path );
        CheckBlarggError( emu->load_file( nativePath ) );
    }

    void NsfEmu::LoadMem( array<Byte>^ buffer, int size )
    {
        if ( size > buffer->Length )
            throw gcnew Exception( "buffer is smaller than size" );

        pin_ptr<Byte> pinnedBuf = &buffer[0];
        CheckBlarggError( emu->load_mem( pinnedBuf, size ) );
    }

    void NsfEmu::StartTrack( int trackNumber )
    {
        CheckBlarggError( emu->start_track( trackNumber ) );
    }

    void NsfEmu::Play( int count, array<short>^ buffer )
    {
        if ( count > buffer->Length )
            throw gcnew Exception( "buffer is smaller than count" );

        pin_ptr<short> pinnedBuf = &buffer[0];
        CheckBlarggError( emu->play( count, pinnedBuf ) );
    }
}
