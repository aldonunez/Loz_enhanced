/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum ButtonState
{
    ButtonState_Lifted     = 0,
    ButtonState_Pressing   = 1,
    ButtonState_Releasing  = 2,
    ButtonState_Held       = 3,
};


struct InputButtons
{
    enum Button
    {
        None    = 0,
        A       = 0x80,
        B       = 0x40,
        Select  = 0x20,
        Start   = 0x10,
        Up      = 8,
        Down    = 4,
        Left    = 2,
        Right   = 1
    };

    uint8_t     Buttons;

    InputButtons( uint value );
    bool Has( uint value ) const;
    void Mask( uint value );
    void Clear( uint value );
};


class Input
{
public:
    static InputButtons GetButtons();

    static bool IsKeyDown( int keyCode );
    static bool IsKeyPressing( int keyCode );
    static ButtonState GetKey( int keyCode );

    static bool IsButtonDown( int buttonCode );
    static bool IsButtonPressing( int buttonCode );
    static ButtonState GetButton( int buttonCode );
    static Direction GetInputDirection();

    static void Update();
};
