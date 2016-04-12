/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Graphics.h"
#include "Input.h"
#include "Sound.h"
#include "World.h"
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>


const double FrameTime = 1 / 60.0;


static ALLEGRO_EVENT_QUEUE* eventQ;
static ALLEGRO_DISPLAY* display;
static ALLEGRO_CONFIG* globalConfig;
static uint32_t frameCounter;


void ResizeView( int screenWidth, int screenHeight );

uint32_t GetFrameCounter()
{
    return frameCounter;
}

void Run()
{
    ALLEGRO_EVENT event = { 0 };
    ALLEGRO_EVENT_SOURCE* keyboardSource = al_get_keyboard_event_source();
    ALLEGRO_EVENT_SOURCE* displaySource = al_get_display_event_source( display );
    ALLEGRO_EVENT_SOURCE* joystickSource = al_get_joystick_event_source();

    if ( keyboardSource == nullptr )
        return;
    if ( displaySource == nullptr )
        return;
    if ( joystickSource == nullptr )
        return;

    al_register_event_source( eventQ, keyboardSource );
    al_register_event_source( eventQ, displaySource );
    al_register_event_source( eventQ, joystickSource );

    World world;

    world.Init();

    double startTime = al_get_time();
    double waitSpan = 0;

    while ( true )
    {
        bool updated = false;

        while ( al_wait_for_event_timed( eventQ, &event, waitSpan ) )
        {
            waitSpan = 0;
            if ( event.any.type == ALLEGRO_EVENT_DISPLAY_CLOSE
                || (event.any.type == ALLEGRO_EVENT_KEY_DOWN 
                && event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) )
            {
                goto Done;
            }
            else if ( event.any.type == ALLEGRO_EVENT_DISPLAY_RESIZE )
            {
                al_acknowledge_resize( display );

                ResizeView( event.display.width, event.display.height );

                updated = true;
            }
            else if ( event.any.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION )
            {
                al_reconfigure_joysticks();
            }
        }

        double now = al_get_time();

        while ( (now - startTime) >= FrameTime )
        {
            frameCounter++;

            Input::Update();
            world.Update();
            Sound::Update();

            startTime += FrameTime;
            updated = true;
        }

        if ( updated )
        {
            world.Draw();

            al_flip_display();
        }

        double timeLeft = startTime + FrameTime - al_get_time();
        if ( timeLeft >= .002 )
            waitSpan = timeLeft - .001;
        else
            waitSpan = 0;
    }

Done:
    ;
}

void ResizeView( int screenWidth, int screenHeight )
{
    float viewAspect = StdViewWidth / (float) StdViewHeight;
    float screenAspect = screenWidth / (float) screenHeight;
    // Only allow whole number scaling
    int scale = 1;

    if ( viewAspect > screenAspect )
    {
        scale = screenWidth / StdViewWidth;
    }
    else
    {
        scale = screenHeight / StdViewHeight;
    }

    if ( scale <= 0 )
        scale = 1;

    int viewWidth = StdViewWidth * scale;
    int viewHeight = StdViewHeight * scale;

    // It looks better when the offsets are whole numbers
    int offsetX = (screenWidth - viewWidth) / 2;
    int offsetY = (screenHeight - viewHeight) / 2;

    al_set_clipping_rectangle( offsetX, offsetY, viewWidth, viewHeight );

    ALLEGRO_TRANSFORM t;

    al_identity_transform( &t );
    al_scale_transform( &t, scale, scale );
    al_translate_transform( &t, offsetX, offsetY );
    al_use_transform( &t );

    Graphics::SetViewParams( scale, offsetX, offsetY );
}

void AdjustForDpi( int& width, int& height )
{
#if _WIN32
    HDC hDC = GetDC( NULL );
    if ( hDC != NULL )
    {
        int dpiX = GetDeviceCaps( hDC, LOGPIXELSX );
        int dpiY = GetDeviceCaps( hDC, LOGPIXELSY );
        ReleaseDC( NULL, hDC );
        width = MulDiv( width, dpiX, 96 );
        height = MulDiv( height, dpiY, 96 );
    }
#endif
}

bool MakeDisplay()
{
    int width = StdViewWidth * 2;
    int height = StdViewHeight * 2;
    int newFlags = ALLEGRO_RESIZABLE | ALLEGRO_PROGRAMMABLE_PIPELINE;

    newFlags |= ALLEGRO_DIRECT3D_INTERNAL;

    al_set_new_display_flags( al_get_new_display_flags() | newFlags );

    AdjustForDpi( width, height );
    display = al_create_display( width, height );
    if ( display == nullptr )
        return false;

    ResizeView( width, height );

    return true;
}

ALLEGRO_CONFIG* LoadConfig()
{
    ALLEGRO_CONFIG* config = nullptr;
    ALLEGRO_PATH* configPath = al_get_standard_path( ALLEGRO_USER_SETTINGS_PATH );

    if ( configPath != nullptr )
    {
        al_set_path_filename( configPath, "loz.ini" );

        const char* configPathStr = al_path_cstr( configPath, ALLEGRO_NATIVE_PATH_SEP );
        if ( configPathStr != nullptr )
        {
            config = al_load_config_file( configPathStr );
        }

        al_destroy_path( configPath );
    }

    return config;
}

ALLEGRO_CONFIG* GetConfig()
{
    return globalConfig;
}

bool InitAllegro()
{
    if ( !al_init() )
        return false;

    if ( !al_install_keyboard() )
        return false;

    if ( !al_install_joystick() )
        return false;

    if ( !al_init_image_addon() )
        return false;

    if ( !al_install_audio() )
        return false;

    if ( !al_init_acodec_addon() )
        return false;

    globalConfig = LoadConfig();

    if ( !MakeDisplay() )
        return false;

    eventQ = al_create_event_queue();
    if ( eventQ == nullptr )
        return false;

    if ( !Graphics::Init() )
        return false;

    if ( !Sound::Init() )
        return false;

    Input::Init();

    al_destroy_config( globalConfig );
    globalConfig = nullptr;

    return true;
}

int main( int argc, char* argv[] )
{
    if ( InitAllegro() )
    {
        Run();
    }

    return 0;
}
