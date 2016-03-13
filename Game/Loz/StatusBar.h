/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class StatusBar
{
public:
    enum
    {
        StatusBarHeight = 0x40,
    };

    enum Features
    {
        Feature_None            = 0,
        Feature_Counters        = 1,
        Feature_Equipment       = 2,
        Feature_MapCursors      = 4,

        Feature_All             = Feature_Counters
                                | Feature_Equipment
                                | Feature_MapCursors,

        Feature_EquipmentAndMap = Feature_Equipment | Feature_MapCursors,
    };

private:
    Features features;

public:
    StatusBar();

    void EnableFeatures( Features features, bool enable );
    void Draw( int baseY );
    void Draw( int baseY, ALLEGRO_COLOR backColor );

private:
    void DrawTile( int tile, int x, int y, int palette );

    void DrawMiniMap( int baseY );
    void DrawOWMiniMap( int baseY );
    void DrawUWMiniMap( int baseY );

    void DrawItems( int baseY );
    void DrawSword( int baseY );
    void DrawItemB( int baseY );
    void DrawHearts( int baseY );
};
