/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


const int ConfirmKey = ALLEGRO_KEY_F;
const int SelectKey = ALLEGRO_KEY_S;
const int MenuKey = ALLEGRO_KEY_ENTER;
const int ItemKey = ALLEGRO_KEY_D;
const int WeaponKey = ALLEGRO_KEY_F;


enum KeyState
{
    KeyState_Lifted     = 0,
    KeyState_Pressing   = 1,
    KeyState_Releasing  = 2,
    KeyState_Held       = 3,
};


struct InputButtons
{
    enum Value
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

    static KeyState GetKey( int keyCode );
    static Direction GetInputDirection();

    static void Update();
};
