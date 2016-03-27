/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Input.h"


struct ButtonMapping
{
    uint8_t SrcCode;
    uint8_t DstCode;
};

static ButtonMapping keyboardMappings[8] = 
{
    { ALLEGRO_KEY_RIGHT,    InputButtons::Right },
    { ALLEGRO_KEY_LEFT,     InputButtons::Left },
    { ALLEGRO_KEY_DOWN,     InputButtons::Down },
    { ALLEGRO_KEY_UP,       InputButtons::Up },
    { ALLEGRO_KEY_ENTER,    InputButtons::Start },
    { ALLEGRO_KEY_S,        InputButtons::Select },
    { ALLEGRO_KEY_D,        InputButtons::B },
    { ALLEGRO_KEY_F,        InputButtons::A },
};

static InputButtons oldInputState( 0 );
static InputButtons inputState( 0 );

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

InputButtons Input::GetButtons()
{
    uint buttons = 0;

    buttons = (oldInputState.Buttons ^ inputState.Buttons) 
        & inputState.Buttons 
        | (inputState.Buttons & 0xF);

    return (InputButtons) buttons;
}

bool Input::IsKeyDown( int keyCode )
{
    return al_key_down( &keyboardState, keyCode );
}

bool Input::IsKeyPressing( int keyCode )
{
    return GetKey( keyCode ) == ButtonState_Pressing;
}

ButtonState Input::GetKey( int keyCode )
{
    int isDown = al_key_down( &keyboardState, keyCode ) ? 1 : 0;
    int wasDown = al_key_down( &oldKeyboardState, keyCode ) ? 1 : 0;

    return (ButtonState) ((wasDown << 1) | isDown);
}

bool Input::IsButtonDown( int buttonCode )
{
    return inputState.Buttons & buttonCode;
}

bool Input::IsButtonPressing( int buttonCode )
{
    return GetButton( buttonCode ) == ButtonState_Pressing;
}

ButtonState Input::GetButton( int buttonCode )
{
    int isDown = (inputState.Buttons & buttonCode) ? 1 : 0;
    int wasDown = (oldInputState.Buttons & buttonCode) ? 1 : 0;

    return (ButtonState) ((wasDown << 1) | isDown);
}

Direction Input::GetInputDirection()
{
    if ( IsButtonDown( InputButtons::Left ) )
    {
        return Dir_Left;
    }
    else if ( IsButtonDown( InputButtons::Right ) )
    {
        return Dir_Right;
    }
    else if ( IsButtonDown( InputButtons::Up ) )
    {
        return Dir_Up;
    }
    else if ( IsButtonDown( InputButtons::Down ) )
    {
        return Dir_Down;
    }

    return Dir_None;
}

static void PollKeyboard()
{
    memcpy( &oldKeyboardState, &keyboardState, sizeof keyboardState );
    al_get_keyboard_state( &keyboardState );

    for ( int i = 0; i < _countof( keyboardMappings ); i++ )
    {
        ButtonMapping& mapping = keyboardMappings[i];
        if ( mapping.DstCode == InputButtons::None )
            continue;

        if ( al_key_down( &keyboardState, mapping.SrcCode ) )
            inputState.Buttons |= mapping.DstCode;
    }
}

static void Poll()
{
    oldInputState = inputState;
    inputState = 0;

    PollKeyboard();
}

void Input::Update()
{
    Poll();
}
