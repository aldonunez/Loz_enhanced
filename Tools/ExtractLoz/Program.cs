/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace ExtractLoz
{
    class SpriteFrame
    {
        public byte X;
        public byte Y;
        public byte Flags;
    }

    class SpriteAnim
    {
        public byte Width;
        public byte Height;
        public SpriteFrame[] Frames;
    }

    class Program
    {
        delegate void Extractor( Options options );

        const int PrimarySquareTable = 0x1697C + 16;
        const int SecondarySquareTable = 0x169B4 + 16;
        const int SecretSquareTable = 0x16976 + 16;
        const int UnderworldSquareTable = 0x16718 + 16;
        const int OWTileCHR = 0xC93B + 16;
        const int UWTileCHR = 0xC11B + 16;
        const int Misc1CHR = 0x877F + 16;
        const int Misc2CHR = 0x8E7F + 16;

        const int TileSize = 16;

        static byte[] tileBuf = new byte[TileSize];

        static void Main( string[] args )
        {
            var options = Options.Parse( args );

            if ( options.Error != null )
            {
                Console.Error.WriteLine( options.Error );
                return;
            }

            if ( options.Function == null )
            {
                Console.WriteLine( "Nothing to work on." );
                return;
            }

            Dictionary<string, Extractor> extractorMap = new Dictionary<string, Extractor>();

            extractorMap.Add( "overworldtiles", ExtractOverworldBundle );
            extractorMap.Add( "underworldtiles", ExtractUnderworldBundle );
            extractorMap.Add( "sprites", ExtractSpriteBundle );
            extractorMap.Add( "text", ExtractTextBundle );
            extractorMap.Add( "sound", ExtractSound );

            Extractor extractor = null;

            if ( options.Function == "all" )
            {
                foreach ( var pair in extractorMap )
                {
                    Console.WriteLine( "Extracting {0} ...", pair.Key );
                    pair.Value( options );
                }
            }
            else if ( extractorMap.TryGetValue( options.Function, out extractor ) )
            {
                Console.WriteLine( "Extracting {0} ...", options.Function );
                extractor( options );
            }
            else
            {
                Console.Error.WriteLine( "Function not supported: {0}", options.Function );
            }
        }

        private static void ExtractOverworldBundle( Options options )
        {
            ExtractOverworldTiles( options );
            ExtractOverworldTileAttrs( options );
            ExtractOverworldMap( options );
            ExtractOverworldMapAttrs( options );
            ExtractOverworldMapSparseAttrs( options );
            ExtractOverworldInfo( options );
            ExtractOverworldInfoEx( options );
            ExtractObjLists( options );
        }

        private static void ExtractUnderworldBundle( Options options )
        {
            ExtractUnderworldTiles( options );
            ExtractUnderworldTileAttrs( options );
            ExtractUnderworldMap( options );
            ExtractUnderworldMapAttrs( options );
            ExtractUnderworldInfo( options );
            ExtractUnderworldCellarMap( options );
            ExtractUnderworldCellarTiles( options );
            ExtractUnderworldCellarTileAttrs( options );
        }

        private static void ExtractSpriteBundle( Options options )
        {
            ExtractSystemPalette( options );
            ExtractSprites( options );
            ExtractOWSpriteVRAM( options );
        }

        private static void ExtractTextBundle( Options options )
        {
            ExtractFont( options );
            ExtractText( options );
            ExtractCredits( options );
        }

        private static void ExtractSound( Options options )
        {
            ExtractSounds( options, "Songs" );
            ExtractSounds( options, "Effects" );
        }

        class SoundItem
        {
            public short Track;
            public short Begin;
            public short End;
            public byte Slot;
            public byte Priority;
            public byte Flags;
            public string Filename;

            public static SoundItem ConvertFields( string[] fields )
            {
                SoundItem item = new SoundItem();
                item.Track = short.Parse( fields[0] );
                item.Begin = short.Parse( fields[1] );
                item.End = short.Parse( fields[2] );
                item.Slot = byte.Parse( fields[3] );
                item.Priority = byte.Parse( fields[4] );
                item.Flags = byte.Parse( fields[5] );
                item.Filename = fields[6];
                return item;
            }
        }

        private static void ExtractSounds( Options options, string tableFileBase )
        {
            var outPath = options.MakeOutPath( tableFileBase + ".dat" );
            using ( var inStream = GetResourceStream( "ExtractLoz.Data." + tableFileBase + ".csv" ) )
            using ( var outWriter = new BinaryWriter( Utility.TruncateFile( outPath ) ) )
            {
                var items = DatafileReader.ReadTable( inStream, SoundItem.ConvertFields );
                foreach ( var item in items )
                {
                    ExtractSoundFile( options, item );
                    outWriter.Write( item.Begin );
                    outWriter.Write( item.End );
                    outWriter.Write( item.Slot );
                    outWriter.Write( item.Priority );
                    outWriter.Write( item.Flags );
                    outWriter.Write( (byte) 0 );
                    WriteFixedString( outWriter.BaseStream, item.Filename, 20 );
                }
                Utility.PadStream( outWriter.BaseStream );
            }
        }

        private static void ExtractSoundFile( Options options, SoundItem item )
        {
            const int SampleRate = 44100;
            const double SampleRateMs = SampleRate / 1000.0;
            const double MillisecondsAFrame = 1000.0 / 60.0;

            string outPath = options.MakeOutPath( item.Filename );
            using ( ExtractNsf.NsfEmu emu = new ExtractNsf.NsfEmu() )
            using ( ExtractNsf.WaveWriter waveWriter = new ExtractNsf.WaveWriter( SampleRate, outPath ) )
            {
                emu.SampleRate = SampleRate;
                emu.LoadFile( options.NsfPath );
                emu.StartTrack( item.Track );

                waveWriter.EnableStereo();

                short[] buffer = new short[1024];
                int limit = (int) (item.End * MillisecondsAFrame);
                while ( emu.Tell < limit )
                {
                    int count = buffer.Length;
                    int samplesRem = (int) (SampleRateMs * (limit - emu.Tell));
                    if ( samplesRem < count )
                    {
                        count = (int) ((samplesRem + 1) & 0xFFFFFFFE);
                    }
                    emu.Play( count, buffer );
                    waveWriter.Write( buffer, count, 1 );
                }
            }
        }

        private static void ExtractSystemPalette( Options options )
        {
            var filePath = options.MakeOutPath( "pal.dat" );

            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                foreach ( var color in DefaultSystemPalette.Colors )
                {
                    writer.Write( color );
                }
            }
        }

        private static void ExtractFont( Options options )
        {
            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                Bitmap bmp = new Bitmap( 16 * 8, 16 * 8 );
                Color[] colors = GetPaletteStandInColors();
                int x = 0;
                int y = 0;

                reader.BaseStream.Position = Misc1CHR;

                for ( int i = 0; i < 0x70; i++ )
                {
                    DrawTile( reader, bmp, colors, x, y );

                    x += 8;
                    if ( x >= bmp.Width )
                    {
                        x = 0;
                        y += 8;
                    }
                }

                SeekBgTile( reader, UWTileCHR, 0xE2 );
                x = 0;

                for ( int i = 0; i < 16; i++ )
                {
                    DrawTile( reader, bmp, colors, x, 0x68 );
                    x += 8;
                }

                int t = 0xE5;
                x = 0x28;
                y = 0x70;

                for ( int i = 0; i < 27; i++ )
                {
                    SeekBgTile( reader, OWTileCHR, t );
                    DrawTile( reader, bmp, colors, x, y );

                    t++;
                    x += 8;
                    if ( x >= bmp.Width )
                    {
                        x = 0;
                        y += 8;
                    }
                }

                SeekUWSpriteTile( reader, UW127SpriteCHR, 0x3E );
                DrawTile( reader, bmp, colors, 0, 0x70 );

                bmp.Save( options.MakeOutPath( "font.png" ), ImageFormat.Png );
            }
        }

        private static void ExtractText( Options options )
        {
            const int TextPtrs = 0x4000 + 16;

            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = TextPtrs;
                ushort[] listPtrs = new ushort[38];

                for ( int i = 0; i < listPtrs.Length; i++ )
                {
                    listPtrs[i] = reader.ReadUInt16();
                }

                var heap = reader.ReadBytes( 0x556 );

                // 0..9     : '0'..'9'
                // $A..$23  : 'A'..'Z'
                // $24..$27 : ' ', justifying-space, ?, ?
                // $28..$2F : ',', '!', '\'', '&', '.', '"', '?', '-'
                // bits 6,7 : end of string
                // bit 7    : go to second line
                // bit 6    : go to third line

                var filePath = options.MakeOutPath( "text.tab" );
                using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
                {
                    writer.Write( (ushort) listPtrs.Length );

                    for ( int i = 0; i < listPtrs.Length; i++ )
                    {
                        ushort ptr = (ushort) (listPtrs[i] - listPtrs[0]);
                        writer.Write( ptr );
                    }

                    writer.Write( heap );

                    Utility.PadStream( writer.BaseStream );
                }
            }
        }

        private static void ExtractCredits( Options options )
        {
            const int TextPtrs = 0xAC2E + 16;

            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = TextPtrs;
                byte[] listPtrsLo = reader.ReadBytes( 0x17 );
                byte[] listPtrsHi = reader.ReadBytes( 0x17 );
                ushort[] listPtrs = new ushort[0x17];

                for ( int i = 0; i < listPtrs.Length; i++ )
                {
                    listPtrs[i] = (ushort) (listPtrsLo[i] | (listPtrsHi[i] << 8));
                }

                var heap = reader.ReadBytes( 0x19E );

                // Each entry is defined as follows:
                //  byte 0  : length
                //  byte 1  : first column (in chars)
                //  byte 2..: text

                var filePath = options.MakeOutPath( "credits.tab" );
                using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
                {
                    writer.Write( (ushort) listPtrs.Length );

                    for ( int i = 0; i < listPtrs.Length; i++ )
                    {
                        ushort ptr = (ushort) (listPtrs[i] - listPtrs[0]);
                        writer.Write( ptr );
                    }

                    writer.Write( heap );

                    Utility.PadStream( writer.BaseStream );
                }
            }

            const int LineBitmap = 0xAC22 + 16;

            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = LineBitmap;
                byte[] bytes = reader.ReadBytes( 12 );
                var filePath = options.MakeOutPath( "creditsLinesBmp.dat" );
                File.WriteAllBytes( filePath, bytes );
            }
        }

        private static void ExtractUnderworldCellarTiles( Options options )
        {
            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                Bitmap bmp = new Bitmap( 16 * 16, 4 * 16 );

                ExtractUnderworldCellarTilesImpl( reader, bmp );

                bmp.Save( options.MakeOutPath( "underworldCellarTiles.png" ), ImageFormat.Png );
            }
        }

        const int UWCellarPrimarySquareTable = 0x1697C + 16;
        const int UWCellarSecondarySquareTable = 0x169B4 + 16;

        private static void ExtractUnderworldCellarTilesImpl( BinaryReader reader, Bitmap bmp )
        {
            reader.BaseStream.Position = UWCellarPrimarySquareTable;
            var primaries = reader.ReadBytes( 56 );

            reader.BaseStream.Position = UWCellarSecondarySquareTable;
            var secondaries = reader.ReadBytes( 16 * 4 );       // 16 squares, 4 8x8 tiles each

            // Underworld cellar rooms are layed out like Overworld rooms, but there are no secret tiles.

            int x = 0;
            int y = 0;

            Color[] colors = GetPaletteStandInColors();

            byte[] tileIndexes = new byte[4];

            for ( int i = 0; i < 56; i++ )
            {
                if ( i < 16 )
                {
                    var primary = i * 4;

                    for ( int j = 0; j < 4; j++ )
                        tileIndexes[j] = secondaries[primary + j];

                    DrawBgSquare( reader, bmp, colors, x, y, UWTileCHR, tileIndexes );
                }
                else
                {
                    var primary = primaries[i];

                    // We don't need to translate secrets into primaries.

                    for ( int j = 0; j < 4; j++ )
                        tileIndexes[j] = (byte) (primary + j);

                    DrawBgSquare( reader, bmp, colors, x, y, UWTileCHR, tileIndexes );
                }

                if ( (i % 16) == 15 )
                {
                    x = 0;
                    y += 16;
                }
                else
                    x += 16;
            }
        }

        private static void ExtractUnderworldTiles( Options options )
        {
            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                Bitmap bmp = new Bitmap( 16 * 16, 4 * 16 );

                ExtractUnderworldTilesImpl( reader, bmp );

                bmp.Save( options.MakeOutPath( "underworldTiles.png" ), ImageFormat.Png );
                bmp.Dispose();

                bmp = new Bitmap( 16 * 16, 11 * 16 );

                ExtractUnderworldWalls( reader, bmp );

                bmp.Save( options.MakeOutPath( "underworldWalls.png" ), ImageFormat.Png );
                bmp.Dispose();

                bmp = new Bitmap( 16 * 16, 16 * 16 );

                ExtractUnderworldDoors( reader, bmp );

                bmp.Save( options.MakeOutPath( "underworldDoors.png" ), ImageFormat.Png );
                bmp.Dispose();
            }
        }

        private static void ExtractUnderworldTilesImpl( BinaryReader reader, Bitmap bmp )
        {
            reader.BaseStream.Position = UnderworldSquareTable;
            var primaries = reader.ReadBytes( 8 );

            int x = 0;
            int y = 0;

            Color[] colors = GetPaletteStandInColors();
            byte[] tileIndexes = new byte[4];

            for ( int i = 0; i < 8; i++ )
            {
                var primary = primaries[i];

                if ( primary < 0x70 || primary > 0xF2 )
                {
                    for ( int j = 0; j < 4; j++ )
                        tileIndexes[j] = primary;

                    DrawBgSquare( reader, bmp, colors, x, y, UWTileCHR, tileIndexes );
                }
                else
                {
                    for ( int j = 0; j < 4; j++ )
                        tileIndexes[j] = (byte) (primary + j);

                    DrawBgSquare( reader, bmp, colors, x, y, UWTileCHR, tileIndexes );
                }

                if ( (i % 16) == 15 )
                {
                    x = 0;
                    y += 16;
                }
                else
                    x += 16;
            }
        }

        private static void ExtractUnderworldWalls( BinaryReader reader, Bitmap bmp )
        {
            const int Walls = 0x15fa0 + 16;

            reader.BaseStream.Position = Walls;
            var wallTiles = reader.ReadBytes( 78 );

            var colors = GetPaletteStandInColors();
            int row = 0;
            int col = 0;
            byte[,] map = new byte[22, 32];

            for ( int i = 0; i < 78; i++ )
            {
                if ( wallTiles[i] != 0 )
                {
                    byte tile = wallTiles[i];

                    map[row + 1, col + 1] = tile;

                    tile = wallTiles[i];
                    if ( tile != 0xf5 && tile != 0xde )
                        tile++;
                    map[(10 - row) + 10, col + 1] = tile;

                    tile = wallTiles[i];
                    if ( tile == 0xde || tile == 0xdc )
                        tile++;
                    else if ( tile != 0xf5 && tile != 0xe0 )
                        tile += 2;
                    map[(10 - row) + 10, (12 - col) + 16 + 2] = tile;

                    tile = wallTiles[i];
                    if ( tile == 0xde || tile == 0xe0 )
                        tile++;
                    else if ( tile != 0xf5 && tile != 0xdc )
                        tile += 3;
                    map[row + 1, (12 - col) + 16 + 2] = tile;
                }

                row++;
                if ( row == 10 || wallTiles[i] == 0 )
                {
                    col++;
                    row = 0;
                }
            }

            for ( int i = 0; i < 32; i++ )
            {
                map[0, i] = 0xF6;
                map[21, i] = 0xF6;
            }

            for ( int i = 0; i < 20; i++ )
            {
                map[i + 1, 0] = 0xF6;
                map[i + 1, 31] = 0xF6;
            }

            for ( int r = 0; r < 22; r++ )
            {
                for ( int c = 0; c < 32; c++ )
                {
                    byte tile = map[r, c];
                    if ( tile != 0 )
                    {
                        SeekBgTile( reader, UWTileCHR, tile );
                        DrawTile( reader, bmp, colors, c * 8, r * 8 );
                    }
                }
            }

            using ( var g = Graphics.FromImage( bmp ) )
            {
                g.FillRectangle( Brushes.Black, 224, 72, 32, 32 );
                g.FillRectangle( Brushes.Black, 0, 72, 32, 32 );
                g.FillRectangle( Brushes.Black, 112, 144, 32, 32 );
                g.FillRectangle( Brushes.Black, 112, 0, 32, 32 );
            }
        }

        private static void ExtractUnderworldDoors( BinaryReader reader, Bitmap bmp )
        {
            const int EastWall = 0x15FEE + 16;
            const int WestWall = 0x1602A + 16;
            const int SouthWall = 0x16066 + 16;
            const int NorthWall = 0x160A2 + 16;

            reader.BaseStream.Position = EastWall;
            byte[] wallE = reader.ReadBytes( 60 );

            reader.BaseStream.Position = WestWall;
            byte[] wallW = reader.ReadBytes( 60 );

            reader.BaseStream.Position = SouthWall;
            byte[] wallS = reader.ReadBytes( 60 );

            reader.BaseStream.Position = NorthWall;
            byte[] wallN = reader.ReadBytes( 60 );

            var colors = GetPaletteStandInColors();
            int baseY = 0;
            bool transparent = false;
            int row;
            int col;
            int k;

            for ( int layer = 0; layer < 2; layer++, baseY += 128 )
            {
                k = 0;

                for ( int i = 0; i < 5; i++ )
                {
                    int startX = i * 32;
                    row = 0;
                    col = 0;
                    for ( int j = 0; j < 12; j++, k++ )
                    {
                        int tile = 0;

                        if ( !transparent || row > 0 )
                        {
                            tile = wallS[k];
                            SeekBgTile( reader, UWTileCHR, tile );
                            DrawTile( reader, bmp, colors,
                                startX + col * 8, row * 8 + baseY + 0 );
                        }

                        if ( !transparent || row < 2 )
                        {
                            tile = wallN[k];
                            SeekBgTile( reader, UWTileCHR, tile );
                            DrawTile( reader, bmp, colors,
                                startX + col * 8, row * 8 + baseY + 40 );
                        }

                        row++;
                        if ( row == 3 )
                        {
                            col++;
                            row -= 3;
                        }
                    }
                }

                for ( int i = 0; i < 2; i++ )
                {
                    DrawUWBorderLineH( reader, bmp, colors, 0, i * 8 + baseY + 24, 20 );
                }

                k = 0;

                for ( int i = 0; i < 5; i++ )
                {
                    int startX = i * 32;
                    row = 0;
                    col = 0;
                    for ( int j = 0; j < 12; j++, k++ )
                    {
                        int tile;

                        if ( !transparent || col > 0 )
                        {
                            tile = wallE[k];
                            SeekBgTile( reader, UWTileCHR, tile );
                            DrawTile( reader, bmp, colors,
                                startX + col * 8, row * 8 + baseY + 64 );
                        }

                        if ( !transparent || col < 2 )
                        {
                            tile = wallW[k];
                            SeekBgTile( reader, UWTileCHR, tile );
                            DrawTile( reader, bmp, colors,
                                startX + col * 8 + 8, row * 8 + baseY + 96 );
                        }

                        row++;
                        if ( (row % 2) == 0 )
                        {
                            col++;
                            row -= 2;

                            if ( col == 3 )
                            {
                                col = 0;
                                row = 2;
                            }
                        }
                    }
                    DrawUWBorderLineV( reader, bmp, colors, startX + 24, baseY + 64, 4 );
                    DrawUWBorderLineV( reader, bmp, colors, startX + 0, baseY + 96, 4 );
                }
                transparent = true;
            }
        }

        private static void DrawUWBorderLineH(
            BinaryReader reader, Bitmap bmp, Color[] colors, int x, int y, int length )
        {
            for ( int j = 0; j < length; j++ )
            {
                SeekBgTile( reader, UWTileCHR, 0xF6 );
                DrawTile( reader, bmp, colors, x + j * 8, y );
            }
        }

        private static void DrawUWBorderLineV(
            BinaryReader reader, Bitmap bmp, Color[] colors, int x, int y, int length )
        {
            for ( int j = 0; j < length; j++ )
            {
                SeekBgTile( reader, UWTileCHR, 0xF6 );
                DrawTile( reader, bmp, colors, x, y + j * 8 );
            }
        }

        private static void ExtractOverworldTiles( Options options )
        {
            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                Bitmap bmp = new Bitmap( 16 * 16, 4 * 16 );

                ExtractTiles( reader, bmp );

                bmp.Save( options.MakeOutPath( "overworldTiles.png" ), ImageFormat.Png );
            }
        }

        private static void ExtractTiles( BinaryReader reader, Bitmap bmp )
        {
            reader.BaseStream.Position = PrimarySquareTable;
            var primaries = reader.ReadBytes( 56 );

            reader.BaseStream.Position = SecondarySquareTable;
            var secondaries = reader.ReadBytes( 16 * 4 );       // 16 squares, 4 8x8 tiles each

            reader.BaseStream.Position = SecretSquareTable;
            var secrets = reader.ReadBytes( 6 );                // 1 byte refs to primary squares

            int x = 0;
            int y = 0;

            // Even though the graphics system can make a texture smaller than 16x16, 
            // the texture is really 16x16 underneath. Use this size to calculate the 
            // colors.

            Color[] colors = GetPaletteStandInColors();

            byte[] tileIndexes = new byte[4];

            for ( int i = 0; i < 56; i++ )
            {
                if ( i < 16 )
                {
                    var primary = i * 4;

                    for ( int j = 0; j < 4; j++ )
                        tileIndexes[j] = secondaries[primary + j];

                    DrawBgSquare( reader, bmp, colors, x, y, OWTileCHR, tileIndexes );
                }
                else
                {
                    var primary = primaries[i];

                    if ( primary >= 0xE5 && primary <= 0xEA )
                    {
                        primary = secrets[primary - 0xE5];
                    }

                    for ( int j = 0; j < 4; j++ )
                        tileIndexes[j] = (byte) (primary + j);

                    DrawBgSquare( reader, bmp, colors, x, y, OWTileCHR, tileIndexes );
                }

                if ( (i % 16) == 15 )
                {
                    x = 0;
                    y += 16;
                }
                else
                    x += 16;
            }
        }

        private static bool IsWalkable( int t )
        {
            // DF D5 D2 CC AD AC 9C 91 8D 
            // < 89

            switch ( t )
            {
            case 0xDF:
            case 0xD5:
            case 0xD2:
            case 0xCC:
            case 0xAD:
            case 0xAC:
            case 0x9C:
            case 0x91:
            case 0x8D:
                return true;

            default:
                if ( t < 0x89 )
                    return true;
                return false;
            }
        }

        enum TileAction
        {
            None,
            Push,
            Bomb,
            Burn,
            Headstone,
            Ladder,
            Raft,
            Cave,
            Stairs,
            Ghost,
            Armos,
        }

        private static TileAction GetAction( int t )
        {
            switch ( t )
            {
            case 0x0B: return TileAction.Raft;
            case 0x0C:
            case 0x0F:
                return TileAction.Cave;
            case 0x12: return TileAction.Stairs;
            case 0x14: return TileAction.Ghost;
            case 0x26: return TileAction.Push;
            case 0x27: return TileAction.Bomb;
            case 0x28: return TileAction.Burn;
            case 0x29: return TileAction.Headstone;
            case 0x2A: return TileAction.Armos;
            case 0x2B: return TileAction.Armos;
            case 0x2C: return TileAction.Armos;
            default:
                if ( t >= 0x5 && t <= 0x9
                    || t >= 0x15 && t <= 0x18 )
                    return TileAction.Ladder;
                return TileAction.None;
            }
        }

        // ActionTriggers
        // - None
        // - Init
        // - Push
        // - Touch
        // - Cover

        private static void ExtractOverworldTileAttrs( Options options )
        {
            int[] tileAttrs = new int[56];
            var tileActions = new TileAction[56];

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = PrimarySquareTable;
                var primaries = reader.ReadBytes( 56 );

                reader.BaseStream.Position = SecondarySquareTable;
                var secondaries = reader.ReadBytes( 16 * 4 );       // 16 squares, 4 8x8 tiles each

                reader.BaseStream.Position = SecretSquareTable;
                var secrets = reader.ReadBytes( 6 );                // 1 byte refs to primary squares

                for ( int i = 0; i < 56; i++ )
                {
                    int attr = 0;
                    int walkBit = 1;

                    if ( i < 16 )
                    {
                        var primary = i * 4;

                        for ( int j = 0; j < 4; j++ )
                        {
                            var t = secondaries[primary + j];
                            if ( !IsWalkable( t ) )
                                attr |= walkBit;
                            walkBit <<= 1;
                        }
                    }
                    else
                    {
                        var primary = primaries[i];

                        if ( primary >= 0xE5 && primary <= 0xEA )
                        {
                            primary = secrets[primary - 0xE5];
                        }

                        for ( int j = 0; j < 4; j++ )
                        {
                            var t = (byte) (primary + j);
                            if ( !IsWalkable( t ) )
                                attr |= walkBit;
                            walkBit <<= 1;
                        }
                    }

                    tileAttrs[i] = attr;
                    tileActions[i] = GetAction( i );
                }
            }

            var filePath = options.MakeOutPath( "overworldTileAttrs.dat" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                for ( int i = 0; i < 56; i++ )
                {
                    byte value = (byte) (tileAttrs[i] | ((int) tileActions[i] << 4));
                    writer.Write( value );
                }

                Utility.PadStream( writer.BaseStream );
            }
        }

        private static bool IsUnderworldWalkable( int t )
        {
            if ( t < 0x78 )
                return true;
            return false;
        }

        private static TileAction GetUnderworldAction( int t )
        {
            switch ( t )
            {
                case 4: return TileAction.Stairs;
                case 6: return TileAction.Ladder;
                default: return TileAction.None;
            }
        }

        private static void ExtractUnderworldTileAttrs( Options options )
        {
            int[] tileAttrs = new int[9];
            var tileActions = new TileAction[9];

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = UnderworldSquareTable;
                var primaries = reader.ReadBytes( 8 );

                for ( int i = 0; i < 8; i++ )
                {
                    int attr = 0;
                    int walkBit = 1;
                    var primary = primaries[i];

                    if ( primary < 0x70 || primary > 0xF2 )
                    {
                        for ( int j = 0; j < 4; j++ )
                        {
                            var t = primary;
                            if ( !IsUnderworldWalkable( t ) )
                                attr |= walkBit;
                            walkBit <<= 1;
                        }
                    }
                    else
                    {
                        for ( int j = 0; j < 4; j++ )
                        {
                            var t = (byte) (primary + j);
                            if ( !IsUnderworldWalkable( t ) )
                                attr |= walkBit;
                            walkBit <<= 1;
                        }
                    }

                    tileAttrs[i] = attr;
                    tileActions[i] = GetUnderworldAction( i );
                }
            }

            tileAttrs[8] = 0xF;
            tileActions[8] = TileAction.None;

            var filePath = options.MakeOutPath( "underworldTileAttrs.dat" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                for ( int i = 0; i < tileAttrs.Length; i++ )
                {
                    byte value = (byte) (tileAttrs[i] | ((int) tileActions[i] << 4));
                    writer.Write( value );
                }

                Utility.PadStream( writer.BaseStream );
            }
        }

        private static void ExtractUnderworldCellarTileAttrs( Options options )
        {
            int[] tileAttrs = new int[56];
            var tileActions = new TileAction[56];

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = UWCellarPrimarySquareTable;
                var primaries = reader.ReadBytes( 56 );

                reader.BaseStream.Position = UWCellarSecondarySquareTable;
                var secondaries = reader.ReadBytes( 16 * 4 );       // 16 squares, 4 8x8 tiles each

                // Underworld cellar rooms are layed out like Overworld rooms, but there are no secrets.

                for ( int i = 0; i < 56; i++ )
                {
                    int attr = 0;
                    int walkBit = 1;

                    if ( i < 16 )
                    {
                        var primary = i * 4;

                        for ( int j = 0; j < 4; j++ )
                        {
                            var t = secondaries[primary + j];
                            if ( !IsUnderworldWalkable( t ) )
                                attr |= walkBit;
                            walkBit <<= 1;
                        }
                    }
                    else
                    {
                        var primary = primaries[i];

                        // We don't need to translate secrets into primaries.

                        for ( int j = 0; j < 4; j++ )
                        {
                            var t = (byte) (primary + j);
                            if ( !IsUnderworldWalkable( t ) )
                                attr |= walkBit;
                            walkBit <<= 1;
                        }
                    }

                    tileAttrs[i] = attr;
                    // leave tileAction as None
                }
            }

            var filePath = options.MakeOutPath( "underworldCellarTileAttrs.dat" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                for ( int i = 0; i < 56; i++ )
                {
                    byte value = (byte) (tileAttrs[i] | ((int) tileActions[i] << 4));
                    writer.Write( value );
                }

                Utility.PadStream( writer.BaseStream );
            }
        }

        const int OWRoomCols = 0x15418 + 16;
        const int OWColDir = 0x19D0F + 16;
        const int OWColTables = 0x15BD8 + 16;

        private static MapLayout ExtractOverworldMap( Options options )
        {
            byte[] roomCols = null;
            ushort[] colTablePtrs = null;
            byte[] colTables = null;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = OWRoomCols;
                roomCols = reader.ReadBytes( 124 * 16 );

                reader.BaseStream.Position = OWColDir;
                colTablePtrs = new ushort[16];
                for ( int i = 0; i < 16; i++ )
                {
                    colTablePtrs[i] = (ushort) (reader.ReadUInt16() - colTablePtrs[0]);
                }
                colTablePtrs[0] = 0;

                // There are only 10 columns in the last table
                reader.BaseStream.Position = OWColTables;
                colTables = reader.ReadBytes( 964 );
            }

            var filePath = options.MakeOutPath( "overworldRoomCols.dat" );
            File.WriteAllBytes( filePath, roomCols );

            filePath = options.MakeOutPath( "overworldCols.tab" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                writer.Write( (ushort) colTablePtrs.Length );
                for ( int i = 0; i < colTablePtrs.Length; i++ )
                {
                    ushort ptr = (ushort) (colTablePtrs[i] - colTablePtrs[0]);
                    writer.Write( ptr );
                }

                writer.Write( colTables );

                Utility.PadStream( writer.BaseStream );
            }

            MapLayout mapLayout = new MapLayout();

            mapLayout.uniqueRoomCount = 124;
            mapLayout.columnsInRoom = 16;
            mapLayout.rowsInRoom = 11;
            mapLayout.owLayoutFormat = true;
            mapLayout.roomCols = roomCols;
            mapLayout.colTablePtrs = colTablePtrs;
            mapLayout.colTables = colTables;

            return mapLayout;
        }

        const int UWRoomCols = 0x160DE + 16;
        const int UWColDir = 0x16704 + 16;
        const int UWColTables = 0x162D6 + 16;

        private static MapLayout ExtractUnderworldMap( Options options )
        {
            byte[] roomCols = null;
            ushort[] colTablePtrs = null;
            byte[] colTables = null;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = UWRoomCols;
                roomCols = reader.ReadBytes( 64 * 12 );

                reader.BaseStream.Position = UWColDir;
                colTablePtrs = new ushort[10];
                for ( int i = 0; i < 10; i++ )
                {
                    colTablePtrs[i] = (ushort) (reader.ReadUInt16() - colTablePtrs[0]);
                }
                colTablePtrs[0] = 0;

                // There are only 9 columns in the last table
                reader.BaseStream.Position = UWColTables;
                colTables = reader.ReadBytes( 222 );
            }

            var filePath = options.MakeOutPath( "underworldRoomCols.dat" );
            using ( var stream = Utility.TruncateFile( filePath ) )
            {
                byte[] padding = new byte[4];
                for ( int i = 0; i < 64; i++ )
                {
                    stream.Write( roomCols, i * 12, 12 );
                    stream.Write( padding, 0, padding.Length );
                }
            }

            filePath = options.MakeOutPath( "underworldCols.tab" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                writer.Write( (ushort) colTablePtrs.Length );
                for ( int i = 0; i < colTablePtrs.Length; i++ )
                {
                    ushort ptr = (ushort) (colTablePtrs[i] - colTablePtrs[0]);
                    writer.Write( ptr );
                }

                writer.Write( colTables );

                Utility.PadStream( writer.BaseStream );
            }

            MapLayout mapLayout = new MapLayout();

            mapLayout.uniqueRoomCount = 64;
            mapLayout.columnsInRoom = 12;
            mapLayout.rowsInRoom = 7;
            mapLayout.owLayoutFormat = false;
            mapLayout.roomCols = roomCols;
            mapLayout.colTablePtrs = colTablePtrs;
            mapLayout.colTables = colTables;

            return mapLayout;
        }

        const int UWCellarRoomCols = 0x163B4 + 16;
        const int UWCellarColTables = 0x163D4 + 16;

        private static void ExtractUnderworldCellarMap( Options options )
        {
            byte[] roomCols = null;
            ushort[] colTablePtrs = null;
            byte[] colTables = null;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = UWCellarRoomCols;
                roomCols = reader.ReadBytes( 2 * 16 );

                colTablePtrs = new ushort[1];
                // There's only one column table, so it's at offset 0 in the output heap.

                // There are only 5 columns in the last table
                reader.BaseStream.Position = UWCellarColTables;
                colTables = reader.ReadBytes( 34 );
            }

            var filePath = options.MakeOutPath( "underworldCellarRoomCols.dat" );
            File.WriteAllBytes( filePath, roomCols );

            filePath = options.MakeOutPath( "underworldCellarCols.tab" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                writer.Write( (ushort) colTablePtrs.Length );
                for ( int i = 0; i < colTablePtrs.Length; i++ )
                {
                    ushort ptr = (ushort) (colTablePtrs[i] - colTablePtrs[0]);
                    writer.Write( ptr );
                }

                writer.Write( colTables );

                Utility.PadStream( writer.BaseStream );
            }
        }

        private static void AnalyzeUniqueLayouts( Options options )
        {
            var owLayout = ExtractOverworldMap( options );
            var uwLayout = ExtractUnderworldMap( options );

            Analyzer.AnalyzeUniqueLayouts( owLayout, "OW", options );
            Analyzer.AnalyzeUniqueLayouts( uwLayout, "UW", options );
        }

        // Outer:
        // bits 0-1: palette selector for outer tiles (border)
        // bit  2:   sea sound effect
        // bit  3:   zora
        // bits 4-7: tile column where Link comes out of cave or level

        const int OWOuterRoomAttr = 0x18400 + 16;

        // Inner:
        // bits 0-1: palette selector for inner tiles
        // bits 2-7: cave index

        const int OWInnerRoomAttr = 0x18480 + 16;

        // Overworld Monsters:
        // bits 0-5: low 6 bits of monster list ID
        // bits 6-7: index of enemy count value

        const int OWMonsterListID = 0x18500 + 16;

        // Overworld Rooms (layout):
        // bits 0-6: unique room ID
        // bit  7:   high bit of monster list ID

        const int OWMapLayout = 0x18580 + 16;

        // Caves:

        const int OWCaves = 0x18600 + 16;

        // Other:
        // bits 0-2: tile row where Link comes out of cave or level, starting from row 1
        // bit  3:   enemies from edges
        // bit  4-5: index for position of stairs from rocks
        // bits 6-7: quest secret (0=same in both, 1=quest#1, 2=quest#2)

        const int OWOtherRoomAttr = 0x18680 + 16;

        const int OWMonsterCount = 0x19324 + 16;

        class OWRoomAttrs
        {
            public byte[] monsterCounts;
            public byte[] outer;
            public byte[] inner;
            public byte[] monsterListIDs;
            public byte[] worldLayout;
            public byte[] other;
        }

        private static OWRoomAttrs ReadOverworldRoomAttrs( BinaryReader reader )
        {
            var attrs = new OWRoomAttrs();

            reader.BaseStream.Position = OWMonsterCount;
            attrs.monsterCounts = reader.ReadBytes( 4 );

            reader.BaseStream.Position = OWOuterRoomAttr;
            attrs.outer = reader.ReadBytes( 128 );

            reader.BaseStream.Position = OWInnerRoomAttr;
            attrs.inner = reader.ReadBytes( 128 );

            reader.BaseStream.Position = OWMonsterListID;
            attrs.monsterListIDs = reader.ReadBytes( 128 );

            reader.BaseStream.Position = OWMapLayout;
            attrs.worldLayout = reader.ReadBytes( 128 );

            reader.BaseStream.Position = OWOtherRoomAttr;
            attrs.other = reader.ReadBytes( 128 );

            return attrs;
        }

        private static void WriteConvertedOWRoomAttrs( BinaryWriter writer, int index, OWRoomAttrs roomAttrs )
        {
            // Output format:

            // unique room ID           7
            //
            // index of outer palette   2
            // index of inner palette   2
            // enemy count              4
            // 
            // monster list ID low      6
            // monster list ID high     1
            // 
            // exit column              4
            // exit row (-1)            3
            // 
            // cave index               6
            // quest secrets            2
            // 
            // index shortcut pos       2
            // zora                     1
            // edge enemies             1
            // sea sound                1

            int outerPalette = roomAttrs.outer[index] & 0x03;
            int seaSound = (roomAttrs.outer[index] & 0x04) >> 2;
            int zora = (roomAttrs.outer[index] & 0x08) >> 3;
            int exitColumn = (roomAttrs.outer[index] & 0xF0) >> 4;

            int innerPalette = roomAttrs.inner[index] & 0x03;
            int cave = (roomAttrs.inner[index] & 0xFC) >> 2;

            int monsterListIDLo6 = roomAttrs.monsterListIDs[index] & 0x3F;
            int monsterCountIndex = (roomAttrs.monsterListIDs[index] & 0xC0) >> 6;

            int uniqueRoomID = roomAttrs.worldLayout[index] & 0x7F;
            int monsterListIDHi1 = (roomAttrs.worldLayout[index] & 0x80) >> 7;

            int exitRow = roomAttrs.other[index] & 0x07;
            int edgeMonsters = (roomAttrs.other[index] & 0x08) >> 3;
            int shortcutStairsIndex = (roomAttrs.other[index] & 0x30) >> 4;
            int questSecrets = (roomAttrs.other[index] & 0xC0) >> 6;

            byte b;

            b = (byte) uniqueRoomID;
            writer.Write( b );

            int monsterCount = roomAttrs.monsterCounts[monsterCountIndex] & 0x0F;

            b = (byte) outerPalette;
            b |= (byte) (innerPalette << 2);
            b |= (byte) (monsterCount << 4);
            writer.Write( b );

            b = (byte) monsterListIDLo6;
            b |= (byte) (monsterListIDHi1 << 6);
            writer.Write( b );

            b = (byte) exitColumn;
            b |= (byte) (exitRow << 4);
            writer.Write( b );

            b = (byte) cave;
            b |= (byte) (questSecrets << 6);
            writer.Write( b );

            b = (byte) shortcutStairsIndex;
            b |= (byte) (zora << 2);
            b |= (byte) (edgeMonsters << 3);
            b |= (byte) (seaSound << 4);
            writer.Write( b );

            writer.Write( (byte) 0 );
        }

        private static void ExtractOverworldMapAttrs( Options options )
        {
            OWRoomAttrs roomAttrs = null;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                roomAttrs = ReadOverworldRoomAttrs( reader );
            }

            var filePath = options.MakeOutPath( "overworldRoomAttr.dat" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                for ( int i = 0; i < 128; i++ )
                {
                    WriteConvertedOWRoomAttrs( writer, i, roomAttrs );
                }

                Utility.PadStream( writer.BaseStream );
            }
        }

        // Outer and S/N:
        // bits 0-1: palette selector for outer tiles (border)
        // bit  2-4: S door
        // bit  5-7: N door

        const int UWOuterRoomAttr = 0x18700 + 16;

        // Inner and E/W:
        // bits 0-1: palette selector for inner tiles
        // bits 2-4: E door
        // bits 5-7: W door

        const int UWInnerRoomAttr = 0x18780 + 16;

        // Overworld Monsters:
        // bits 0-5: low 6 bits of monster list ID
        // bits 6-7: index of enemy count value

        const int UWMonsterListID = 0x18800 + 16;

        // Underworld Rooms (layout):
        // bits 0-5: unique room ID
        // bit  6:   push block in room
        // bit  7:   high bit of monster list ID

        const int UWMapLayout = 0x18880 + 16;

        // Items:
        // bits 0-4: item
        // bit  5-6: sound effect
        // bit  7:   dark

        const int UWItemRoomAttr = 0x18900 + 16;

        // Special:
        // bits 0-2: secret trigger and action
        // bit  3:   unused
        // bits 4-5: index of item position
        // bits 6-7: unused

        const int UWSpecialRoomAttr = 0x18980 + 16;

        const int UWMonsterCount = 0x19420 + 16;

        class UWRoomAttrs
        {
            public byte[] monsterCounts;
            public byte[] outerSN;
            public byte[] innerEW;
            public byte[] monsterListIDs;
            public byte[] worldLayout;
            public byte[] items;
            public byte[] special;
        }

        private static UWRoomAttrs ReadUnderworldRoomAttrs( BinaryReader reader, int uwLevelGroup )
        {
            var attrs = new UWRoomAttrs();
            int groupOffset = 768 * uwLevelGroup;

            // The counts are the same for all UW levels.
            reader.BaseStream.Position = UWMonsterCount;
            attrs.monsterCounts = reader.ReadBytes( 4 );

            reader.BaseStream.Position = UWOuterRoomAttr + groupOffset;
            attrs.outerSN = reader.ReadBytes( 128 );

            reader.BaseStream.Position = UWInnerRoomAttr + groupOffset;
            attrs.innerEW = reader.ReadBytes( 128 );

            reader.BaseStream.Position = UWMonsterListID + groupOffset;
            attrs.monsterListIDs = reader.ReadBytes( 128 );

            reader.BaseStream.Position = UWMapLayout + groupOffset;
            attrs.worldLayout = reader.ReadBytes( 128 );

            reader.BaseStream.Position = UWItemRoomAttr + groupOffset;
            attrs.items = reader.ReadBytes( 128 );

            reader.BaseStream.Position = UWSpecialRoomAttr + groupOffset;
            attrs.special = reader.ReadBytes( 128 );

            return attrs;
        }

        private static void WriteConvertedUWRoomAttrs( BinaryWriter writer, int index, UWRoomAttrs roomAttrs )
        {
            // Output format:

            // unique room ID           6
            //
            // index of outer palette   2
            // index of inner palette   2
            // enemy count              4
            // 
            // monster list ID low      6
            // monster list ID high     1
            // 
            // S door                   3
            // N door                   3
            // 
            // E door                   3
            // W door                   3
            // 
            // item                     5
            // index item pos           2
            // 
            // secret                   3
            // push block               1
            // dark                     1
            // sound                    2

            int roomLeft = roomAttrs.outerSN[index];
            int roomRight = roomAttrs.innerEW[index];

            int outerPalette = roomAttrs.outerSN[index] & 0x03;
            int south = (roomAttrs.outerSN[index] >> 2) & 0x07;
            int north = (roomAttrs.outerSN[index] >> 5) & 0x07;

            int innerPalette = roomAttrs.innerEW[index] & 0x03;
            int east = (roomAttrs.innerEW[index] >> 2) & 0x07;
            int west = (roomAttrs.innerEW[index] >> 5) & 0x07;

            int monsterListIDLo6 = roomAttrs.monsterListIDs[index] & 0x3F;
            int monsterCountIndex = (roomAttrs.monsterListIDs[index] >> 6) & 3;

            int uniqueRoomID = roomAttrs.worldLayout[index] & 0x3F;
            int pushBlock = (roomAttrs.worldLayout[index] >> 6) & 1;
            int monsterListIDHi1 = (roomAttrs.worldLayout[index] >> 7) & 1;

            int item = roomAttrs.items[index] & 0x1F;
            int sound = (roomAttrs.items[index] >> 5) & 3;
            int dark = (roomAttrs.items[index] >> 7) & 1;

            int secret = roomAttrs.special[index] & 0x07;
            int itemPosIndex = (roomAttrs.special[index] >> 4) & 3;

            byte b;

            b = (byte) uniqueRoomID;
            writer.Write( b );

            int monsterCount = roomAttrs.monsterCounts[monsterCountIndex] & 0x0F;

            b = (byte) outerPalette;
            b |= (byte) (innerPalette << 2);
            b |= (byte) (monsterCount << 4);
            writer.Write( b );

            b = (byte) monsterListIDLo6;
            b |= (byte) (monsterListIDHi1 << 6);
            writer.Write( b );

            if ( uniqueRoomID == 0x3E || uniqueRoomID == 0x3F )
            {
                b = (byte) roomLeft;
            }
            else
            {
                b = (byte) south;
                b |= (byte) (north << 3);
            }
            writer.Write( b );

            if ( uniqueRoomID == 0x3E || uniqueRoomID == 0x3F )
            {
                b = (byte) roomRight;
            }
            else
            {
                b = (byte) east;
                b |= (byte) (west << 3);
            }
            writer.Write( b );

            b = (byte) item;
            b |= (byte) (itemPosIndex << 5);
            writer.Write( b );

            b = (byte) secret;
            b |= (byte) (pushBlock << 3);
            b |= (byte) (dark << 4);
            b |= (byte) (sound << 5);
            writer.Write( b );
        }

        private static void ExtractUnderworldMapAttrs( Options options )
        {
            for ( int i = 0; i < 4; i++ )
            {
                ExtractUnderworldMapAttrs( options, i );
            }
        }

        private static void ExtractUnderworldMapAttrs( Options options, int uwLevelGroup )
        {
            UWRoomAttrs roomAttrs = null;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                roomAttrs = ReadUnderworldRoomAttrs( reader, uwLevelGroup );
            }

            var filename = string.Format( "underworldRoomAttr{0}.dat", uwLevelGroup );
            var filePath = options.MakeOutPath( filename );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                for ( int i = 0; i < 128; i++ )
                {
                    WriteConvertedUWRoomAttrs( writer, i, roomAttrs );
                }

                Utility.PadStream( writer.BaseStream );
            }
        }

        const int OWArmosStairsRoomCount = 6;
        const int OWArmosStairsRoomId = 0x10CB3 + 16;
        const int OWArmosStairsCol = 0x10CBA + 16;
        const int OWArmosStairsRow = 0x10CE5 + 16;

        const int OWArmosItemRoomId = 0x10CB2 + 16;
        const int OWArmosItemX = 0x10CB9 + 16;
        const int OWArmosItemId = 0x10CF5 + 16;

        const int OWItemRoomId = 0x1789A + 16;
        const int OWItemId = 0x1788A + 16;
        const int OWItemX = 0x1788E + 16;
        const int OWItemY = 0x17890 + 16;   // Unconfirmed

        const int OWShortcutCount = 4;
        const int OWShortcutRoomId = 0x19334 + 16;
        //    1D 23 49 79
        // X  50 40 50 90
        // Y  70 90 70 90
        const int OWShortcutPos = 0x19329 + 16;

        // Stairs pos from recorder: 69

        private static void ExtractOverworldMapSparseAttrs( Options options )
        {
            const int AttrLines = 11;
            const int Alignment = 2;

            byte[] armosStairsRoomIds = null;
            byte[] armosStairsXs = null;
            byte armosSecretY;

            byte armosItemRoomId;
            byte armosItemX;
            byte armosItemId;

            // Dock: 3F 55
            byte[] dockRoomIds = new byte[] { 0x3F, 0x55 };

            // Heart Container: FF X=C0 Y=90 ItemId=1A
            byte itemRoomId;
            byte itemId;
            byte itemX;
            byte itemY;

            byte[] shortcutRoomIds;

            // Mazes: 1B 61
            // exits: 02 01
            const int OWMazePath = 0x6D97 + 16;
            const int OWMazePathLen = 4;

            byte[] mazeRoomIds = new byte[] { 0x61, 0x1B };
            byte[] mazeExitDirs = new byte[] { 0x01, 0x02 };
            byte[][] mazePaths = new byte[2][];

            // Secret sound for shifting map: 1F
            //                     direction: 08
            byte[] secretShiftRoomIds = new byte[] { 0x1F };
            byte[] secretShiftDirs = new byte[] { 0x08 };

            const int OWLadderRoomCount = 6;
            const int OWLadderRoomId = 0x1F20D + 16;

            byte[] ladderRoomIds = null;

            // Fairy pond: 39 43
            byte[] fairyRoomIds = new byte[] { 0x39, 0x43 };

            // L7 pond: 42
            const int OWRecorderRoomCount = 11;
            const int OWRecorderRoomId = 0x1EF66 + 16;

            byte[] recorderRoomIds = null;
            byte[] recorderStairsPositions = null;

            byte[] attrReplacementRoomIds = new byte[] { 0x0B, 0x0E, 0x0F, 0x22, 0x34, 0x3C, 0x74 };
            OWRoomAttrs roomAttrs = null;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                // Armos Stairs

                reader.BaseStream.Position = OWArmosStairsRoomId;
                armosStairsRoomIds = reader.ReadBytes( OWArmosStairsRoomCount );

                reader.BaseStream.Position = OWArmosStairsCol;
                armosStairsXs = reader.ReadBytes( OWArmosStairsRoomCount );

                reader.BaseStream.Position = OWArmosStairsRow;
                armosSecretY = reader.ReadByte();

                // Armos Item

                reader.BaseStream.Position = OWArmosItemRoomId;
                armosItemRoomId = reader.ReadByte();

                reader.BaseStream.Position = OWArmosItemX;
                armosItemX = reader.ReadByte();

                reader.BaseStream.Position = OWArmosItemId;
                armosItemId = reader.ReadByte();

                // Item (Heart Container)

                reader.BaseStream.Position = OWItemRoomId;
                itemRoomId = reader.ReadByte();

                reader.BaseStream.Position = OWItemX;
                itemX = reader.ReadByte();

                reader.BaseStream.Position = OWItemY;
                itemY = reader.ReadByte();

                reader.BaseStream.Position = OWItemId;
                itemId = reader.ReadByte();

                // Shortcuts

                reader.BaseStream.Position = OWShortcutRoomId;
                shortcutRoomIds = reader.ReadBytes( OWShortcutCount );

                // Mazes

                reader.BaseStream.Position = OWMazePath;
                for ( int i = 0; i < mazePaths.Length; i++ )
                    mazePaths[i] = reader.ReadBytes( OWMazePathLen );

                // Ladder

                reader.BaseStream.Position = OWLadderRoomId;
                ladderRoomIds = reader.ReadBytes( OWLadderRoomCount );

                // Recorder

                reader.BaseStream.Position = OWRecorderRoomId;
                recorderRoomIds = reader.ReadBytes( OWRecorderRoomCount );
                recorderStairsPositions = new byte[recorderRoomIds.Length];
                for ( int i = 0; i < recorderRoomIds.Length; i++ )
                    recorderStairsPositions[i] = 0x69;

                // Room attr replacements

                roomAttrs = ReadOverworldRoomAttrs( reader );
                roomAttrs.outer[0x3C] = 0x72;
                roomAttrs.outer[0x74] = 0x72;
                roomAttrs.inner[0x0E] = 0x7B;
                roomAttrs.inner[0x0F] = 0x83;
                roomAttrs.inner[0x22] = 0x84;
                roomAttrs.inner[0x34] = 0x0F;
                roomAttrs.inner[0x3C] = 0x0B;
                roomAttrs.inner[0x45] = 0x12;
                roomAttrs.inner[0x74] = 0x7A;
                roomAttrs.monsterListIDs[0x0B] = 0x2F;
                roomAttrs.worldLayout[0x0B] = 0x7B;
                roomAttrs.worldLayout[0x3C] = 0x7B;
                roomAttrs.worldLayout[0x74] = 0x5A;
                roomAttrs.other[0x3C] = 0x01;
                roomAttrs.other[0x74] = 0x00;
            }

            var filePath = options.MakeOutPath( "overworldRoomSparseAttr.tab" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                var ptrs = new ushort[AttrLines];
                int i = 0;
                int bufBase = (1 + AttrLines) * 2;

                writer.BaseStream.Position = bufBase;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRoomXY( writer, armosStairsRoomIds, armosStairsXs, armosSecretY );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRoomItem( writer, armosItemRoomId, armosItemX, armosSecretY, armosItemId );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRooms( writer, dockRoomIds );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRoomItem( writer, itemRoomId, itemX, itemY, itemId );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRooms( writer, shortcutRoomIds );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRoomMaze( writer, mazeRoomIds, mazeExitDirs, mazePaths );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRoomByte( writer, secretShiftRoomIds, secretShiftDirs );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRooms( writer, ladderRoomIds );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRoomPos( writer, recorderRoomIds, recorderStairsPositions );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRooms( writer, fairyRoomIds );
                i++;

                ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                WriteRoomAttrReplacements( writer, attrReplacementRoomIds, roomAttrs );
                i++;

                // Pad now, because after this we'll seek to the beginning of the file.
                Utility.PadStream( writer.BaseStream );

                writer.BaseStream.Position = 0;
                writer.Write( (ushort) ptrs.Length );
                foreach ( var ptr in ptrs )
                {
                    writer.Write( (ushort) (ptr - bufBase) );
                }
            }
        }

        private static void WriteRoomXY( BinaryWriter writer, byte[] roomIds, byte[] xs, byte y )
        {
            writer.Write( (byte) roomIds.Length );
            writer.Write( (byte) 3 );
            for ( int i = 0; i < roomIds.Length; i++ )
            {
                writer.Write( roomIds[i] );
                writer.Write( xs[i] );
                writer.Write( y );
            }
        }

        private static void WriteRoomItem(
            BinaryWriter writer, byte roomId, byte x, byte y, byte itemId )
        {
            writer.Write( (byte) 1 );
            writer.Write( (byte) 4 );
            writer.Write( roomId );
            writer.Write( x );
            writer.Write( y );
            writer.Write( itemId );
        }

        private static void WriteRooms( BinaryWriter writer, byte[] roomIds )
        {
            writer.Write( (byte) roomIds.Length );
            writer.Write( (byte) 1 );
            foreach ( var id in roomIds )
                writer.Write( id );
        }

        private static void WriteRoomPos( 
            BinaryWriter writer, byte[] roomIds, byte[] positions )
        {
            writer.Write( (byte) roomIds.Length );
            writer.Write( (byte) 2 );
            for ( int i = 0; i < roomIds.Length; i++ )
            {
                writer.Write( roomIds[i] );
                writer.Write( positions[i] );
            }
        }

        private static void WriteRoomMaze( 
            BinaryWriter writer, byte[] roomIds, byte[] exitDirs, byte[][] paths )
        {
            writer.Write( (byte) roomIds.Length );
            writer.Write( (byte) 6 );
            for ( int i = 0; i < roomIds.Length; i++ )
            {
                writer.Write( roomIds[i] );
                writer.Write( exitDirs[i] );
                foreach ( var dir in paths[i] )
                    writer.Write( dir );
            }
        }

        private static void WriteRoomByte( 
            BinaryWriter writer, byte[] roomIds, byte[] bytes )
        {
            writer.Write( (byte) roomIds.Length );
            writer.Write( (byte) 2 );
            for ( int i = 0; i < roomIds.Length; i++ )
            {
                writer.Write( roomIds[i] );
                writer.Write( bytes[i] );
            }
        }

        private static void WriteRoomAttrReplacements(
            BinaryWriter writer, byte[] roomIds, OWRoomAttrs roomAttrs )
        {
            writer.Write( (byte) roomIds.Length );
            writer.Write( (byte) 7 );
            for ( int i = 0; i < roomIds.Length; i++ )
            {
                writer.Write( roomIds[i] );
                WriteConvertedOWRoomAttrs( writer, roomIds[i], roomAttrs );
            }
        }

        const int OWInfoBlock = 0x19300 + 16;
        const int InfoBlockPalettesOffset = 3;
        const int InfoBlockStartY = 0x28;
        const int InfoBlockStartRoomId = 0x2F;
        const int InfoBlockTriforceRoomId = 0x30;
        const int InfoBlockBossRoomId = 0x3E;
        const int InfoBlockLevelNumber = 0x33;
        const int InfoBlockDrawnMapOffset = 0x2D;
        const int InfoBlockCellarRoomIdArray = 0x34;
        const int InfoBlockShortcutPosArray = 0x29;
        const int InfoBlockDrawnMap = 0x3F;
        const int InfoBlockCellarPalette1 = 0x7C;
        const int InfoBlockCellarPalette2 = 0x9C;
        const int InfoBlockDarkPalette = 0xBC;
        const int InfoBlockDeathPalette = 0xDC;

        const int InfoBlockShortcutPosCount = 4;
        const int InfoBlockCellarRoomIdCount = 10;
        const int OWLastSpritePal = 0x1A281 + 16;

        private static void WritePalettes( 
            BinaryWriter writer, byte[] paletteBytes, int paletteByteCount )
        {
            for ( int i = 0; i < paletteByteCount; i++ )
            {
                var colorIndex = paletteBytes[i];
                if ( colorIndex >= DefaultSystemPalette.Colors.Length )
                    colorIndex = 0;
                // ARGB 8888
                // Index 0 is opaque for BG palettes, and transparent for sprites.
                // The first 4 palettes are for BG; the second 4 are for sprites.
                writer.Write( colorIndex );
                // Alpha for BG palettes has to be 0 at color index 0, too, because sprites 
                // can go behind the background.
            }
        }

        private static void ExtractOverworldInfo( Options options )
        {
            var filePath = options.MakeOutPath( "overworldInfo.dat" );

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                const int PaletteByteCount = 8 * 4;

                reader.BaseStream.Position = OWInfoBlock + InfoBlockPalettesOffset;
                byte[] paletteBytes = reader.ReadBytes( PaletteByteCount );

                // Overwrite the last sprite palette, because the original doesn't seem to be used, 
                // but the other one is. It's used by zoras and moblins, and for flashing.
                reader.BaseStream.Position = OWLastSpritePal;
                reader.Read( paletteBytes, 7 * 4, 4 );

                for ( int i = 0; i < PaletteByteCount; i++ )
                {
                    var colorIndex = paletteBytes[i];
                    // ARGB 8888
                    // Index 0 is opaque for BG palettes, and transparent for sprites.
                    // The first 4 palettes are for BG; the second 4 are for sprites.
                    writer.Write( colorIndex );
                    // Alpha for BG palettes has to be 0 at color index 0, too, because sprites 
                    // can go behind the background.
                }

                reader.BaseStream.Position = OWInfoBlock + InfoBlockStartY;
                byte startY = reader.ReadByte();
                writer.Write( startY );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockStartRoomId;
                byte startRoomId = reader.ReadByte();
                writer.Write( startRoomId );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockTriforceRoomId;
                byte triforceRoomId = reader.ReadByte();
                writer.Write( triforceRoomId );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockBossRoomId;
                byte bossRoomId = reader.ReadByte();
                writer.Write( bossRoomId );

                byte song = 2;
                writer.Write( song );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockLevelNumber;
                byte levelNumber = reader.ReadByte();
                writer.Write( levelNumber );

                // The Overworld's effective level number is the same.
                writer.Write( levelNumber );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockDrawnMapOffset;
                byte mapOffset = reader.ReadByte();
                writer.Write( mapOffset );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockCellarRoomIdArray;
                byte[] cellarRoomIds = reader.ReadBytes( InfoBlockCellarRoomIdCount );
                writer.Write( cellarRoomIds );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockShortcutPosArray;
                byte[] shortcutPos = reader.ReadBytes( InfoBlockShortcutPosCount );
                writer.Write( shortcutPos );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockDrawnMap;
                var drawnMap = reader.ReadBytes( 16 );
                writer.Write( drawnMap );

                Utility.PadStream( writer.BaseStream );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockCellarPalette1;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                WritePalettes( writer, paletteBytes, PaletteByteCount );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockCellarPalette2;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                WritePalettes( writer, paletteBytes, PaletteByteCount );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockDarkPalette;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                WritePalettes( writer, paletteBytes, PaletteByteCount );

                reader.BaseStream.Position = OWInfoBlock + InfoBlockDeathPalette;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                writer.Write( paletteBytes );

                Utility.PadStream( writer.BaseStream );
            }

            var dir = new LevelDirectory();
            dir.LevelInfoBlock = "overworldInfo.dat";
            dir.RoomCols = "overworldRoomCols.dat";
            dir.ColTables = "overworldCols.tab";
            dir.TileAttrs = "overworldTileAttrs.dat";
            dir.TilesImage = "overworldTiles.png";
            dir.PlayerImage = "playerItem.png";
            dir.PlayerSheet = "playerItemsSheet.tab";
            dir.NpcImage = "owNpcs.png";
            dir.NpcSheet = "owNpcsSheet.tab";
            dir.RoomAttrs = "overworldRoomAttr.dat";
            dir.LevelInfoEx = "overworldInfoEx.tab";
            dir.ObjLists = "objLists.tab";
            dir.Extra1 = "overworldRoomSparseAttr.tab";
            WriteLevelDir( options, 0, 0, dir );
            WriteLevelDir( options, 1, 0, dir );
        }

        private static void ExtractUnderworldInfo( Options options )
        {
            for ( int quest = 0; quest < 2; quest++ )
            {
                for ( int level = 1; level < 10; level++ )
                {
                    ExtractUnderworldInfo( options, quest, level );
                }
            }
        }

        const int InfoBlockSize = 0xFC;

        static readonly string[] bossImageFilenames = new string[] 
        {
            "",
            "uwBoss1257.png",
            "uwBoss1257.png",
            "uwBoss3468.png",
            "uwBoss3468.png",
            "uwBoss1257.png",
            "uwBoss3468.png",
            "uwBoss1257.png",
            "uwBoss3468.png",
            "uwBoss9.png",
        };

        static readonly string[] bossSheetFilenames = new string[] 
        {
            "",
            "uwBossSheet1257.tab",
            "uwBossSheet1257.tab",
            "uwBossSheet3468.tab",
            "uwBossSheet3468.tab",
            "uwBossSheet1257.tab",
            "uwBossSheet3468.tab",
            "uwBossSheet1257.tab",
            "uwBossSheet3468.tab",
            "uwBossSheet9.tab",
        };

        const int InfoBlockDiffPtrs = 0x183A4 + 0x10;
        const int FirstInfoBlockDiff = 0x1816F + 0x10;

        const int InfoBlockDiffEffectiveLevelNumber = 0xA;
        const int InfoBlockDiffStartRoomId = 6;
        const int InfoBlockDiffTriforceRoomId = 7;
        const int InfoBlockDiffBossRoomId = 0x15;
        const int InfoBlockDiffDrawnMapOffset = 4;
        const int InfoBlockDiffCellarRoomIdArray = 0xB;
        const int InfoBlockDiffDrawnMap = 0x16;
        const int InfoBlockDiffShortcutPosArray = 0;

        private static void ExtractUnderworldInfo( Options options, int quest, int level )
        {
            var filename = string.Format( "levelInfo_{0}_{1}.dat", quest, level );
            var filePath = options.MakeOutPath( filename );
            int effectiveLevel = level;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                int quest2DiffAddr = 0;

                if ( quest == 1 )
                {
                    reader.BaseStream.Position = InfoBlockDiffPtrs;
                    ushort firstPtr = reader.ReadUInt16();
                    reader.BaseStream.Position = InfoBlockDiffPtrs + (level - 1) * 2;
                    ushort ptr = reader.ReadUInt16();
                    quest2DiffAddr = (ptr - firstPtr) + FirstInfoBlockDiff;

                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffEffectiveLevelNumber;
                    effectiveLevel = reader.ReadByte();

                    reader.BaseStream.Position = InfoBlockDiffPtrs + (effectiveLevel - 1) * 2;
                    ptr = reader.ReadUInt16();
                    quest2DiffAddr = (ptr - firstPtr) + FirstInfoBlockDiff;
                }

                const int PaletteByteCount = 8 * 4;
                int blockOffset = InfoBlockSize * effectiveLevel;

                reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockPalettesOffset;
                byte[] paletteBytes = reader.ReadBytes( PaletteByteCount );

                for ( int i = 0; i < PaletteByteCount; i++ )
                {
                    var colorIndex = paletteBytes[i];
                    // ARGB 8888
                    // Index 0 is opaque for BG palettes, and transparent for sprites.
                    // The first 4 palettes are for BG; the second 4 are for sprites.
                    writer.Write( colorIndex );
                    // Alpha for BG palettes has to be 0 at color index 0, too, because sprites 
                    // can go behind the background.
                }

                reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockStartY;
                byte startY = reader.ReadByte();
                writer.Write( startY );

                if ( quest == 0 )
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockStartRoomId;
                else
                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffStartRoomId;
                byte startRoomId = reader.ReadByte();
                writer.Write( startRoomId );

                if ( quest == 0 )
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockTriforceRoomId;
                else
                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffTriforceRoomId;
                byte triforceRoomId = reader.ReadByte();
                writer.Write( triforceRoomId );

                if ( quest == 0 )
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockBossRoomId;
                else
                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffBossRoomId;
                byte bossRoomId = reader.ReadByte();
                writer.Write( bossRoomId );

                byte song = (byte) (level < 9 ? 3 : 7);
                writer.Write( song );

                if ( quest == 0 )
                {
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockLevelNumber;
                    byte levelNumber = reader.ReadByte();
                    writer.Write( levelNumber );
                }
                else
                {
                    writer.Write( (byte) level );
                }

                writer.Write( (byte) effectiveLevel );

                if ( quest == 0 )
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockDrawnMapOffset;
                else
                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffDrawnMapOffset;
                byte mapOffset = reader.ReadByte();
                writer.Write( mapOffset );

                if ( quest == 0 )
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockCellarRoomIdArray;
                else
                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffCellarRoomIdArray;
                byte[] cellarRoomIds = reader.ReadBytes( InfoBlockCellarRoomIdCount );
                if ( quest == 0 && level == 3 )
                    cellarRoomIds[0] = 0x0F;
                writer.Write( cellarRoomIds );

                if ( quest == 0 )
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockShortcutPosArray;
                else
                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffShortcutPosArray;
                byte[] shortcutPos = reader.ReadBytes( InfoBlockShortcutPosCount );
                writer.Write( shortcutPos );

                if ( quest == 0 )
                    reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockDrawnMap;
                else
                    reader.BaseStream.Position = quest2DiffAddr + InfoBlockDiffDrawnMap;
                var drawnMap = reader.ReadBytes( 16 );
                writer.Write( drawnMap );

                Utility.PadStream( writer.BaseStream );

                reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockCellarPalette1;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                WritePalettes( writer, paletteBytes, PaletteByteCount );

                reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockCellarPalette2;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                WritePalettes( writer, paletteBytes, PaletteByteCount );

                reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockDarkPalette;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                WritePalettes( writer, paletteBytes, PaletteByteCount );

                reader.BaseStream.Position = OWInfoBlock + blockOffset + InfoBlockDeathPalette;
                paletteBytes = reader.ReadBytes( PaletteByteCount );
                writer.Write( paletteBytes );

                Utility.PadStream( writer.BaseStream );
            }

            int levelGroup = 0;
            if ( level >= 7 )
                levelGroup++;
            if ( quest == 1 )
                levelGroup += 2;

            string roomAttrFilename = string.Format( "underworldRoomAttr{0}.dat", levelGroup );
            string bossImageFilename = bossImageFilenames[effectiveLevel];
            string bossSheetFilename = bossSheetFilenames[effectiveLevel];

            // TODO: Some of these are for the OW, until we extract the matching UW parts.

            var dir = new LevelDirectory();
            dir.LevelInfoBlock = filename;
            dir.RoomCols = "underworldRoomCols.dat";
            dir.ColTables = "underworldCols.tab";
            dir.TileAttrs = "underworldTileAttrs.dat";
            dir.TilesImage = "underworldTiles.png";
            dir.PlayerImage = "playerItem.png";
            dir.PlayerSheet = "playerItemsSheet.tab";
            dir.NpcImage = "uwNpcs.png";
            dir.NpcSheet = "uwNpcsSheet.tab";
            dir.BossImage = bossImageFilename;
            dir.BossSheet = bossSheetFilename;
            dir.RoomAttrs = roomAttrFilename;
            dir.LevelInfoEx = "overworldInfoEx.tab";
            dir.ObjLists = "objLists.tab";
            dir.Extra1 = "overworldRoomSparseAttr.tab";
            dir.Extra2 = "underworldWalls.png";
            dir.Extra3 = "underworldDoors.png";
            WriteLevelDir( options, quest, level, dir );
        }

        class LevelDirectory
        {
            public string LevelInfoBlock;
            public string RoomCols;
            public string ColTables;
            public string TileAttrs;
            public string TilesImage;
            public string PlayerImage;
            public string PlayerSheet;
            public string NpcImage;
            public string NpcSheet;
            public string BossImage;
            public string BossSheet;
            public string RoomAttrs;
            public string LevelInfoEx;
            public string ObjLists;
            public string Extra1;
            public string Extra2;
            public string Extra3;
            public string Extra4;
        }

        private static void WriteLevelDir( Options options, int quest, int level, LevelDirectory dir )
        {
            const int EntryLength = 32;

            var filePath = options.MakeOutPath( string.Format( "levelDir_{0}_{1}.dat", quest, level ) );

            using ( var stream = Utility.TruncateFile( filePath ) )
            {
                WriteFixedString( stream, dir.LevelInfoBlock, EntryLength );
                WriteFixedString( stream, dir.RoomCols, EntryLength );
                WriteFixedString( stream, dir.ColTables, EntryLength );
                WriteFixedString( stream, dir.TileAttrs, EntryLength );
                WriteFixedString( stream, dir.TilesImage, EntryLength );
                WriteFixedString( stream, dir.PlayerImage, EntryLength );
                WriteFixedString( stream, dir.PlayerSheet, EntryLength );
                WriteFixedString( stream, dir.NpcImage, EntryLength );
                WriteFixedString( stream, dir.NpcSheet, EntryLength );
                WriteFixedString( stream, dir.BossImage, EntryLength );
                WriteFixedString( stream, dir.BossSheet, EntryLength );
                WriteFixedString( stream, dir.RoomAttrs, EntryLength );
                WriteFixedString( stream, dir.LevelInfoEx, EntryLength );
                WriteFixedString( stream, dir.ObjLists, EntryLength );
                WriteFixedString( stream, dir.Extra1, EntryLength );
                WriteFixedString( stream, dir.Extra2, EntryLength );
                WriteFixedString( stream, dir.Extra3, EntryLength );
                WriteFixedString( stream, dir.Extra4, EntryLength );
            }
        }

        private static void WriteFixedString( Stream stream, string s, int length )
        {
            if ( s == null )
                s = "";
            if ( s.Length >= length )
                throw new ArgumentException( "s" );

            var bytes = System.Text.Encoding.ASCII.GetBytes( s );
            stream.Write( bytes, 0, bytes.Length );

            int lenToWrite = length - bytes.Length;

            for ( int i = 0; i < lenToWrite; i++ )
            {
                stream.WriteByte( 0 );
            }
        }

        private delegate void ReadTranslateDelegate( BinaryReader reader, BinaryWriter writer );

        private static void ExtractOverworldInfoEx( Options options )
        {
            var filePath = options.MakeOutPath( "overworldInfoEx.tab" );

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                const int Alignment = 4;
                const int DataLines = 8;

                ReadTranslateDelegate[] converters = new ReadTranslateDelegate[DataLines] 
                {
                    ReadTranslateOWPondColors,
                    ReadTranslateSpawnSpots,
                    ReadTranslateObjAttrs,
                    ReadTranslateCavePalettes,
                    ReadTranslateCaves,
                    ReadTranslateLevelPersonStringIds,
                    ReadTranslateHitPoints,
                    ReadTranslatePlayerDamage,
                };

                var ptrs = new ushort[DataLines];
                int bufBase = (1 + DataLines) * 2;

                writer.BaseStream.Position = bufBase;

                for ( int i = 0; i < converters.Length; i++ )
                {
                    ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                    var converter = converters[i];
                    converter( reader, writer );
                }

                // Pad now, because after this we'll seek to the beginning of the file.
                Utility.PadStream( writer.BaseStream );

                writer.BaseStream.Position = 0;
                writer.Write( (ushort) ptrs.Length );
                foreach ( var ptr in ptrs )
                {
                    writer.Write( (ushort) (ptr - bufBase) );
                }
            }
        }

        private static void ReadTranslateOWPondColors( BinaryReader reader, BinaryWriter writer )
        {
            const int OWPondColorSeq = 0x1FEE8 + 16;

            reader.BaseStream.Position = OWPondColorSeq;
            var colorIndexes = reader.ReadBytes( 12 );

            writer.Write( colorIndexes.Length );
            writer.Write( colorIndexes );
        }

        private static void ReadTranslateSpawnSpots( BinaryReader reader, BinaryWriter writer )
        {
            const int SpawnSpots = 0x1464E + 16;

            reader.BaseStream.Position = SpawnSpots;
            var spots = reader.ReadBytes( 9 * 4 );

            writer.Write( spots.Length );

            for ( int i = 0; i < spots.Length; i++ )
            {
                writer.Write( spots[i] );
            }
        }

        private static void ReadTranslateObjAttrs( BinaryReader reader, BinaryWriter writer )
        {
            const int ObjAttrs = 0x1FAEF + 0x10;

            byte[] fieldLengths = new byte[] { 0, 1 };
            byte[] byHandAttrs = LoadArray8( "ObjAttrs", fieldLengths );
            ushort[] finalAttrs = new ushort[128];

            reader.BaseStream.Position = ObjAttrs;
            byte[] origAttrs = reader.ReadBytes( 128 );

            for ( int i = 0; i < origAttrs.Length; i++ )
            {
                finalAttrs[i] |= origAttrs[i];
            }

            for ( int i = 0; i < byHandAttrs.Length; i++ )
            {
                finalAttrs[i] |= (ushort) (byHandAttrs[i] << 8);
            }

            // Extract the item drop classes.

            for ( int i = 1; i < finalAttrs.Length; i++ )
            {
                finalAttrs[i] |= (4 << 9);
            }

            const int NoDropTypes = 0x1301B + 0x10;
            const int Class1Types = 0x13022 + 0x10;
            const int Class2Types = 0x13028 + 0x10;
            const int Class3Types = 0x13031 + 0x10;

            int[] lengths = new int[] { 7, 6, 9, 9 };
            int[] addrs = new int[]
            {
                NoDropTypes,
                Class1Types,
                Class2Types,
                Class3Types
            };

            for ( int i = 0; i < addrs.Length; i++ )
            {
                reader.BaseStream.Position = addrs[i];
                var types = reader.ReadBytes( lengths[i] );

                for ( int j = 0; j < types.Length; j++ )
                {
                    int type = types[j];
                    finalAttrs[type] = (ushort) (finalAttrs[type] & ~(7 << 9));
                    finalAttrs[type] |= (ushort) (i << 9);
                }
            }

            for ( int i = 0; i < finalAttrs.Length; i++ )
            {
                writer.Write( finalAttrs[i] );
            }
        }

        private static void ReadTranslateCavePalettes( BinaryReader reader, BinaryWriter writer )
        {
            const int OWCavePalettes = 0x1A260 + 16;

            reader.BaseStream.Position = OWCavePalettes;
            var colorIndexes = reader.ReadBytes( 8 );

            writer.Write( (int) 2 );
            writer.Write( (int) 2 );
            WritePalettes( writer, colorIndexes, colorIndexes.Length );
        }

        private static void ReadTranslateCaves( BinaryReader reader, BinaryWriter writer )
        {
            const int OWCaveDwellers = 0x6E6F + 16;
            const int OWCaveStringIds = 0x45A2 + 16;
            const int OWCaveItems = 0x18600 + 16;
            const int OWCavePrices = 0x1863C + 16;

            reader.BaseStream.Position = OWCaveDwellers;
            var types = reader.ReadBytes( 20 );

            reader.BaseStream.Position = OWCaveStringIds;
            var stringIds = reader.ReadBytes( 20 );

            reader.BaseStream.Position = OWCaveItems;
            var items = reader.ReadBytes( 20 * 3 );

            reader.BaseStream.Position = OWCavePrices;
            var prices = reader.ReadBytes( 20 * 3 );

            writer.Write( (int) 20 );

            for ( int i = 0; i < 20; i++ )
            {
                byte origStringAttr = stringIds[i];
                byte stringAttr = (byte) ((origStringAttr & 0xC0) | ((origStringAttr & 0x3F) / 2));

                writer.Write( types[i] );
                writer.Write( stringAttr );
                writer.Write( items, i * 3, 3 );
                writer.Write( prices, i * 3, 3 );
            }
        }

        private static void ReadTranslateLevelPersonStringIds( BinaryReader reader, BinaryWriter writer )
        {
            byte[] stringIds = null;

            reader.BaseStream.Position = 0x4A1B + 0x10;
            stringIds = reader.ReadBytes( 8 );
            for ( int i = 0; i < stringIds.Length; i++ )
                stringIds[i] = (byte) (stringIds[i] / 2);
            writer.Write( stringIds );

            reader.BaseStream.Position = 0x4A61 + 0x10;
            stringIds = reader.ReadBytes( 8 );
            for ( int i = 0; i < stringIds.Length; i++ )
                stringIds[i] = (byte) (stringIds[i] / 2);
            writer.Write( stringIds );

            reader.BaseStream.Position = 0x4A80 + 0x10;
            stringIds = reader.ReadBytes( 4 );
            for ( int i = 0; i < stringIds.Length; i++ )
                stringIds[i] = (byte) (stringIds[i] / 2);
            writer.Write( stringIds );

            for ( int i = 0; i < 4; i++ )
                writer.Write( (byte) 0 );
        }

        private static void ReadTranslateHitPoints( BinaryReader reader, BinaryWriter writer )
        {
            const int EnemyHP = 0x1FB4E + 0x10;

            reader.BaseStream.Position = EnemyHP;
            var hpBytes = reader.ReadBytes( 128 );
            writer.Write( hpBytes );
        }

        private static void ReadTranslatePlayerDamage( BinaryReader reader, BinaryWriter writer )
        {
            const int PlayerDamage = 0x72BA + 0x10;

            reader.BaseStream.Position = PlayerDamage;
            var bytes = reader.ReadBytes( 128 );
            writer.Write( bytes );
        }

        private static void ExtractObjLists( Options options )
        {
            const int ObjListDir = 0x1473F + 16;
            const int ObjLists = 0x14676 + 16;

            byte[] lists = null;
            ushort[] listPtrs = null;

            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                reader.BaseStream.Position = ObjLists;
                lists = reader.ReadBytes( ObjListDir - ObjLists );

                reader.BaseStream.Position = ObjListDir;
                listPtrs = new ushort[30];
                for ( int i = 0; i < listPtrs.Length; i++ )
                {
                    listPtrs[i] = reader.ReadUInt16();
                }
            }

            var filePath = options.MakeOutPath( "objLists.tab" );
            using ( var writer = new BinaryWriter( Utility.TruncateFile( filePath ) ) )
            {
                writer.Write( (ushort) listPtrs.Length );
                for ( int i = 0; i < listPtrs.Length; i++ )
                {
                    ushort ptr = (ushort) (listPtrs[i] - listPtrs[0]);
                    writer.Write( ptr );
                }

                writer.Write( lists );

                Utility.PadStream( writer.BaseStream );
            }
        }

        private static byte[] LoadArray8( string name, byte[] fieldLengths )
        {
            string resName = "ExtractLoz.Data." + name + ".csv";
            using ( var stream = GetResourceStream( resName ) )
            {
                return DatafileReader.ReadHexArray8( stream, fieldLengths );
            }
        }

        private static Color[] GetPaletteStandInColors()
        {
            Color[] colors = new Color[] 
            {
                Color.FromArgb( 0, 0, 0 ),
                Color.FromArgb( 16, 0x80, 0x00 ),
                Color.FromArgb( 32, 0x00, 0x80 ),
                Color.FromArgb( 48, 0x80, 0x80 ),
            };
            return colors;
        }

        private static Color[] GetPaletteContrastColors()
        {
            Color[] colors = new Color[] 
            {
                Color.FromArgb( 0, 0, 0 ),
                Color.FromArgb( 64, 0, 0 ),
                Color.FromArgb( 128, 0, 0 ),
                Color.FromArgb( 192, 0, 0 ),
            };
            return colors;
        }

        private static void ExtractSprites( Options options )
        {
            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                Bitmap bmp = new Bitmap( 8 * 16, 8 * 16 );
                ExtractPlayerItemSprites( reader, bmp );
                bmp.Save( options.MakeOutPath( "playerItem.png" ), ImageFormat.Png );
                WritePlayerItemSpecs( options );

                bmp = new Bitmap( 8 * 16, 8 * 16 );
                ExtractOverworldNpcSprites( reader, bmp );
                bmp.Save( options.MakeOutPath( "owNpcs.png" ), ImageFormat.Png );
                WriteOverworldNpcSpecs( options );

                bmp = new Bitmap( 8 * 16, 8 * 16 );
                ExtractUnderworldNpcSprites( reader, bmp );
                bmp.Save( options.MakeOutPath( "uwNpcs.png" ), ImageFormat.Png );
                WriteUnderworldNpcSpecs( options );

                bmp = new Bitmap( 8 * 16, 8 * 16 );
                ExtractUnderworldBossSpriteGroup( reader, bmp, "UWBossImage1257", UW1257BossCHR, 0 );
                bmp.Save( options.MakeOutPath( "uwBoss1257.png" ), ImageFormat.Png );
                WriteUnderworldBossSpecs( options, "uwBossSheet1257.tab", "UWBossSheet1257.csv" );

                bmp = new Bitmap( 8 * 16, 8 * 16 );
                ExtractUnderworldBossSpriteGroup( reader, bmp, "UWBossImage3468", UW3468BossCHR, 0 );
                bmp.Save( options.MakeOutPath( "uwBoss3468.png" ), ImageFormat.Png );
                WriteUnderworldBossSpecs( options, "uwBossSheet3468.tab", "UWBossSheet3468.csv" );

                bmp = new Bitmap( 8 * 16, 8 * 16 );
                ExtractUnderworldBossSpriteGroup( reader, bmp, "UWBossImage9", UW9BossCHR, 0 );
                bmp.Save( options.MakeOutPath( "uwBoss9.png" ), ImageFormat.Png );
                WriteUnderworldBossSpecs( options, "uwBossSheet9.tab", "UWBossSheet9.csv" );
            }
        }

        private static ushort[,] LoadSpriteMap( string name )
        {
            string resName = "ExtractLoz.Data." + name + ".csv";
            using ( var stream = GetResourceStream( resName ) )
            {
                return DatafileReader.ReadHexMap16( stream );
            }
        }

        private static void ExtractPlayerItemSprites( BinaryReader reader, Bitmap bmp )
        {
            ushort[,] map = LoadSpriteMap( "PlayerItemsImage" );
            Color[] colors = GetPaletteStandInColors();
            int y = 0;

            for ( int r = 0; r < 8; r++ )
            {
                int x = 0;

                for ( int c = 0; c < 16; c++ )
                {
                    int code = map[r, c];
                    int s = code & 0xFF;
                    bool flipX = (code & 0x100) != 0;
                    bool flipY = (code & 0x200) != 0;
                    bool skip = (code & 0x8000) != 0;
                    bool narrow = false;

                    if ( ((code & 0xFF00) == 0x100) && (s >= 0x62) && (s < 0x6C) )
                        narrow = true;

                    if ( narrow )
                        x--;
                    if ( !skip )
                        // We don't need monster sprites yet. So, the chrBase is useless right now.
                        DrawHalfSprite( reader, bmp, colors, x, y, OWMonsterCHR, s, false, flipX, flipY );
                    if ( narrow )
                        x++;
                    x += 8;
                }

                y += 16;
            }

            // Draw the heart from the background tiles.
            DrawHalfSprite( reader, bmp, colors, 11 * 8, 4 * 16, Misc2CHR, 0x8E );

            // Draw the full triforce.
            DrawUWBossHalfSprite( reader, bmp, colors, 0x70, 0x50, UW9BossCHR, 0xF2 );
            DrawUWBossHalfSprite( reader, bmp, colors, 0x78, 0x50, UW9BossCHR, 0xF4 );
        }

        private static void WritePlayerItemSpecs( Options options )
        {
            var outPath = options.MakeOutPath( "playerItemsSheet.tab" );
            using ( var inStream = GetResourceStream( "ExtractLoz.Data.PlayerItemsSheet.csv" ) )
            using ( var outStream = Utility.TruncateFile( outPath ) )
            {
                DatafileReader.ConvertSpriteAnimTable( inStream, outStream );
            }
        }

        private static void ExtractOverworldNpcSprites( BinaryReader reader, Bitmap bmp )
        {
            ushort[,] map = LoadSpriteMap( "OWNpcsImage" );
            Color[] colors = GetPaletteStandInColors();
            int y = 0;

            for ( int r = 0; r < 8; r++ )
            {
                int x = 0;

                for ( int c = 0; c < 16; c++ )
                {
                    int code = map[r, c];
                    int s = code & 0xFF;
                    bool flipX = (code & 0x100) != 0;
                    bool flipY = (code & 0x200) != 0;
                    bool skip = (code & 0x8000) != 0;

                    if ( !skip )
                        // We don't need monster sprites yet. So, the chrBase is useless right now.
                        DrawHalfSprite( reader, bmp, colors, x, y, OWMonsterCHR, s, false, flipX, flipY );
                    x += 8;
                }

                y += 16;
            }
        }

        private static void ExtractUnderworldNpcSprites( BinaryReader reader, Bitmap bmp )
        {
            ExtractUnderworldNpcSpriteGroup( reader, bmp, "UWNpcsImageCommon", CommonUWSprites, 0 );
            ExtractUnderworldNpcSpriteGroup( reader, bmp, "UWNpcsImage127", UW127SpriteCHR, 0x10 );
            ExtractUnderworldNpcSpriteGroup( reader, bmp, "UWNpcsImage358", UW358SpriteCHR, 0x30 );
            ExtractUnderworldNpcSpriteGroup( reader, bmp, "UWNpcsImage469", UW469SpriteCHR, 0x50 );

            // Draw the moldorm ball from the player tiles.
            Color[] colors = GetPaletteStandInColors();
            DrawHalfSprite( reader, bmp, colors, 12 * 8, 0, 0, 0x44 );
        }

        private static void ExtractUnderworldNpcSpriteGroup( 
            BinaryReader reader, Bitmap bmp, string mapResName, int chrBase, int baseY )
        {
            ushort[,] map = LoadSpriteMap( mapResName );
            Color[] colors = GetPaletteStandInColors();
            int y = baseY;

            for ( int r = 0; r < 8; r++ )
            {
                int x = 0;

                for ( int c = 0; c < 16; c++ )
                {
                    int code = map[r, c];
                    int s = code & 0xFF;
                    bool flipX = (code & 0x100) != 0;
                    bool flipY = (code & 0x200) != 0;
                    bool skip = (code & 0x8000) != 0;

                    if ( !skip )
                        DrawUWHalfSprite( reader, bmp, colors, x, y, chrBase, s, false, flipX, flipY );
                    x += 8;
                }

                y += 16;
            }
        }

        private static void ExtractUnderworldBossSpriteGroup(
            BinaryReader reader, Bitmap bmp, string mapResName, int chrBase, int baseY )
        {
            ushort[,] map = LoadSpriteMap( mapResName );
            Color[] colors = GetPaletteStandInColors();
            int y = baseY;

            for ( int r = 0; r < 8; r++ )
            {
                int x = 0;

                for ( int c = 0; c < 16; c++ )
                {
                    int code = map[r, c];
                    int s = code & 0xFF;
                    bool flipX = (code & 0x100) != 0;
                    bool flipY = (code & 0x200) != 0;
                    bool skip = (code & 0x8000) != 0;

                    if ( !skip )
                        DrawUWBossHalfSprite( reader, bmp, colors, x, y, chrBase, s, false, flipX, flipY );
                    x += 8;
                }

                y += 16;
            }
        }

        private static void WriteOverworldNpcSpecs( Options options )
        {
            var outPath = options.MakeOutPath( "owNpcsSheet.tab" );
            using ( var inStream = GetResourceStream( "ExtractLoz.Data.OWNpcsSheet.csv" ) )
            using ( var outStream = Utility.TruncateFile( outPath ) )
            {
                DatafileReader.ConvertSpriteAnimTable( inStream, outStream );
            }
        }

        private static void WriteUnderworldNpcSpecs( Options options )
        {
            var outPath = options.MakeOutPath( "uwNpcsSheet.tab" );
            using ( var inStream = GetResourceStream( "ExtractLoz.Data.UWNpcsSheet.csv" ) )
            using ( var outStream = Utility.TruncateFile( outPath ) )
            {
                DatafileReader.ConvertSpriteAnimTable( inStream, outStream );
            }
        }

        private static void WriteUnderworldBossSpecs( Options options, string fileName, string resName )
        {
            var outPath = options.MakeOutPath( fileName );
            using ( var inStream = GetResourceStream( "ExtractLoz.Data." + resName ) )
            using ( var outStream = Utility.TruncateFile( outPath ) )
            {
                DatafileReader.ConvertSpriteAnimTable( inStream, outStream );
            }
        }

        private static void ExtractOWSpriteVRAM( Options options )
        {
            using ( var reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                Bitmap bmp = new Bitmap( 8 * 16, 8 * 16 );
                Color[] colors = GetPaletteContrastColors();
                int y = 0;
                int i = 0;

                for ( int r = 0; r < 16; r++ )
                {
                    int x = 0;

                    for ( int c = 0; c < 16; c++ )
                    {
                        DrawSpriteTile( reader, bmp, colors, x, y, OWMonsterCHR, i );
                        i++;
                        x += 8;
                    }
                    y += 8;
                }

                bmp.Save( options.MakeOutPath( "owSpriteVRAM.png" ), ImageFormat.Png );
            }
        }

        private static void DrawTile( BinaryReader reader, Bitmap bmp, Color[] colors, int x, int y,
            bool transparent = false, bool flipX = false, bool flipY = false )
        {
            // Read a whole tile's pixel data
            reader.Read( tileBuf, 0, tileBuf.Length );

            for ( int v = 0; v < 8; v++ )
            {
                for ( int u = 0; u < 8; u++ )
                {
                    int lo = (tileBuf[v] >> (7 - u)) & 1;
                    int hi = (tileBuf[v + 8] >> (7 - u)) & 1;
                    int pixel = lo | (hi << 1);
                    Color color = colors[pixel];

                    int xOffset = flipX ? (7 - u) : u;
                    int yOffset = flipY ? (7 - v) : v;

                    if ( pixel != 0 || !transparent )
                        bmp.SetPixel( x + xOffset, y + yOffset, color );
                }
            }
        }

        static readonly int[] TileXOffset = new int[] { 0, 0, 8, 8 };
        static readonly int[] TileYOffset = new int[] { 0, 8, 0, 8 };

        private static void DrawBgSquare( BinaryReader reader, Bitmap bmp, Color[] colors, int x, int y,
            int chrBase, byte[] indexes,
            bool transparent = false )
        {
            for ( int i = 0; i < 4; i++ )
            {
                int t = indexes[i];

                if ( t < 0x70 )
                {
                    reader.BaseStream.Position = Misc1CHR + t * TileSize;
                }
                else if ( t >= 0xF2 )
                {
                    reader.BaseStream.Position = Misc2CHR + (t - 0xF2) * TileSize;
                }
                else
                {
                    reader.BaseStream.Position = chrBase + (t - 0x70) * TileSize;
                }

                DrawTile( reader, bmp, colors, x + TileXOffset[i], y + TileYOffset[i], transparent );
            }
        }

        private static void SeekBgTile( BinaryReader reader, int chrBase, int t )
        {
            if ( t < 0x70 )
            {
                reader.BaseStream.Position = Misc1CHR + t * TileSize;
            }
            else if ( t >= 0xF2 )
            {
                reader.BaseStream.Position = Misc2CHR + (t - 0xF2) * TileSize;
            }
            else
            {
                reader.BaseStream.Position = chrBase + (t - 0x70) * TileSize;
            }
        }

        private static void SeekUWSpriteTile( BinaryReader reader, int chrBase, int t )
        {
            if ( t < 0x70 )
            {
                reader.BaseStream.Position = LinkCHR + t * TileSize;
            }
            else if ( t < 0x8E )
            {
                reader.BaseStream.Position = Common2CHR + (t - 0x70) * TileSize;
            }
            else if ( t < 0x9E )
            {
                reader.BaseStream.Position = CommonUWSprites + (t - 0x8E) * TileSize;
            }
            else
            {
                reader.BaseStream.Position = chrBase + (t - 0x9E) * TileSize;
            }
        }

        private static void SeekUWBossTile( BinaryReader reader, int chrBase, int t )
        {
            if ( t < 0x70 )
            {
                reader.BaseStream.Position = LinkCHR + t * TileSize;
            }
            else if ( t < 0x8E )
            {
                reader.BaseStream.Position = Common2CHR + (t - 0x70) * TileSize;
            }
            else if ( t < 0x9E )
            {
                reader.BaseStream.Position = CommonUWSprites + (t - 0x8E) * TileSize;
            }
            else if ( t < 0xC0 )
            {
                throw new InvalidOperationException();
            }
            else
            {
                reader.BaseStream.Position = chrBase + (t - 0xC0) * TileSize;
            }
        }

        const int LinkCHR = 0x807F + 16;
        const int Common2CHR = 0x4DB4 + 16;
        const int CommonUWSprites = 0xDCBB + 16;
        const int OWMonsterCHR = 0xD15B + 16;
        const int UW358SpriteCHR = 0xD87B + 16;
        const int UW469SpriteCHR = 0xDA9B + 16;
        const int UW127SpriteCHR = 0xDDBB + 16;
        const int UW1257BossCHR = 0xDFDB + 16;
        const int UW3468BossCHR = 0xE3DB + 16;
        const int UW9BossCHR = 0xE7DB + 16;

        private static void DrawHalfSprite( BinaryReader reader, Bitmap bmp, Color[] colors,
            int x, int y,
            int chrBase, int index,
            bool transparent = false, bool flipX = false, bool flipY = false )
        {
            if ( flipY )
            {
                DrawSpriteTile( reader, bmp, colors, x, y + 8, chrBase, index, transparent, flipX, flipY );
                DrawSpriteTile( reader, bmp, colors, x, y, chrBase, index + 1, transparent, flipX, flipY );
            }
            else
            {
                DrawSpriteTile( reader, bmp, colors, x, y, chrBase, index, transparent, flipX, flipY );
                DrawSpriteTile( reader, bmp, colors, x, y + 8, chrBase, index + 1, transparent, flipX, flipY );
            }
        }

        private static void DrawSpriteTile( BinaryReader reader, Bitmap bmp, Color[] colors,
            int x, int y,
            int chrBase, int index,
            bool transparent = false, bool flipX = false, bool flipY = false )
        {
            int t = index;

            if ( t < 0x70 )
            {
                reader.BaseStream.Position = LinkCHR + t * TileSize;
            }
            else if ( t < 0x8E )
            {
                reader.BaseStream.Position = Common2CHR + (t - 0x70) * TileSize;
            }
            else
            {
                reader.BaseStream.Position = chrBase + (t - 0x8E) * TileSize;
            }

            DrawTile( reader, bmp, colors, x, y, transparent, flipX, flipY );
        }

        private static void DrawUWHalfSprite( BinaryReader reader, Bitmap bmp, Color[] colors,
            int x, int y,
            int chrBase, int index,
            bool transparent = false, bool flipX = false, bool flipY = false )
        {
            if ( flipY )
            {
                DrawUWSpriteTile( reader, bmp, colors, x, y + 8, chrBase, index, transparent, flipX, flipY );
                DrawUWSpriteTile( reader, bmp, colors, x, y, chrBase, index + 1, transparent, flipX, flipY );
            }
            else
            {
                DrawUWSpriteTile( reader, bmp, colors, x, y, chrBase, index, transparent, flipX, flipY );
                DrawUWSpriteTile( reader, bmp, colors, x, y + 8, chrBase, index + 1, transparent, flipX, flipY );
            }
        }

        private static void DrawUWSpriteTile( BinaryReader reader, Bitmap bmp, Color[] colors,
            int x, int y,
            int chrBase, int index,
            bool transparent = false, bool flipX = false, bool flipY = false )
        {
            SeekUWSpriteTile( reader, chrBase, index );
            DrawTile( reader, bmp, colors, x, y, transparent, flipX, flipY );
        }

        private static void DrawUWBossHalfSprite( BinaryReader reader, Bitmap bmp, Color[] colors,
            int x, int y,
            int chrBase, int index,
            bool transparent = false, bool flipX = false, bool flipY = false )
        {
            if ( flipY )
            {
                DrawUWBossTile( reader, bmp, colors, x, y + 8, chrBase, index, transparent, flipX, flipY );
                DrawUWBossTile( reader, bmp, colors, x, y, chrBase, index + 1, transparent, flipX, flipY );
            }
            else
            {
                DrawUWBossTile( reader, bmp, colors, x, y, chrBase, index, transparent, flipX, flipY );
                DrawUWBossTile( reader, bmp, colors, x, y + 8, chrBase, index + 1, transparent, flipX, flipY );
            }
        }

        private static void DrawUWBossTile( BinaryReader reader, Bitmap bmp, Color[] colors,
            int x, int y,
            int chrBase, int index,
            bool transparent = false, bool flipX = false, bool flipY = false )
        {
            SeekUWBossTile( reader, chrBase, index );
            DrawTile( reader, bmp, colors, x, y, transparent, flipX, flipY );
        }

        private static Stream GetResourceStream( string name )
        {
            var asm = System.Reflection.Assembly.GetExecutingAssembly();
            return asm.GetManifestResourceStream( name );
        }
    }
}
