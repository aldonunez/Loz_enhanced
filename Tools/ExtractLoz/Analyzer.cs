/*
   Copyright 2018 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.IO;

namespace ExtractLoz
{
    struct MapLayout
    {
        public bool owLayoutFormat;
        public int uniqueRoomCount;
        public int columnsInRoom;
        public int rowsInRoom;
        public byte[] roomCols;
        public ushort[] colTablePtrs;
        public byte[] colTables;
    }

    class Analyzer
    {
        public static void AnalyzeUniqueLayouts( MapLayout layout, string typeName, Options options )
        {
            byte[,] map = new byte[layout.rowsInRoom, layout.columnsInRoom];
            int mapSize = 1 * layout.rowsInRoom * layout.columnsInRoom;
            int totalUncompressed = 0;
            int totalCompressedRow = 0;
            int totalCompressedFull = 0;

            for ( int i = 0; i < layout.uniqueRoomCount; i++ )
            {
                if ( !ExtractUniqueLayout( layout, i, map ) )
                    continue;

                var compressedMapRleRow = CompressMapRleRow( map );
                var compressedMapRleFull = CompressMapRleFull( map );

                totalUncompressed += mapSize;
                totalCompressedRow += compressedMapRleRow.Length;
                totalCompressedFull += compressedMapRleFull.Length;

                if ( options.AnalysisWrites )
                {
                    string filename = string.Format( "map.uncompressed.{0}.{1:D3}.bin", typeName, i );
                    var filePath = options.MakeOutPath( filename );
                    using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
                    {
                        for ( int r = 0; r < map.GetLength( 0 ); r++ )
                        {
                            for ( int c = 0; c < map.GetLength( 1 ); c++ )
                            {
                                writer.Write( map[r, c] );
                            }
                        }
                    }

                    filename = string.Format( "map.compressed-rle-row.{0}.{1:D3}.bin", typeName, i );
                    filePath = options.MakeOutPath( filename );
                    File.WriteAllBytes( filePath, compressedMapRleRow );

                    filename = string.Format( "map.compressed-rle-full.{0}.{1:D3}.bin", typeName, i );
                    filePath = options.MakeOutPath( filename );
                    File.WriteAllBytes( filePath, compressedMapRleFull );
                }
            }

            float rowCompressionRatio = (totalCompressedRow / (float) totalUncompressed) * 100;
            float fullCompressionRatio = (totalCompressedFull / (float) totalUncompressed) * 100;

            Console.WriteLine( "map: {0} uncompressed size:      {1:N0}", typeName, totalUncompressed );
            Console.WriteLine( "map: {0} compressed-by-row size: {1:N0} ({2:F0}%)", typeName, totalCompressedRow, rowCompressionRatio );
            Console.WriteLine( "map: {0} compressed-full size:   {1:N0} ({2:F0}%)", typeName, totalCompressedFull, fullCompressionRatio );
        }

        private static bool ExtractUniqueLayout( MapLayout layout, int index, byte[,] map )
        {
            int MaxColumnStartOffset = (layout.columnsInRoom - 1) * layout.rowsInRoom;

            int rowEnd = layout.rowsInRoom;
            int roomCol = index * layout.columnsInRoom;

            for ( int c = 0; c < layout.columnsInRoom; c++ )
            {
                byte columnDesc = layout.roomCols[roomCol];
                int tableIndex = (columnDesc & 0xF0) >> 4;
                int columnIndex = (columnDesc & 0x0F);

                if ( tableIndex >= layout.colTablePtrs.Length )
                    return false;

                int k = 0;
                int j = 0;
                int colTableStart = layout.colTablePtrs[tableIndex];

                for ( j = 0; j < MaxColumnStartOffset; j++ )
                {
                    byte t = layout.colTables[colTableStart + j];

                    if ( (t & 0x80) != 0 )
                    {
                        if ( k == columnIndex )
                            break;
                        k++;
                    }
                }

                for ( int r = 0; r < rowEnd; j++ )
                {
                    byte t = layout.colTables[colTableStart + j];
                    int tileRef;

                    if ( layout.owLayoutFormat )
                        tileRef = t & 0x3F;
                    else
                        tileRef = t & 0x7;

                    map[r++, c] = (byte) tileRef;

                    if ( layout.owLayoutFormat )
                    {
                        if ( (t & 0x40) != 0 && r < rowEnd )
                        {
                            map[r++, c] = (byte) tileRef;
                        }
                    }
                    else
                    {
                        int repeat = (t >> 4) & 0x7;
                        for ( int m = 0; m < repeat && r < rowEnd; m++ )
                        {
                            map[r++, c] = (byte) tileRef;
                        }
                    }
                }

                roomCol++;
            }

            return true;
        }

        private static byte[] CompressMapRleRow( byte[,] map )
        {
            MemoryStream stream = new MemoryStream();
            var compressor = new RleCompressor( stream );

            for ( int r = 0; r < map.GetLength( 0 ); r++ )
            {
                for ( int c = 0; c < map.GetLength( 1 ); c++ )
                {
                    compressor.Push( map[r, c] );
                }

                compressor.Drain();
            }

            return stream.ToArray();
        }

        private static byte[] CompressMapRleFull( byte[,] map )
        {
            MemoryStream stream = new MemoryStream();
            var compressor = new RleCompressor( stream );

            for ( int r = 0; r < map.GetLength( 0 ); r++ )
            {
                for ( int c = 0; c < map.GetLength( 1 ); c++ )
                {
                    compressor.Push( map[r, c] );
                }
            }

            compressor.Drain();

            return stream.ToArray();
        }

        class RleCompressor
        {
            MemoryStream stream;
            int count;
            byte value;

            public RleCompressor( MemoryStream stream )
            {
                this.stream = stream;
            }

            public void Push( byte value )
            {
                if ( count == 0 )
                {
                    count = 1;
                    this.value = value;
                }
                else if ( value == this.value )
                {
                    count++;
                    if ( count == 256 )
                        DrainInternal();
                }
                else
                {
                    DrainInternal();
                    count = 1;
                    this.value = value;
                }
            }

            private void DrainInternal()
            {
                if ( count == 1 )
                {
                    stream.WriteByte( value );
                }
                else
                {
                    int element = (count - 1) | 0x80;
                    stream.WriteByte( (byte) element );
                    stream.WriteByte( value );
                }

                count = 0;
            }

            public void Drain()
            {
                if ( count != 0 )
                    DrainInternal();
            }
        }
    }
}
