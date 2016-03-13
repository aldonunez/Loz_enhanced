/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Util.h"

namespace Util
{

bool LoadResource( const char* filename, ResourceLoader* loader )
{
    FILE* file = nullptr;

    errno_t err = fopen_s( &file, filename, "rb" );
    if ( err != 0 )
        return false;

    fseek( file, 0, SEEK_END );
    int fileSize = ftell( file );
    fseek( file, 0, SEEK_SET );

    if ( !loader->Load( file, fileSize ) )
        return false;

    fclose( file );

    return true;
}

bool IsPerpendicular( Direction dir1, Direction dir2 )
{
    switch ( dir1 )
    {
    case Dir_Right:
    case Dir_Left:
        return dir2 == Dir_Up || dir2 == Dir_Down;

    case Dir_Down:
    case Dir_Up:
        return dir2 == Dir_Left || dir2 == Dir_Right;
    }
    return false;
}

Direction GetOppositeDir( Direction dir )
{
    switch ( dir )
    {
    case Dir_Right: return Dir_Left;
    case Dir_Left:  return Dir_Right;
    case Dir_Down:  return Dir_Up;
    case Dir_Up:    return Dir_Down;
    }
    return Dir_None;
}

int GetDirectionOrd( Direction dir )
{
    // ORIGINAL: the original game goes in the opposite order.

    for ( int i = 0; i < 4; i++ )
    {
        if ( (dir & 1) != 0 )
            return i;
        dir = (Direction) (dir >> 1);
    }

    return 0;
}

int GetDirectionBit( Direction dir )
{
    return dir;
}

Direction GetOrdDirection( int ord )
{
    // ORIGINAL: the original game goes in the opposite order.
    return (Direction) (1 << ord);
}

bool IsVertical( Direction dir )
{
    return dir == Dir_Down || dir == Dir_Up;
}

bool IsHorizontal( Direction dir )
{
    return dir == Dir_Right || dir == Dir_Left;
}

bool IsGrowingDir( Direction dir )
{
    return dir == Dir_Right || dir == Dir_Down;
}

static int sAllDirs[] = 
{
    8,
    9,
    1,
    5,
    4,
    6,
    2,
    10
};

int GetDirection8Ord( Direction dir )
{
    for ( int i = 0; i < _countof( sAllDirs ); i++ )
    {
        if ( dir == sAllDirs[i] )
            return i;
    }
    return 0;
}

Direction GetDirection8( int ord )
{
    return (Direction) sAllDirs[ord];
}

Direction GetOppositeDir8( Direction dir )
{
    uint32_t ord = GetDirection8Ord( dir );
    ord = (ord + 4) % 8;
    return GetDirection8( ord );
}

Direction GetNextDirection8( Direction dir )
{
    uint32_t index = Util::GetDirection8Ord( dir );
    index = (index + 1) % 8;
    return Util::GetDirection8( index );
}

Direction GetPrevDirection8( Direction dir )
{
    uint32_t index = Util::GetDirection8Ord( dir );
    index = (index - 1) % 8;
    return Util::GetDirection8( index );
}


int GetSector16( float x, float y )
{
    uint32_t    sector = 0;

    if ( y < 0 )
    {
        sector += 8;
        y = -y;
        x = -x;
    }

    if ( x < 0 )
    {
        sector += 4;
        float temp = x;
        x = y;
        y = -temp;
    }

    if ( x < y )
    {
        sector += 2;
        float temp = y - x;
        x = x + y;
        y = temp;
        // Because we're only finding out the sector, only the angle matters, not the point along it.
        // So, we can skip multiplying x and y by 1/(2^.5)
    }

    Rotate( NEG_PI_OVER_8, x, y );

    if ( y > 0 )
        sector++;

    sector %= 16;
    return sector;
}

void Rotate( float angle, float& x, float& y )
{
    float sine = sin( angle );
    float cosine = cos( angle );
    float x1 = x;
    float y1 = y;

    x = x1 * cosine - y * sine;
    y = x1 * sine   + y * cosine;
}

void PolarToCart( float angle, float distance, float& x, float& y )
{
    y = sin( angle ) * distance;
    x = cos( angle ) * distance;
}


int GetRandom( int max )
{
    return rand() % max;
}

void MoveSimple( int& x, int& y, Direction dir, int speed )
{
    switch ( dir )
    {
    case Dir_Right: x += speed; break;
    case Dir_Left:  x -= speed; break;
    case Dir_Down:  y += speed; break;
    case Dir_Up:    y -= speed; break;
    }
}

void MoveSimple( uint8_t& x, uint8_t& y, Direction dir, int speed )
{
    switch ( dir )
    {
    case Dir_Right: x += speed; break;
    case Dir_Left:  x -= speed; break;
    case Dir_Down:  y += speed; break;
    case Dir_Up:    y -= speed; break;
    }
}

void MoveSimple8( float& x, float& y, Direction dir, float speed )
{
    switch ( dir & (Dir_Right | Dir_Left) )
    {
    case Dir_Right: x += speed; break;
    case Dir_Left:  x -= speed; break;
    }

    switch ( dir & (Dir_Down | Dir_Up) )
    {
    case Dir_Down:  y += speed; break;
    case Dir_Up:    y -= speed; break;
    }
}

void MoveSimple8( int& x, int& y, Direction dir, int speed )
{
    switch ( dir & (Dir_Right | Dir_Left) )
    {
    case Dir_Right: x += speed; break;
    case Dir_Left:  x -= speed; break;
    }

    switch ( dir & (Dir_Down | Dir_Up) )
    {
    case Dir_Down:  y += speed; break;
    case Dir_Up:    y -= speed; break;
    }
}

void MoveSimple8( uint8_t& x, uint8_t& y, Direction dir, int speed )
{
    switch ( dir & (Dir_Right | Dir_Left) )
    {
    case Dir_Right: x += speed; break;
    case Dir_Left:  x -= speed; break;
    }

    switch ( dir & (Dir_Down | Dir_Up) )
    {
    case Dir_Down:  y += speed; break;
    case Dir_Up:    y -= speed; break;
    }
}

}
