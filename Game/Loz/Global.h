/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


typedef unsigned int uint;

const int StdViewWidth = 256;
const int StdViewHeight = 240;

const int PaletteCount = 8;
const int PaletteLength = 4;
const int ForegroundPalCount = 4;
const int BackgroundPalCount = 4;

enum
{
    SysPaletteLength    = 64,
};

enum
{
    WhiteBgPalette  = 0,
    RedBgPalette    = 1,
    PlayerPalette   = 4,
    BlueFgPalette   = 5,
    RedFgPalette    = 6,
    LevelFgPalette  = 7,
};

enum
{
    LevelBlockWidth     = 16,
    LevelBlockHeight    = 8,
    LevelBlockRooms     = 128,
};

enum Direction
{
    Dir_None,
    Dir_Right   = 1,
    Dir_Left    = 2,
    Dir_Down    = 4,
    Dir_Up      = 8
};

enum
{
    VerticalMask        = Dir_Down | Dir_Up,
    HorizontalMask      = Dir_Right | Dir_Left,

    OppositeVerticals   = VerticalMask,
    OppositeHorizontals = HorizontalMask,
};

struct Bounds
{
    uint16_t X;
    uint16_t Y;
    uint16_t Width;
    uint16_t Height;
};

struct Point
{
    short   X;
    short   Y;
};

typedef uint8_t RSpot;

enum ItemId
{
    Item_Bomb,
    Item_WoodSword,
    Item_WhiteSword,
    Item_MagicSword,
    Item_Food,
    Item_Recorder,
    Item_BlueCandle,
    Item_RedCandle,
    Item_WoodArrow,
    Item_SilverArrow,
    Item_Bow,
    Item_MagicKey,
    Item_Raft,
    Item_Ladder,
    Item_PowerTriforce,
    Item_5Rupees,
    Item_Rod,
    Item_Book,
    Item_BlueRing,
    Item_RedRing,
    Item_Bracelet,
    Item_Letter,
    Item_Compass,
    Item_Map,
    Item_Rupee,
    Item_Key,
    Item_HeartContainer,
    Item_TriforcePiece,
    Item_MagicShield,
    Item_WoodBoomerang,
    Item_MagicBoomerang,
    Item_BluePotion,
    Item_RedPotion,
    Item_Clock,
    Item_Heart,
    Item_Fairy,

    Item_Max    = 0x3F,
    Item_None   = Item_Max
};

enum
{
    String_DoorRepair       = 5,
    String_AintEnough       = 10,
    String_LostHillsHint    = 11,
    String_LostWoodsHint    = 12,
    String_Grumble          = 18,
    String_MoreBombs        = 25,
    String_MoneyOrLife      = 27,
    String_EnterLevel9      = 34,
};


uint32_t GetFrameCounter();
ALLEGRO_CONFIG* GetConfig();
