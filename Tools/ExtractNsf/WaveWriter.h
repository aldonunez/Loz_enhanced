/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

class Wave_Writer;

namespace ExtractNsf
{
    using namespace System;

    public ref class WaveWriter
    {
        Wave_Writer* writer;

    public:
        WaveWriter( int sampleRate, String^ filename );
        ~WaveWriter();

        void EnableStereo();
        void Write( array<short>^ buffer, int count, int skip );
        property int SampleCount
        {
            int get();
        }
        void Close();

    protected:
        !WaveWriter();
    };
}
