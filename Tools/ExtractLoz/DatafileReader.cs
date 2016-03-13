/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using Microsoft.VisualBasic.FileIO;

namespace ExtractLoz
{
    static class DatafileReader
    {
        delegate object ReadMTableItem( string[] fields, int index );
        delegate void WriteMTableItem( BinaryWriter writer, object item );
        delegate int GetMTableItemSize( object item );

        static class MTableReader
        {
            public class Node
            {
                public string Name;
                public object Children;
            }

            private static Node ReadMTable( Stream stream, ReadMTableItem readItem )
            {
                var root = new Node();

                using ( var parser = new TextFieldParser( stream ) )
                {
                    parser.SetDelimiters( "," );
                    parser.ReadLine();

                    while ( !parser.EndOfData )
                    {
                        var fields = parser.ReadFields();
                        var curLevel = root;

                        int i = 0;
                        for ( i = 0; i < 10 && fields[i] != ":"; i++ )
                        {
                            if ( fields[i] == "" )
                                continue;

                            string key = fields[i];
                            Node child;
                            if ( curLevel.Children == null )
                            {
                                curLevel.Children = new List<Node>();
                            }
                            var children = (List<Node>) curLevel.Children;
                            if ( children.Count == 0 || key != children[children.Count - 1].Name )
                            {
                                child = new Node();
                                child.Name = key;
                                children.Add( child );
                                curLevel = child;
                            }
                            else
                            {
                                curLevel = children[children.Count - 1];
                            }
                        }

                        for ( i++; i < fields.Length && fields[i] == ""; i++ )
                        {
                        }

                        curLevel.Children = readItem( fields, i );
                    }
                }
                return root;
            }

            private static void WriteMTable(
                Stream stream, Node root, WriteMTableItem writeItem, GetMTableItemSize getItemSize )
            {
                using ( var writer = new BinaryWriter( stream ) )
                {
                    var itemList = (List<Node>) root.Children;
                    int basePtr = (1 + itemList.Count) * 2;
                    var ptrs = new ushort[itemList.Count];

                    writer.BaseStream.Position = basePtr;

                    for ( int i = 0; i < itemList.Count; i++ )
                    {
                        var node = itemList[i];
                        var dimSizes = new List<int>();
                        while ( true )
                        {
                            var list = node.Children as List<Node>;
                            if ( list != null )
                            {
                                dimSizes.Add( list.Count );
                                Debug.Assert( list.Count > 0 );
                                node = list[0];
                            }
                            else
                            {
                                int size = getItemSize( node.Children );
                                dimSizes.Add( size );
                                break;
                            }
                        }
                        for ( int j = dimSizes.Count - 2; j >= 0; j-- )
                        {
                            dimSizes[j] *= dimSizes[j + 1];
                        }
                        ptrs[i] = (ushort) writer.BaseStream.Position;
                        writer.Write( (byte) (dimSizes.Count - 1) );
                        for ( int j = 1; j < dimSizes.Count; j++ )
                        {
                            writer.Write( (byte) dimSizes[j] );
                        }
                        WriteArrayDim( writer, itemList[i], writeItem );
                    }

                    Utility.PadStream( writer.BaseStream );

                    writer.BaseStream.Position = 0;
                    writer.Write( (ushort) itemList.Count );
                    foreach ( var ptr in ptrs )
                    {
                        writer.Write( (ushort) (ptr - basePtr) );
                    }
                }
            }

            private static void WriteArrayDim(
                BinaryWriter writer, Node node, WriteMTableItem writeItem )
            {
                var list = node.Children as List<Node>;

                if ( list != null )
                {
                    foreach ( var child in list )
                    {
                        WriteArrayDim( writer, child, writeItem );
                    }
                }
                else
                {
                    writeItem( writer, node.Children );
                }
            }

            public static void ConvertMTable(
                Stream inStream,
                Stream outStream,
                ReadMTableItem readItem,
                WriteMTableItem writeItem,
                GetMTableItemSize getItemSize )
            {
                var root = ReadMTable( inStream, readItem );
                WriteMTable( outStream, root, writeItem, getItemSize );
            }
        }

        static class TableReader
        {
            public class Node
            {
                public string Name;
                public object Child;
            }

            private static List<Node> ReadTable( Stream stream, ReadMTableItem readItem )
            {
                var list = new List<Node>();

                using ( var parser = new TextFieldParser( stream ) )
                {
                    parser.SetDelimiters( "," );
                    parser.ReadLine();              // Skip the header

                    while ( !parser.EndOfData )
                    {
                        var fields = parser.ReadFields();
                        var key = fields[0];
                        var node = new Node();
                        node.Name = key;
                        node.Child = readItem( fields, 1 );
                        list.Add( node );
                    }
                }
                return list;
            }

            private static void WriteTable(
                Stream stream, List<Node> itemList, WriteMTableItem writeItem, GetMTableItemSize getItemSize )
            {
                const int Alignment = 2;

                using ( var writer = new BinaryWriter( stream ) )
                {
                    int basePtr = (1 + itemList.Count) * 2;
                    var ptrs = new ushort[itemList.Count];

                    writer.BaseStream.Position = basePtr;

                    for ( int i = 0; i < itemList.Count; i++ )
                    {
                        var node = itemList[i];
                        ptrs[i] = (ushort) Utility.AlignStream( writer.BaseStream, Alignment );
                        writeItem( writer, node.Child );
                    }

                    Utility.PadStream( writer.BaseStream );

                    writer.BaseStream.Position = 0;
                    writer.Write( (ushort) itemList.Count );
                    foreach ( var ptr in ptrs )
                    {
                        writer.Write( (ushort) (ptr - basePtr) );
                    }
                }
            }

            public static void ConvertTable(
                Stream inStream,
                Stream outStream,
                ReadMTableItem readItem,
                WriteMTableItem writeItem,
                GetMTableItemSize getItemSize )
            {
                var list = ReadTable( inStream, readItem );
                WriteTable( outStream, list, writeItem, getItemSize );
            }
        }

        private static SpriteAnim ReadSpriteAnim( string[] fields, int index )
        {
            int i = index;
            int count = int.Parse( fields[i] );
            var anim = new SpriteAnim();
            anim.Frames = new SpriteFrame[count];
            anim.Width = byte.Parse( fields[i + 1] );
            anim.Height = byte.Parse( fields[i + 2] );
            i += 3;

            for ( int j = 0; j < count; j++ )
            {
                for ( ; i < fields.Length && fields[i] == ""; i++ )
                {
                }

                var frame = new SpriteFrame();
                frame.X = byte.Parse( fields[i] );
                frame.Y = byte.Parse( fields[i + 1] );
                frame.Flags = byte.Parse( fields[i + 2] );
                i += 3;
                anim.Frames[j] = frame;
            }

            return anim;
        }

        private static void WriteSpriteAnim( BinaryWriter writer, object obj )
        {
            var anim = (SpriteAnim) obj;

            writer.Write( (byte) anim.Frames.Length );
            writer.Write( anim.Width );
            writer.Write( anim.Height );

            foreach ( var frame in anim.Frames )
            {
                writer.Write( frame.X );
                writer.Write( frame.Y );
                writer.Write( frame.Flags );
            }
        }

        private static int GetSpriteAnimSize( object obj )
        {
            var anim = (SpriteAnim) obj;
            int size = 3 + anim.Frames.Length * 3;
            return size;
        }

        public static void ConvertSpriteAnimMTable( Stream inStream, Stream outStream )
        {
            MTableReader.ConvertMTable(
                inStream, outStream, ReadSpriteAnim, WriteSpriteAnim, GetSpriteAnimSize );
        }

        public static void ConvertSpriteAnimTable( Stream inStream, Stream outStream )
        {
            TableReader.ConvertTable(
                inStream, outStream, ReadSpriteAnim, WriteSpriteAnim, GetSpriteAnimSize );
        }

        public static ushort[,] ReadHexMap16( Stream stream )
        {
            ushort[,] map = null;
            var lines = new List<string[]>();

            using ( var parser = new TextFieldParser( stream ) )
            {
                parser.SetDelimiters( "," );

                while ( !parser.EndOfData )
                {
                    var fields = parser.ReadFields();
                    lines.Add( fields );
                }
            }

            int rowCount = lines.Count;
            int colCount = lines[0].Length;
            map = new ushort[rowCount, colCount];

            for ( int r = 0; r < rowCount; r++ )
            {
                var line = lines[r];
                for ( int c = 0; c < colCount; c++ )
                {
                    map[r, c] = ushort.Parse( line[c], NumberStyles.HexNumber );
                }
            }
            return map;
        }

        public static byte[] ReadHexArray8( Stream stream, byte[] fieldLengths )
        {
            byte[] array = null;
            var lines = new List<string[]>();

            using ( var parser = new TextFieldParser( stream ) )
            {
                parser.SetDelimiters( "," );
                // Throw away the header line.
                parser.ReadLine();

                while ( !parser.EndOfData )
                {
                    var fields = parser.ReadFields();
                    lines.Add( fields );
                }
            }

            int rowCount = lines.Count;
            array = new byte[rowCount];

            for ( int r = 0; r < rowCount; r++ )
            {
                var line = lines[r];
                byte b = 0;
                int offset = 0;
                for ( int c = 0; c < line.Length && c < fieldLengths.Length; c++ )
                {
                    if ( fieldLengths[c] == 0 )
                        continue;
                    int mask = (1 << fieldLengths[c]) - 1;
                    int f = byte.Parse( line[c], NumberStyles.HexNumber );
                    f &= mask;
                    f <<= offset;
                    b = (byte) (b | f);
                    offset += fieldLengths[c];
                }
                array[r] = b;
            }
            return array;
        }

        public delegate T ReadTableItem<T>( string[] fields );

        public static List<T> ReadTable<T>( Stream stream, ReadTableItem<T> readItem )
        {
            var list = new List<T>();

            using ( var parser = new TextFieldParser( stream ) )
            {
                parser.SetDelimiters( "," );
                parser.ReadLine();              // Skip the header

                while ( !parser.EndOfData )
                {
                    var fields = parser.ReadFields();
                    var node = readItem( fields );
                    list.Add( node );
                }
            }
            return list;
        }
    }
}
