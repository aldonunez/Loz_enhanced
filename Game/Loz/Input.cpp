/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Input.h"


static ALLEGRO_KEYBOARD_STATE oldKeyboardState;
static ALLEGRO_KEYBOARD_STATE keyboardState;


InputButtons::InputButtons( uint value )
    :   Buttons( value )
{
}

bool InputButtons::Has( uint value ) const
{
    return (Buttons & value) != 0;
}

void InputButtons::Mask( uint value )
{
    Buttons &= value;
}

void InputButtons::Clear( uint value )
{
    Buttons = Buttons ^ value;
}

// TODO: calculate this once a frame

InputButtons Input::GetButtons()
{
    uint buttons = 0;

    if ( IsKeyPressing( WeaponKey ) )
        buttons |= InputButtons::A;

    if ( IsKeyPressing( ItemKey ) )
        buttons |= InputButtons::B;

    if ( IsKeyPressing( SelectKey ) )
        buttons |= InputButtons::Select;

    if ( IsKeyPressing( MenuKey ) )
        buttons |= InputButtons::Start;

    if ( IsKeyDown( ALLEGRO_KEY_UP ) )
        buttons |= InputButtons::Up;

    if ( IsKeyDown( ALLEGRO_KEY_DOWN ) )
        buttons |= InputButtons::Down;

    if ( IsKeyDown( ALLEGRO_KEY_LEFT ) )
        buttons |= InputButtons::Left;

    if ( IsKeyDown( ALLEGRO_KEY_RIGHT ) )
        buttons |= InputButtons::Right;

    return (InputButtons) buttons;
}

bool Input::IsKeyDown( int keyCode )
{
    return al_key_down( &keyboardState, keyCode );
}

bool Input::IsKeyPressing( int keyCode )
{
    return GetKey( keyCode ) == KeyState_Pressing;
}

KeyState Input::GetKey( int keyCode )
{
    int isDown = al_key_down( &keyboardState, keyCode ) ? 1 : 0;
    int wasDown = al_key_down( &oldKeyboardState, keyCode ) ? 1 : 0;

    return (KeyState) ((wasDown << 1) | isDown);
}

Direction Input::GetInputDirection()
{
    if ( IsKeyDown( ALLEGRO_KEY_LEFT ) )
    {
        return Dir_Left;
    }
    else if ( IsKeyDown( ALLEGRO_KEY_RIGHT ) )
    {
        return Dir_Right;
    }
    else if ( IsKeyDown( ALLEGRO_KEY_UP ) )
    {
        return Dir_Up;
    }
    else if ( IsKeyDown( ALLEGRO_KEY_DOWN ) )
    {
        return Dir_Down;
    }

    return Dir_None;
}

static void Poll()
{
    memcpy( &oldKeyboardState, &keyboardState, sizeof keyboardState );
    al_get_keyboard_state( &keyboardState );
}

void Input::Update()
{
    Poll();
}
