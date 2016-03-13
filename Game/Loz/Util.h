/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum Direction;


namespace Util
{
    template <typename T>
    T Max( T a, T b )
    {
        if ( a > b )
            return a;
        return b;
    }

    bool IsPerpendicular( Direction dir1, Direction dir2 );
    Direction GetOppositeDir( Direction dir );
    int GetDirectionOrd( Direction dir );
    int GetDirectionBit( Direction dir );
    Direction GetOrdDirection( int ord );

    int GetDirection8Ord( Direction dir );
    Direction GetDirection8( int ord );
    Direction GetOppositeDir8( Direction dir );
    Direction GetNextDirection8( Direction dir );
    Direction GetPrevDirection8( Direction dir );
    bool IsVertical( Direction dir );
    bool IsHorizontal( Direction dir );
    bool IsGrowingDir( Direction dir );

    const float _2_PI           =  6.283185307179586;
    const float PI_OVER_16      =  0.196349540849362;
    const float PI_OVER_8       =  0.392699081698724;
    const float NEG_PI_OVER_8   = -0.392699081698724;

    int GetSector16( float x, float y );
    void Rotate( float angle, float& x, float& y );
    void PolarToCart( float angle, float distance, float& x, float& y );

    int GetRandom( int max );
    void MoveSimple( int& x, int& y, Direction dir, int speed );
    void MoveSimple( uint8_t& x, uint8_t& y, Direction dir, int speed );
    void MoveSimple8( float& x, float& y, Direction dir, float speed );
    void MoveSimple8( int& x, int& y, Direction dir, int speed );
    void MoveSimple8( uint8_t& x, uint8_t& y, Direction dir, int speed );

    template <typename T>
    void ShuffleArray( T* array, int length )
    {
        for ( int i = length - 1; i > 0; i-- )
        {
            int j = GetRandom( i + 1 );
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }


    template <class T>
    struct Array
    {
        const uint Count;
        const T*   Elems;

        Array( uint count, const T* elems )
            :   Count( count ),
                Elems( elems )
        {
        }
    };


    class ResourceLoader
    {
    public:
        virtual bool Load( FILE* file, size_t fileSize ) = 0;
    };


    template <typename T, int Length>
    class BlobResLoader : public ResourceLoader
    {
        T*  blob;

    public:
        BlobResLoader( T* blob )
            :   blob( blob )
        {
            assert( blob != nullptr );
        }

        virtual bool Load( FILE* file, size_t fileSize ) override
        {
            assert( file != nullptr );
            assert( sizeof( T ) * Length <= fileSize );

            fread( blob, sizeof( T ), Length, file );

            return true;
        }
    };


    template <typename T>
    bool LoadList( const char* filename, T* list, size_t length )
    {
        FILE* file = nullptr;

        errno_t err = fopen_s( &file, filename, "rb" );
        if ( err != 0 )
            return false;

        fread( list, sizeof T, length, file );
        fclose( file );

        return true;
    }


    template <typename T>
    class Table : public ResourceLoader
    {
        size_t      length;
        uint16_t*   offsets;
        uint8_t*    heap;

    public:
        Table()
            :   offsets( nullptr ),
                heap( nullptr )
        {
        }

        ~Table()
        {
            Free();
        }

        virtual bool Load( FILE* file, size_t fileSize ) override
        {
            assert( file != nullptr );
            Free();

            uint16_t len16;
            int size = fileSize - sizeof len16;

            uint8_t* buffer = new uint8_t[size];
            if ( buffer == nullptr )
                return false;

            fread( &len16, 1, sizeof len16, file );
            fread( buffer, 1, size, file );

            length = len16;
            offsets = (uint16_t*) buffer;
            heap = buffer + length * sizeof offsets[0];

            return true;
        }

        const T* GetItem( size_t index )
        {
            assert( index < length );
            if ( index >= length )
                return nullptr;

            return (T*) (heap + offsets[index]);
        }

        size_t GetLength()
        {
            return length;
        }

    private:
        void Free()
        {
            if ( offsets != nullptr )
            {
                delete [] offsets;
                offsets = nullptr;
                heap = nullptr;
            }
        }
    };


    template <typename T>
    class MTable : public ResourceLoader
    {
        int         length;
        uint8_t*    buffer;

    public:
        MTable()
            :   buffer( nullptr )
        {
        }

        ~MTable()
        {
            delete [] buffer;
        }

        virtual bool Load( FILE* file, size_t fileSize ) override
        {
            assert( file != nullptr );
            assert( buffer == nullptr );

            uint16_t    len16;
            int         size = fileSize - sizeof len16;

            buffer = new uint8_t[size];
            if ( buffer == nullptr )
                return false;

            fread( &len16, 1, sizeof len16, file );
            fread( buffer, 1, size, file );
            length = len16;

            return true;
        }

        const T* GetItem( size_t index )
        {
            if ( index >= GetLength() )
                return nullptr;

            uint8_t* itemBase = GetItemBase( index );
            assert( *itemBase == 0 );
            return (T*) &itemBase[1];
        }

        const T* GetItem( size_t index, int i0 )
        {
            if ( index >= GetLength() )
                return nullptr;

            int offset = 2;
            uint8_t* itemBase = GetItemBase( index );
            assert( *itemBase == 1 );
            offset += i0 * itemBase[1];
            return (T*) &itemBase[offset];
        }

        const T* GetItem( size_t index, int i0, int i1 )
        {
            if ( index >= GetLength() )
                return nullptr;

            int offset = 3;
            uint8_t* itemBase = GetItemBase( index );
            assert( *itemBase == 2 );
            offset += i0 * itemBase[1];
            offset += i1 * itemBase[2];
            return (T*) &itemBase[offset];
        }

        size_t GetLength()
        {
            return length;
        }

    private:
        uint8_t* GetItemBase( size_t index )
        {
            return buffer + (length * 2) + ((uint16_t*) buffer)[index];
        }
    };


    bool LoadResource( const char* filename, ResourceLoader* loader );


    template <typename TTable>
    const void* FindSparseAttr( TTable& table, int attrId, int elemId )
    {
        const uint8_t*  valueArray = (uint8_t*) table.GetItem( attrId );
        int             count = valueArray[0];
        int             elemSize = valueArray[1];
        const uint8_t*  elem = &valueArray[2];

        for ( int i = 0; i < count; i++, elem += elemSize )
        {
            if ( *elem == elemId )
                return elem;
        }

        return nullptr;
    }
}


inline Direction operator |( Direction left, Direction right )
{
    return (Direction) ((unsigned int) left | (unsigned int) right);
}

inline Direction operator &( Direction left, Direction right )
{
    return (Direction) ((unsigned int) left & (unsigned int) right);
}

inline Direction operator ^( Direction left, Direction right )
{
    return (Direction) ((unsigned int) left ^ (unsigned int) right);
}

inline uint abs( uint a )
{
    return abs( (int) a );
}
