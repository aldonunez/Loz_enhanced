/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class TextBox
{
public:
    enum
    {
        StartX      = 0x20,
        StartY      = 0x68,
        CharDelay   = 6,
    };

private:
    int             left;
    int             top;
    int             height;
    int             charDelay;
    int             charTimer;
    bool            drawingDialog;
    const uint8_t*  startCharPtr;
    const uint8_t*  curCharPtr;

public:
    TextBox( const uint8_t* str, int delay = CharDelay );

    void Reset( const uint8_t* str );
    bool IsDone();
    int  GetHeight();
    int  GetX();
    int  GetY();
    void SetX( int x );
    void SetY( int y );

    void Update();
    void Draw();
};
