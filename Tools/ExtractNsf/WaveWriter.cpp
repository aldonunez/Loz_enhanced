/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Stdafx.h"
#include "WaveWriter.h"
#include <msclr/marshal.h>
#include "Game_Music_Emu\demo\Wave_Writer.h"

namespace ExtractNsf
{
    WaveWriter::WaveWriter( int sampleRate, String^ filename )
    {
        msclr::interop::marshal_context context;
        const char* nativePath = context.marshal_as<const char*>( filename );
        writer = new Wave_Writer( sampleRate, nativePath );
    }

    WaveWriter::~WaveWriter()
    {
        this->!WaveWriter();
    }

    WaveWriter::!WaveWriter()
    {
        delete writer;
        writer = NULL;
    }

    void WaveWriter::EnableStereo()
    {
        writer->enable_stereo();
    }

    void WaveWriter::Write( array<short>^ buffer, int count, int skip )
    {
        if ( count > buffer->Length )
            throw gcnew Exception( "buffer is smaller than count" );

        pin_ptr<short> pinnedBuf = &buffer[0];
        writer->write( pinnedBuf, count, skip );
    }

    int WaveWriter::SampleCount::get()
    {
        return writer->sample_count();
    }

    void WaveWriter::Close()
    {
        writer->close();
    }
}
