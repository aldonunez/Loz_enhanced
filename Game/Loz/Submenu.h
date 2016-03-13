/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "SpriteAnimator.h"


class Submenu
{
public:
    enum
    {
        Width           = StdViewWidth,
        Height          = 0xAE,

        ActiveItems     = 8,
        PassiveItems    = 6,
    };

private:
    bool        enabled;
    bool        activated;
    int         activeUISlot;
    int         activeSlots[ActiveItems];
    int         activeItems[ActiveItems];
    SpriteImage cursor;

public:
    Submenu();

    void Enable();
    void Disable();
    void Activate();
    void Deactivate();

    void Update();
    void Draw( int bottom );

private:
    void DrawBackground( int top );
    void DrawActiveInventory( int top );
    void DrawPassiveInventory( int top );
    void DrawCurrentSelection( int top );
    void DrawTriforce( int top );
    void DrawMap( int top );
};
