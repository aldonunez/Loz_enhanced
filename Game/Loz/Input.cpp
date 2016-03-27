/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Input.h"


enum InputAxis
{
    InputAxis_None,
    InputAxis_Horizontal,
    InputAxis_Vertical
};

struct ButtonMapping
{
    uint8_t SrcCode;
    uint8_t DstCode;
};

struct AxisMapping
{
    uint8_t Stick;
    uint8_t SrcAxis;
    uint8_t DstAxis;
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

static ButtonMapping joystickButtonMappings[4] = 
{
    { 9,        InputButtons::Start },
    { 8,        InputButtons::Select },
    { 1,        InputButtons::B },
    { 2,        InputButtons::A },
};

static AxisMapping joystickAxisMappings[2] = 
{
    { 2, 0,     InputAxis_Horizontal },
    { 2, 1,     InputAxis_Vertical },
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

static void PollJoystick()
{
    ALLEGRO_JOYSTICK* joystick = al_get_joystick( 0 );
    if ( joystick == nullptr )
        return;

    int numButtons = al_get_joystick_num_buttons( joystick );
    int numSticks = al_get_joystick_num_sticks( joystick );

    ALLEGRO_JOYSTICK_STATE joystickState;
    al_get_joystick_state( joystick, &joystickState );

    for ( int i = 0; i < _countof( joystickButtonMappings ); i++ )
    {
        ButtonMapping& mapping = joystickButtonMappings[i];
        if ( mapping.DstCode == InputButtons::None )
            continue;

        if ( mapping.SrcCode < numButtons && joystickState.button[mapping.SrcCode] > 0 )
            inputState.Buttons |= mapping.DstCode;
    }

    for ( int i = 0; i < _countof( joystickAxisMappings ); i++ )
    {
        AxisMapping& mapping = joystickAxisMappings[i];
        if ( mapping.DstAxis == InputAxis_None )
            continue;

        if ( mapping.Stick < numSticks )
        {
            int numAxes = al_get_joystick_num_axes( joystick, mapping.Stick );
            if ( mapping.SrcAxis < numAxes )
            {
                float axisValue = joystickState.stick[mapping.Stick].axis[mapping.SrcAxis];
                if ( axisValue > 0 )
                {
                    if ( mapping.DstAxis == InputAxis_Horizontal )
                        inputState.Buttons |= InputButtons::Right;
                    else
                        inputState.Buttons |= InputButtons::Down;
                }
                else if ( axisValue < 0 )
                {
                    if ( mapping.DstAxis == InputAxis_Horizontal )
                        inputState.Buttons |= InputButtons::Left;
                    else
                        inputState.Buttons |= InputButtons::Up;
                }
            }
        }
    }
}

static void Poll()
{
    oldInputState = inputState;
    inputState = 0;

    PollKeyboard();
    PollJoystick();
}

void Input::Update()
{
    Poll();
}
