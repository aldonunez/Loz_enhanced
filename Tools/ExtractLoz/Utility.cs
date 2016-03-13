/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.IO;

namespace ExtractLoz
{
    static class Utility
    {
        public static int AlignStream( Stream stream, int alignment )
        {
            int pos = (int) stream.Position;
            int subAlign = alignment - 1;
            pos = (pos + subAlign) & ~subAlign;
            stream.Position = pos;
            return pos;
        }

        public static void PadStream( Stream stream )
        {
            AlignStream( stream, 4 );
            stream.SetLength( stream.Position );
        }

        public static FileStream TruncateFile( string path )
        {
            return File.Open( path, FileMode.Create, FileAccess.Write );
        }
    }
}
