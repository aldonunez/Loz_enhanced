/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

// ExtractNsf.h

#pragma once

class Nsf_Emu;

namespace ExtractNsf {

    using namespace System;

	public ref class NsfEmu
	{
        Nsf_Emu* emu;

    public:
        NsfEmu();
        ~NsfEmu();

        property int SampleRate
        {
            int get();
            void set( int value );
        }

        property double Gain
        {
            void set( double value );
        }

        property int TrackCount
        {
            int get();
        }

        property int CurrentTrack
        {
            int get();
        }

        property String^ Warning
        {
            String^ get();
        }

        property int Tell
        {
            int get();
        }

        void LoadFile( String^ path );
        void StartTrack( int trackNumber );
        void Play( int count, array<short>^ buffer );

    protected:
        !NsfEmu();
	};
}
