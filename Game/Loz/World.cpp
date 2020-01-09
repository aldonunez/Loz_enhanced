/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "World.h"
#include "WorldImpl.h"
#include "Graphics.h"
#include "Input.h"
#include "TileAttr.h"
#include "Player.h"
#include "ItemObj.h"
#include "PlayerItemsAnim.h"
#include "SpriteAnimator.h"
#include "ObjType.h"
#include "Monsters.h"
#include "Profile.h"
#include "Credits.h"
#include "TextBox.h"
#include "UWBossAnim.h"
#include "GameMenu.h"
#include "RegisterMenu.h"
#include "EliminateMenu.h"
#include "SaveFolder.h"
#include "Sound.h"
#include "SoundId.h"


enum
{
    LevelGroups     = 3,
};

enum
{
    Cave_Items      = 0x79,
    Cave_Shortcut   = 0x7A,
};

enum Secret
{
    Secret_None,
    Secret_FoesDoor,
    Secret_Ringleader,
    Secret_LastBoss,
    Secret_BlockDoor,
    Secret_BlockStairs,
    Secret_MoneyOrLife,
    Secret_FoesItem
};

struct SparseRoomAttr
{
    uint8_t     roomId;
    OWRoomAttrs attrs;
};

struct SparseMaze
{
    uint8_t     roomId;
    uint8_t     exitDir;
    uint8_t     path[4];
};

struct SparseRoomItem
{
    uint8_t     roomId;
    uint8_t     x;
    uint8_t     y;
    uint8_t     itemId;
};


enum
{
    Sparse_ArmosStairs,
    Sparse_ArmosItem,
    Sparse_Dock,
    Sparse_Item,
    Sparse_Shortcut,
    Sparse_Maze,
    Sparse_SecretScroll,
    Sparse_Ladder,
    Sparse_Recorder,
    Sparse_Fairy,
    Sparse_RoomReplacement,
};

enum
{
    Extra_PondColors,
    Extra_SpawnSpots,
    Extra_ObjAttrs,
    Extra_CavePalettes,
    Extra_Caves,
    Extra_LevelPersonStringIds,
    Extra_HitPoints,
    Extra_PlayerDamage,
};

struct ColorSeq
{
    int     Length;
    uint8_t Colors[1];
};

struct SpotSeq
{
    int     Length;
    RSpot   Spots[1];
};

struct PaletteSet
{
    int     Length;
    int     Start;
    uint8_t Palettes[1][PaletteLength];
};

struct CaveSpecList
{
    int         Count;
    CaveSpec    Specs[1];
};

struct LevelPersonStrings
{
    uint8_t StringIds[LevelGroups][PersonTypes];
};

const int OWMarginRight  = 0xE0;
const int OWMarginLeft   = 0x10;
const int OWMarginTop    = 0x4D;
const int OWMarginBottom = 0xCD;

const int UWMarginRight  = 0xD0;
const int UWMarginLeft   = 0x20;
const int UWMarginTop    = 0x5D;
const int UWMarginBottom = 0xBD;

const int UWBorderRight  = 0xE0;
const int UWBorderLeft   = 0x20;
const int UWBorderTop    = 0x60;
const int UWBorderBottom = 0xD0;

const int UWBlockRow        = 10;

const int DoorWidth         = 32;
const int DoorHeight        = 32;
const int DoorOverlayBaseY  = 128;
const int DoorUnderlayBaseY = 0;

const int DoorHoleCoordH = 0x90;
const int DoorHoleCoordV = 0x78;

const int UWBombRadius = 32;

static const uint8_t levelGroups[] = 
{
    0, 0, 1, 1, 0, 1, 0, 1, 2
};

const Point doorMiddles[] = 
{
    { 0xE0, 0x98 },
    { 0x20, 0x98 },
    { 0x80, 0xD0 },
    { 0x80, 0x60 }
};

const int doorSrcYs[] = 
{
    64,
    96,
    0,
    32
};

const Point doorPos[] = 
{
    { 224,  136 },
    { 0,    136 },
    { 112,  208 },
    { 112,  64 }
};

struct DoorStateFaces
{
    uint8_t Closed;
    uint8_t Open;
};

const DoorStateFaces doorFaces[] = 
{
    { 0, 0 },
    { 3, 3 },
    { 3, 3 },
    { 3, 3 },
    { 3, 4 },
    { 1, 0 },
    { 1, 0 },
    { 2, 0 },
};

static const Cell doorCorners[] =
{
    { 0x0A, 0x1C },
    { 0x0A, 0x02 },
    { 0x12, 0x0F },
    { 0x02, 0x0F },
};

static const Cell behindDoorCorners[] =
{
    { 0x0A, 0x1E },
    { 0x0A, 0x00 },
    { 0x14, 0x0F },
    { 0x00, 0x0F },
};

struct DoorStateBehaviors
{
    TileBehavior Closed;
    TileBehavior Open;
};

static const DoorStateBehaviors doorBehaviors[] =
{
    { TileBehavior_Doorway, TileBehavior_Doorway },     // Open
    { TileBehavior_Wall, TileBehavior_Wall },           // Wall (None)
    { TileBehavior_Door, TileBehavior_Door },           // False Wall
    { TileBehavior_Door, TileBehavior_Door },           // False Wall 2
    { TileBehavior_Door, TileBehavior_Door },           // Bombable
    { TileBehavior_Door, TileBehavior_Doorway },        // Key
    { TileBehavior_Door, TileBehavior_Doorway },        // Key 2
    { TileBehavior_Door, TileBehavior_Doorway },        // Shutter
};

struct EquipValue
{
    uint8_t Value;
    uint8_t Slot;
};

// The item ID to item slot map is at $6B14, and copied to RAM at $72A4.
// The item ID to item value map is at $6B38, and copied to RAM at $72C8.
// They're combined here.
static const EquipValue sItemToEquipment[] = 
{
    { 4, ItemSlot_Bombs },
    { 1, ItemSlot_Sword },
    { 2, ItemSlot_Sword },
    { 3, ItemSlot_Sword },
    { 1, ItemSlot_Food },
    { 1, ItemSlot_Recorder },
    { 1, ItemSlot_Candle },
    { 2, ItemSlot_Candle },
    { 1, ItemSlot_Arrow },
    { 2, ItemSlot_Arrow },
    { 1, ItemSlot_Bow },
    { 1, ItemSlot_MagicKey },
    { 1, ItemSlot_Raft },
    { 1, ItemSlot_Ladder },
    { 1, ItemSlot_PowerTriforce },
    { 5, ItemSlot_RupeesToAdd },
    { 1, ItemSlot_Rod },
    { 1, ItemSlot_Book },
    { 1, ItemSlot_Ring },
    { 2, ItemSlot_Ring },
    { 1, ItemSlot_Bracelet },
    { 1, ItemSlot_Letter },
    { 1, ItemSlot_Compass9 },
    { 1, ItemSlot_Map9 },
    { 1, ItemSlot_RupeesToAdd },
    { 1, ItemSlot_Keys },
    { 1, ItemSlot_HeartContainers },
    { 1, ItemSlot_TriforcePieces },
    { 1, ItemSlot_MagicShield },
    { 1, ItemSlot_Boomerang },
    { 2, ItemSlot_Boomerang },
    { 1, ItemSlot_Potion },
    { 2, ItemSlot_Potion },
    { 1, ItemSlot_Clock },
    { 1, ItemSlot_Bombs },
    { 3, ItemSlot_Bombs },
};

static const uint8_t teleportYs[] = { 0x8D, 0xAD, 0x8D, 0x8D, 0xAD, 0x8D, 0xAD, 0x5D };
static const uint8_t teleportRoomIds[] = { 0x36, 0x3B, 0x73, 0x44, 0x0A, 0x21, 0x41, 0x6C };

const int StartX = 0x78;
const int FirstCaveIndex = 0x10;
const int TriforcePieceX = 0x78;


WorldImpl::TileActionFunc WorldImpl::sActionFuncs[] = 
{
    &WorldImpl::NoneTileAction,
    &WorldImpl::PushTileAction,
    &WorldImpl::BombTileAction,
    &WorldImpl::BurnTileAction,
    &WorldImpl::HeadstoneTileAction,
    &WorldImpl::LadderTileAction,
    &WorldImpl::RaftTileAction,
    &WorldImpl::CaveTileAction,
    &WorldImpl::StairsTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::BlockTileAction,
};

WorldImpl::TileBehaviorFunc WorldImpl::sBehaviorFuncs[] = 
{
    &WorldImpl::NoneTileAction,
    &WorldImpl::NoneTileAction,
    &WorldImpl::NoneTileAction,
    &WorldImpl::StairsTileAction,
    &WorldImpl::NoneTileAction,

    &WorldImpl::NoneTileAction,
    &WorldImpl::NoneTileAction,
    &WorldImpl::CaveTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::GhostTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::ArmosTileAction,
    &WorldImpl::DoorTileAction,
    &WorldImpl::NoneTileAction,
};

WorldImpl::UpdateFunc WorldImpl::sModeFuncs[Modes] = 
{
    nullptr,
    &WorldImpl::UpdateGameMenu,
    &WorldImpl::UpdateLoadLevel,
    &WorldImpl::UpdateUnfurl,
    &WorldImpl::UpdateEnter,
    &WorldImpl::UpdatePlay,
    &WorldImpl::UpdateLeave,
    &WorldImpl::UpdateScroll,
    &WorldImpl::UpdateContinueQuestion,
    &WorldImpl::UpdatePlay,
    &WorldImpl::UpdateLeaveCellar,
    &WorldImpl::UpdatePlay,
    nullptr,
    nullptr,
    &WorldImpl::UpdateRegisterMenu,
    &WorldImpl::UpdateEliminateMenu,
    &WorldImpl::UpdateStairsState,
    &WorldImpl::UpdateDie,
    &WorldImpl::UpdateEndLevel,
    &WorldImpl::UpdateWinGame,
    &WorldImpl::UpdatePlayCellar,
    &WorldImpl::UpdatePlayCave,
};

WorldImpl::DrawFunc WorldImpl::sDrawFuncs[Modes] = 
{
    nullptr,
    &WorldImpl::DrawGameMenu,
    &WorldImpl::DrawLoadLevel,
    &WorldImpl::DrawUnfurl,
    &WorldImpl::DrawEnter,
    &WorldImpl::DrawPlay,
    &WorldImpl::DrawLeave,
    &WorldImpl::DrawScroll,
    &WorldImpl::DrawContinueQuestion,
    &WorldImpl::DrawPlay,
    &WorldImpl::DrawLeaveCellar,
    &WorldImpl::DrawPlay,
    nullptr,
    nullptr,
    &WorldImpl::DrawGameMenu,
    &WorldImpl::DrawGameMenu,
    &WorldImpl::DrawStairsState,
    &WorldImpl::DrawDie,
    &WorldImpl::DrawEndLevel,
    &WorldImpl::DrawWinGame,
    &WorldImpl::DrawPlayCellar,
    &WorldImpl::DrawPlayCave,
};

WorldImpl::UpdateFunc WorldImpl::sPlayCellarFuncs[PlayCellarState::MaxSubstate] =
{
    &WorldImpl::UpdatePlayCellar_Start,
    &WorldImpl::UpdatePlayCellar_FadeOut,
    &WorldImpl::UpdatePlayCellar_LoadRoom,
    &WorldImpl::UpdatePlayCellar_FadeIn,
    &WorldImpl::UpdatePlayCellar_Walk,
};

WorldImpl::UpdateFunc WorldImpl::sPlayCaveFuncs[PlayCaveState::MaxSubstate] =
{
    &WorldImpl::UpdatePlayCave_Start,
    &WorldImpl::UpdatePlayCave_Wait,
    &WorldImpl::UpdatePlayCave_LoadRoom,
    &WorldImpl::UpdatePlayCave_Walk,
};

WorldImpl::UpdateFunc WorldImpl::sEndLevelFuncs[EndLevelState::MaxSubstate] = 
{
    &WorldImpl::UpdateEndLevel_Start,
    &WorldImpl::UpdateEndLevel_Wait,
    &WorldImpl::UpdateEndLevel_Flash,
    &WorldImpl::UpdateEndLevel_FillHearts,
    &WorldImpl::UpdateEndLevel_Wait,
    &WorldImpl::UpdateEndLevel_Furl,
    &WorldImpl::UpdateEndLevel_Wait,
};

WorldImpl::UpdateFunc WorldImpl::sWinGameFuncs[WinGameState::MaxSubstate] = 
{
    &WorldImpl::UpdateWinGame_Start,
    &WorldImpl::UpdateWinGame_Text1,
    &WorldImpl::UpdateWinGame_Stand,
    &WorldImpl::UpdateWinGame_Hold1,
    &WorldImpl::UpdateWinGame_Colors,
    &WorldImpl::UpdateWinGame_Hold2,
    &WorldImpl::UpdateWinGame_Text2,
    &WorldImpl::UpdateWinGame_Hold3,
    &WorldImpl::UpdateWinGame_NoObjects,
    &WorldImpl::UpdateWinGame_Credits,
};

WorldImpl::UpdateFunc WorldImpl::sScrollFuncs[ScrollState::MaxSubstate] = 
{
    &WorldImpl::UpdateScroll_Start,
    &WorldImpl::UpdateScroll_AnimatingColors,
    &WorldImpl::UpdateScroll_FadeOut,
    &WorldImpl::UpdateScroll_LoadRoom,
    &WorldImpl::UpdateScroll_Scroll,
};

WorldImpl::UpdateFunc WorldImpl::sDeathFuncs[DeathState::MaxSubstate] = 
{
    &WorldImpl::UpdateDie_Start,
    &WorldImpl::UpdateDie_Flash,
    &WorldImpl::UpdateDie_Wait1,
    &WorldImpl::UpdateDie_Turn,
    &WorldImpl::UpdateDie_Fade,
    &WorldImpl::UpdateDie_GrayLink,
    &WorldImpl::UpdateDie_Spark,
    &WorldImpl::UpdateDie_Wait2,
    &WorldImpl::UpdateDie_GameOver,
};

WorldImpl::UpdateFunc WorldImpl::sLeaveCellarFuncs[LeaveCellarState::MaxSubstate] = 
{
    &WorldImpl::UpdateLeaveCellar_Start,
    &WorldImpl::UpdateLeaveCellar_FadeOut,
    &WorldImpl::UpdateLeaveCellar_LoadRoom,
    &WorldImpl::UpdateLeaveCellar_FadeIn,
    &WorldImpl::UpdateLeaveCellar_Walk,
    &WorldImpl::UpdateLeaveCellar_Wait,
    &WorldImpl::UpdateLeaveCellar_LoadOverworldRoom,
};

WorldImpl::UpdateFunc WorldImpl::sEnterFuncs[EnterState::MaxSubstate] =
{
    &WorldImpl::UpdateEnter_Start,
    &WorldImpl::UpdateEnter_Wait,
    &WorldImpl::UpdateEnter_FadeIn,
    &WorldImpl::UpdateEnter_Walk,
    &WorldImpl::UpdateEnter_WalkCave,
};

static WorldImpl* sWorld;


void GetWorldCoord( int roomId, int& row, int& col )
{
    row = (roomId & 0xF0) >> 4;
    col = roomId & 0x0F;
}

int MakeRoomId( int row, int col )
{
    return (row << 4) | col;
}

int GetNextRoomId( int curRoomId, Direction dir )
{
    int row;
    int col;

    GetWorldCoord( curRoomId, row, col );

    switch ( dir )
    {
    case Dir_Left:
        if ( col == 0 )
            return curRoomId;
        col--;
        break;

    case Dir_Right:
        if ( col == World::WorldWidth - 1 )
            return curRoomId;
        col++;
        break;

    case Dir_Up:
        if ( row == 0 )
            return curRoomId;
        row--;
        break;

    case Dir_Down:
        if ( row == World::WorldHeight - 1 )
            return curRoomId;
        row++;
        break;
    }

    int nextRoomId = MakeRoomId( row, col );
    return nextRoomId;
}

void GetRoomCoord( int position, int& row, int& col )
{
    row = position & 0x0F;
    col = (position & 0xF0) >> 4;
    row -= 4;
}

void GetRSpotCoord( uint32_t position, int& x, int& y )
{
    x = (position & 0x0F) << 4;
    y = (position & 0xF0) | 0xD;
}

Point GetRoomItemPosition( uint8_t position )
{
    Point point;
    point.X = position & 0xF0;
    point.Y = (uint8_t) (position << 4);
    return point;
}

int GetDoorStateFace( int type, bool state )
{
    if ( state )
        return doorFaces[type].Open;
    return doorFaces[type].Closed;
}

void ClearScreen()
{
    al_clear_to_color( al_map_rgb( 0, 0, 0 ) );
}

void ClearScreen( int sysColor )
{
    ALLEGRO_COLOR color = Graphics::GetSystemColor( sysColor );
    al_clear_to_color( color );
}

void WorldImpl::ClearDeadObjectQueue()
{
    for ( int i = 0; i < objectsToDeleteCount; i++ )
    {
        delete objectsToDelete[i];
        objectsToDelete[i] = nullptr;
    }

    objectsToDeleteCount = 0;
}

void WorldImpl::SetOnlyObject( int slot, Object* obj )
{
    assert( slot >= 0 && slot < MaxObjects );
    if ( objects[slot] != nullptr )
    {
        if ( objectsToDeleteCount == MaxObjects )
            ClearDeadObjectQueue();
        objectsToDelete[objectsToDeleteCount] = objects[slot];
        objectsToDeleteCount++;
    }
    objects[slot] = obj;
}

Ladder* WorldImpl::GetLadderObj()
{
    return (Ladder*) objects[LadderSlot];
}

void WorldImpl::SetLadderObj( Ladder* ladder )
{
    SetOnlyObject( LadderSlot, ladder );
}

void WorldImpl::SetBlockObj( Object* block )
{
    SetOnlyObject( BlockSlot, block );
}

void WorldImpl::DeleteObjects()
{
    for ( int i = 0; i < MaxObjects; i++ )
    {
        if ( objects[i] != nullptr )
        {
            delete objects[i];
            objects[i] = nullptr;
        }
    }

    ClearDeadObjectQueue();
}

void WorldImpl::CleanUpRoomItems()
{
    DeleteObjects();
    World::SetItem( ItemSlot_Clock, 0 );
}

void WorldImpl::DeleteDeadObjects()
{
    for ( int i = 0; i < MaxObjects; i++ )
    {
        Object* obj = objects[i];
        if ( obj != nullptr && obj->IsDeleted() )
        {
            delete obj;
            objects[i] = nullptr;
        }
    }

    ClearDeadObjectQueue();
}

void WorldImpl::InitObjectTimers()
{
    for ( int i = 0; i < MaxObjects; i++ )
    {
        objectTimers[i] = 0;
    }
}

void WorldImpl::DecrementObjectTimers()
{
    for ( int i = 0; i < MaxObjects; i++ )
    {
        if ( objectTimers[i] != 0 )
            objectTimers[i]--;

        if ( objects[i] != nullptr )
            objects[i]->DecrementObjectTimer();
    }

    // ORIGINAL: Here the player isn't part of the array, but in the original it's the first element.
    player->DecrementObjectTimer();
}

void WorldImpl::InitStunTimers()
{
    longTimer = 0;
    for ( int i = 0; i < MaxObjects; i++ )
    {
        stunTimers[i] = 0;
    }
}

void WorldImpl::DecrementStunTimers()
{
    if ( longTimer > 0 )
    {
        longTimer--;
        return;
    }

    longTimer = 9;

    for ( int i = 0; i < MaxObjects; i++ )
    {
        if ( stunTimers[i] != 0 )
            stunTimers[i]--;

        if ( objects[i] != nullptr )
            objects[i]->DecrementStunTimer();
    }

    // ORIGINAL: Here the player isn't part of the array, but in the original it's the first element.
    player->DecrementStunTimer();
}

void WorldImpl::InitPlaceholderTypes()
{
    memset( placeholderTypes, 0, sizeof placeholderTypes );
}

int WorldImpl::FindEmptyMonsterSlot()
{
    for ( int i = LastMonsterSlot; i >= 0; i-- )
    {
        if ( objects[i] == nullptr )
            return i;
    }
    return -1;
}

void WorldImpl::ClearRoomItemData()
{
    recorderUsed = 0;
    candleUsed = false;
    summonedWhirlwind = false;
    shuttersPassedDirs = Dir_None;
    brightenRoom = false;
    activeShots = 0;
}

void WorldImpl::SetPlayerColor()
{
    static const uint8_t sysColors[] = 
    {
        0x29,
        0x32,
        0x16
    };

    int value = profile.Items[ItemSlot_Ring];

    Graphics::SetColorIndexed( PlayerPalette, 1, sysColors[value] );
}

static void SetFlashPalette()
{
    static const uint8_t palette[] = 
    {
        0x0F,
        0x30,
        0x30,
        0x30
    };

    for ( int i = 2; i < BackgroundPalCount; i++ )
    {
        Graphics::SetPaletteIndexed( i, palette );
    }

    Graphics::UpdatePalettes();
}

static void SetLevelPalettes( const uint8_t palettes[2][PaletteLength] )
{
    for ( int i = 0; i < 2; i++ )
    {
        Graphics::SetPaletteIndexed( 2 + i, palettes[i] );
    }

    Graphics::UpdatePalettes();
}

static void SetLevelPalette()
{
    const World::LevelInfoBlock* infoBlock = World::GetLevelInfo();

    for ( int i = 2; i < BackgroundPalCount; i++ )
    {
        Graphics::SetPaletteIndexed( i, infoBlock->Palettes[i] );
    }

    Graphics::UpdatePalettes();
}

static void SetLevelFgPalette()
{
    const World::LevelInfoBlock* infoBlock = World::GetLevelInfo();
    Graphics::SetPaletteIndexed( LevelFgPalette, infoBlock->Palettes[LevelFgPalette] );
}


WorldImpl::WorldImpl()
    :   curRoomId( 0 ),
        curTileMapIndex( 0 ),
        loadMobFunc( nullptr ),
        lastMode( Mode_Demo ),
        curMode( Mode_Play ),
        curColorSeqNum( 0 ),
        darkRoomFadeStep( 0 ),
        curMazeStep( 0 ),
        spotIndex( 0 ),
        tempShutterRoomId( 0 ),
        tempShutterDoorDir( 0 ),
        tempShuttersRoomId( 0 ),
        tempShutters( false ),
        wallsBmp( nullptr ),
        doorsBmp( nullptr ),
        edgeX( 0 ),
        edgeY( 0x40 ),
        worldKillCycle( 0 ),
        worldKillCount( 0 ),
        roomKillCount( 0 ),
        roomAllDead( false ),
        madeRoomItem( false ),
        enablePersonFireballs( false ),
        nextRoomHistorySlot( 0 ),
        helpDropCounter( 0 ),
        helpDropValue( 0 ),
        whirlwindTeleporting( 0 ),
        teleportingRoomIndex( 0 ),
        swordBlocked( false ),
        pause( 0 ),
        submenu( 0 ),
        submenuOffsetY( 0 ),
        statusBarVisible( false ),
        textBox1( nullptr ),
        textBox2( nullptr ),
        credits( nullptr ),
        gameMenu( nullptr ),
        nextGameMenu( nullptr ),

        player(),
        giveFakePlayerPos(),
        playerPosTimer(),
        fakePlayerPos(),

        objects(),
        objectsToDelete(),
        objectsToDeleteCount(),
        objectTimers(),
        curObjSlot(),
        longTimer(),
        stunTimers(),
        placeholderTypes(),

        doorwayDir(),
        triggeredDoorCmd(),
        triggeredDoorDir(),
        fromUnderground(),
        activeShots(),
        triggerShutters(),
        summonedWhirlwind(),
        powerTriforceFanfare(),
        recorderUsed(),
        candleUsed(),
        shuttersPassedDirs(),
        brightenRoom(),
        profileSlot(),
        profile(),
        curUWBlockFlags(),
        ghostCount(),
        armosCount()
{
}

WorldImpl::~WorldImpl()
{
    delete player;
    player = nullptr;

    DeleteObjects();

    if ( wallsBmp != nullptr )
    {
        al_destroy_bitmap( wallsBmp );
        wallsBmp = nullptr;
    }

    if ( doorsBmp != nullptr )
    {
        al_destroy_bitmap( doorsBmp );
        doorsBmp = nullptr;
    }
}

void WorldImpl::LoadOpenRoomContext()
{
    colCount = 32;
    rowCount = 22;
    startRow = 0;
    startCol = 0;
    tileTypeCount = 56;
    marginRight = OWMarginRight;
    marginLeft = OWMarginLeft;
    marginBottom = OWMarginBottom;
    marginTop = OWMarginTop;
}

void WorldImpl::LoadClosedRoomContext()
{
    colCount = 24;
    rowCount = 14;
    startRow = 4;
    startCol = 4;
    tileTypeCount = 9;
    marginRight = UWMarginRight;
    marginLeft = UWMarginLeft;
    marginBottom = UWMarginBottom;
    marginTop = UWMarginTop;
}

void WorldImpl::LoadMapResourcesFromDirectory( int uniqueRoomCount )
{
    Util::LoadList( directory.RoomCols, roomCols, uniqueRoomCount );

    Util::LoadResource( directory.ColTables, &colTables );

    Util::LoadList( directory.TileAttrs, tileAttrs, tileTypeCount );

    Graphics::LoadTileSheet( Sheet_Background, directory.TilesImage );
}

void WorldImpl::LoadOverworldContext()
{
    LoadOpenRoomContext();
    LoadMapResourcesFromDirectory( 124 );
    Util::LoadResource( "owPrimaryMobs.list", &primaryMobs );
    Util::LoadResource( "owSecondaryMobs.list", &secondaryMobs );
    Util::LoadList( "owTileBehaviors.dat", tileBehaviors, TileTypes );
}

void WorldImpl::LoadUnderworldContext()
{
    LoadClosedRoomContext();
    LoadMapResourcesFromDirectory( 64 );
    Util::LoadResource( "uwPrimaryMobs.list", &primaryMobs );
    Util::LoadList( "uwTileBehaviors.dat", tileBehaviors, TileTypes );
}

void WorldImpl::LoadCellarContext()
{
    LoadOpenRoomContext();

    Util::LoadList( "underworldCellarRoomCols.dat", roomCols, 2 );

    Util::LoadResource( "underworldCellarCols.tab", &colTables );

    Util::LoadList( "underworldCellarTileAttrs.dat", tileAttrs, tileTypeCount );

    Graphics::LoadTileSheet( Sheet_Background, "underworldTiles.png" );

    Util::LoadResource( "uwCellarPrimaryMobs.list", &primaryMobs );
    Util::LoadResource( "uwCellarSecondaryMobs.list", &secondaryMobs );
    Util::LoadList( "uwTileBehaviors.dat", tileBehaviors, TileTypes );
}

void WorldImpl::LoadLevel( int level )
{
    LevelDirectory::FixedString levelDirName = "";

    sprintf_s( levelDirName, "levelDir_%d_%d.dat", profile.Quest, level );

    Util::LoadList( levelDirName, &directory, 1 );

    Util::BlobResLoader<LevelInfoBlock, 1> infoBlockLoader( &infoBlock );

    Util::LoadResource( directory.LevelInfoBlock, &infoBlockLoader );

    if ( wallsBmp != nullptr )
    {
        al_destroy_bitmap( wallsBmp );
        wallsBmp = nullptr;
    }

    if ( doorsBmp != nullptr )
    {
        al_destroy_bitmap( doorsBmp );
        doorsBmp = nullptr;
    }

    tempShutterRoomId = 0;
    tempShutterDoorDir = 0;
    tempShuttersRoomId = 0;
    tempShutters = false;
    prevRoomWasCellar = false;
    darkRoomFadeStep = 0;
    memset( levelKillCounts, 0, sizeof levelKillCounts );
    memset( roomHistory, 0, sizeof roomHistory );
    whirlwindTeleporting = 0;

    if ( level == 0 )
    {
        LoadOverworldContext();
        curUWBlockFlags = nullptr;
    }
    else
    {
        LoadUnderworldContext();
        wallsBmp = al_load_bitmap( directory.Extra2 );
        doorsBmp = al_load_bitmap( directory.Extra3 );
        if ( level < 7 )
            curUWBlockFlags = profile.LevelFlags1;
        else
            curUWBlockFlags = profile.LevelFlags2;

        for ( int i = 0; i < _countof( tileMaps ); i++ )
        {
            memset( tileMaps[i].tileRefs, Tile_WallEdge, sizeof tileMaps[i].tileRefs );
        }
    }

    Graphics::LoadTileSheet( Sheet_PlayerAndItems, directory.PlayerImage, directory.PlayerSheet );

    Graphics::LoadTileSheet( Sheet_Npcs, directory.NpcImage, directory.NpcSheet );

    if ( directory.BossImage[0] != '\0' )
        Graphics::LoadTileSheet( Sheet_Boss, directory.BossImage, directory.BossSheet );

    Util::LoadList( directory.RoomAttrs, roomAttrs, Rooms );

    Util::LoadResource( directory.LevelInfoEx, &extraData );

    Util::LoadResource( directory.ObjLists, &objLists );

    Util::LoadResource( directory.Extra1, &sparseRoomAttrs );

    Direction facing = Dir_Up;

    if ( player != nullptr )
    {
        facing = player->GetFacing();
        delete player;
    }

    player = new Player();
    player->SetFacing( facing );

    // Replace room attributes, if in second quest.

    if ( level == 0 && profile.Quest == 1 )
    {
        const uint8_t* pReplacement = sparseRoomAttrs.GetItem( Sparse_RoomReplacement );
        int replacementCount = *pReplacement;
        const SparseRoomAttr*   sparseAttr = (SparseRoomAttr*) &pReplacement[2];

        for ( int i = 0; i < replacementCount; i++ )
        {
            int roomId = sparseAttr[i].roomId;
            roomAttrs[roomId] = sparseAttr[i].attrs;
        }
    }
}

void WorldImpl::Init()
{
    int sysPal[SysPaletteLength];
    Util::LoadList( "pal.dat", sysPal, SysPaletteLength );
    Graphics::LoadSystemPalette( sysPal );

    Graphics::LoadTileSheet( Sheet_Font, "font.png" );
    Graphics::LoadTileSheet( Sheet_PlayerAndItems, "playerItem.png", "playerItemsSheet.tab" );

    Util::LoadResource( "text.tab", &textTable );

    GotoFileMenu();
}

void WorldImpl::Start( int slot, const Profile& profile )
{
    this->profile = profile;
    this->profile.Hearts = Profile::GetMaxHeartsValue( DefaultHearts );
    profileSlot = slot;

    GotoLoadLevel( 0, true );
}

void WorldImpl::Update()
{
    GameMode mode = GetMode();

    if ( lastMode != mode )
    {
        if ( IsPlaying( lastMode ) && mode != Mode_WinGame )
        {
            CleanUpRoomItems();
            Graphics::DisableGrayscale();
            if ( mode != Mode_Unfurl )
            {
                OnLeavePlay();
                if ( player != nullptr )
                    player->Stop();
            }
        }

        lastMode = mode;

        if ( gameMenu != nullptr )
            delete gameMenu;
        gameMenu = nextGameMenu;
        nextGameMenu = nullptr;
    }

    UpdateFunc update = sModeFuncs[curMode];
    (this->*update)();
}

void WorldImpl::Draw()
{
    if ( statusBarVisible )
        statusBar.Draw( submenuOffsetY );

    DrawFunc draw = sDrawFuncs[curMode];
    (this->*draw)();
}

void WorldImpl::DrawRoom()
{
    DrawMap( curRoomId, curTileMapIndex, 0, 0 );
}

void World::PauseFillHearts()
{
    sWorld->pause = 2;
}

void World::LeaveRoom( Direction dir, int roomId )
{
    sWorld->GotoLeave( dir, roomId );
}

void World::LeaveCellar()
{
    sWorld->GotoLeaveCellar();
}

void World::LeaveCellarByShortcut( int targetRoomId )
{
    sWorld->curRoomId = targetRoomId;
    sWorld->TakeShortcut();
    LeaveCellar();
}

void World::Die()
{
    sWorld->GotoDie();
}

void World::UnfurlLevel()
{
    sWorld->GotoUnfurl();
}

void World::ChooseFile( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    sWorld->GotoFileMenu( summaries );
}

void World::RegisterFile( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    sWorld->GotoRegisterMenu( summaries );
}

void World::EliminateFile( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    sWorld->GotoEliminateMenu( summaries );
}

bool World::IsPlaying()
{
    return IsPlaying( sWorld->curMode );
}

bool World::IsPlaying( GameMode mode )
{
    return mode == Mode_Play
        || mode == Mode_PlayCave
        || mode == Mode_PlayCellar
        || mode == Mode_PlayShortcuts;
}

bool World::IsPlayingCave()
{
    return GetMode() == Mode_PlayCave;
}

GameMode World::GetMode()
{
    if ( sWorld->curMode == Mode_InitPlayCave )
        return Mode_PlayCave;
    if ( sWorld->curMode == Mode_InitPlayCellar )
        return Mode_PlayCellar;
    return sWorld->curMode;
}

int World::GetMarginRight()
{
    return sWorld->marginRight;
}

int World::GetMarginLeft()
{
    return sWorld->marginLeft;
}

int World::GetMarginBottom()
{
    return sWorld->marginBottom;
}

int World::GetMarginTop()
{
    return sWorld->marginTop;
}

Player* World::GetPlayer()
{
    return sWorld->player;
}

Point World::GetObservedPlayerPos()
{
    return sWorld->fakePlayerPos;
}

Ladder* World::GetLadder()
{
    return sWorld->GetLadderObj();
}

void World::SetLadder( Ladder* ladder )
{
    sWorld->SetLadderObj( ladder );
}

void World::UseRecorder()
{
    sWorld->UseRecorder();
}

void WorldImpl::UseRecorder()
{
    Sound::PushSong( Song_recorder );
    objectTimers[FluteMusicSlot] = 0x98;

    if ( IsOverworld() )
    {
        if ( IsPlaying() && state.play.roomType == PlayState::Regular )
        {
            static const uint8_t roomIds[] = 
            { 0x42, 0x06, 0x29, 0x2B, 0x30, 0x3A, 0x3C, 0x58, 0x60, 0x6E, 0x72 };

            bool makeWhirlwind = true;

            for ( int i = 0; i < _countof( roomIds ); i++ )
            {
                if ( roomIds[i] == curRoomId )
                {
                    if (   (i == 0 && profile.Quest == 0) 
                        || (i != 0 && profile.Quest != 0) )
                        makeWhirlwind = false;
                    break;
                }
            }

            if ( makeWhirlwind )
                SummonWhirlwind();
            else
                MakeFluteSecret();
        }
    }
    else
    {
        recorderUsed = 1;
    }
}

void WorldImpl::SummonWhirlwind()
{
    if ( !summonedWhirlwind
        && whirlwindTeleporting == 0 
        && IsOverworld()
        && IsPlaying()
        && state.play.roomType == PlayState::Regular 
        && GetItem( ItemSlot_TriforcePieces ) != 0 )
    {
        int slot = FindEmptyMonsterSlot();
        if ( slot >= 0 )
        {
            Whirlwind* whirlwind = new Whirlwind( 0, player->GetY() );
            SetObject( slot, whirlwind );

            summonedWhirlwind = true;
            teleportingRoomIndex = GetNextTeleportingRoomIndex();
            whirlwind->SetTeleportPrevRoomId( teleportRoomIds[teleportingRoomIndex] );
        }
    }
}

void WorldImpl::MakeFluteSecret()
{
    // TODO:
    // The original game makes a FluteSecret object (type $5E) and puts it in one of the first 9 
    // object slots that it finds going from higher to lower slots. The FluteSecret object manages 
    // the animation. See $EFA4, and the FluteSecret's update routine at $FEF4.
    // But, for now we'll keep doing it as we have been.

    if ( !state.play.uncoveredRecorderSecret && FindSparseFlag( Sparse_Recorder, curRoomId ) )
    {
        state.play.uncoveredRecorderSecret = true;
        state.play.animatingRoomColors = true;
        state.play.timer = 88;
    }
}

int World::GetRecorderUsed()
{
    return sWorld->recorderUsed;
}

void World::SetRecorderUsed( int value )
{
    sWorld->recorderUsed = value;
}

bool World::GetCandleUsed()
{
    return sWorld->candleUsed;
}

void World::SetCandleUsed()
{
    sWorld->candleUsed = true;
}

int World::GetWhirlwindTeleporting()
{
    return sWorld->whirlwindTeleporting;
}

void World::SetWhirlwindTeleporting( int value )
{
    sWorld->whirlwindTeleporting = value;
}

bool World::IsSwordBlocked()
{
    return sWorld->swordBlocked;
}

void World::SetSwordBlocked( bool value )
{
    sWorld->swordBlocked = value;
}

TileBehavior WorldImpl::GetTileBehavior( int row, int col )
{
    return (TileBehavior) tileMaps[curTileMapIndex].tileBehaviors[row][col];
}

TileBehavior WorldImpl::GetTileBehaviorXY( int x, int y )
{
    int col = x / TileWidth;
    int row = (y - TileMapBaseY) / TileHeight;

    return GetTileBehavior( row, col );
}

void World::SetMobXY( int x, int y, int mob )
{
    sWorld->SetMobXY( x, y, mob );
}

void WorldImpl::SetMobXY( int x, int y, int mob )
{
    int fineCol = x / TileWidth;
    int fineRow = (y - TileMapBaseY) / TileHeight;

    if ( fineCol < 0 || fineCol >= Columns || fineRow < 0 || fineRow >= Rows )
        return;

    SetMob( fineRow, fineCol, mob );
}

void WorldImpl::SetMob( int row, int col, int mob )
{
    (this->*loadMobFunc)( &tileMaps[curTileMapIndex], row, col, mob );

    for ( int r = row; r < row + 2; r++ )
    {
        for ( int c = col; c < col + 2; c++ )
        {
            uint8_t t = tileMaps[curTileMapIndex].tileRefs[r][c];
            tileMaps[curTileMapIndex].tileBehaviors[r][c] = tileBehaviors[t];
        }
    }

    // TODO: Will we need to run some function to initialize the map object, like in LoadLayout?
}

int World::GetInnerPalette()
{
    return sWorld->GetInnerPalette();
}

int WorldImpl::GetInnerPalette()
{
    return roomAttrs[curRoomId].GetInnerPalette();
}

Cell World::GetRandomWaterTile()
{
    return sWorld->GetRandomWaterTile();
}

Cell WorldImpl::GetRandomWaterTile()
{
    Cell waterList[Rows * Columns];
    int waterCount = 0;

    for ( int r = 0; r < Rows - 1; r++ )
    {
        for ( int c = 0; c < Columns - 1; c++ )
        {
            if (   GetTileBehavior( r,   c   ) == TileBehavior_Water
                && GetTileBehavior( r,   c+1 ) == TileBehavior_Water
                && GetTileBehavior( r+1, c   ) == TileBehavior_Water
                && GetTileBehavior( r+1, c+1 ) == TileBehavior_Water )
            {
                waterList[waterCount] = { (uint8_t) r, (uint8_t) c };
                waterCount++;
            }
        }
    }

    assert( waterCount > 0 );

    int r = Util::GetRandom( waterCount );
    Cell cell = waterList[r];
    return { (uint8_t) (cell.Row + BaseRows), cell.Col };
}

Object* World::GetObject( int slot )
{
    if ( slot == PlayerSlot )
        return sWorld->player;
    return sWorld->objects[slot];
}

void World::SetObject( int slot, Object* obj )
{
    sWorld->SetOnlyObject( slot, obj );
}

int World::FindEmptyMonsterSlot()
{
    return sWorld->FindEmptyMonsterSlot();
}

int World::FindEmptyFireSlot()
{
    for ( int i = FirstFireSlot; i < LastFireSlot; i++ )
    {
        if ( sWorld->objects[i] == nullptr )
            return i;
    }
    return -1;
}

int World::GetCurrentObjectSlot()
{
    return sWorld->curObjSlot;
}

void World::SetCurrentObjectSlot( int slot )
{
    sWorld->curObjSlot = slot;
}

int& World::GetObjectTimer( int slot )
{
    return sWorld->objectTimers[slot];
}

void World::SetObjectTimer( int slot, int value )
{
    sWorld->objectTimers[slot] = value;
}

int  World::GetStunTimer( int slot )
{
    return sWorld->stunTimers[slot];
}

void World::SetStunTimer( int slot, int value )
{
    sWorld->stunTimers[slot] = value;
}

void World::PushTile( int row, int col )
{
    sWorld->InteractTile( row, col, WorldImpl::TInteract_Push );
}

void World::TouchTile( int row, int col )
{
    sWorld->InteractTile( row, col, WorldImpl::TInteract_Touch );
}

void World::CoverTile( int row, int col )
{
    sWorld->InteractTile( row, col, WorldImpl::TInteract_Cover );
}

void WorldImpl::InteractTile( int row, int col, TileInteraction interaction )
{
    if ( row < 0 || col < 0 || row >= Rows || col >= Columns )
        return;

    TileBehavior behavior = GetTileBehavior( row, col );
    TileBehaviorFunc behaviorFunc = sBehaviorFuncs[behavior];
    (this->*behaviorFunc)( row, col, interaction );
}

bool World::CollidesWall( TileBehavior behavior )
{
    return behavior == TileBehavior_Wall
        || behavior == TileBehavior_Doorway
        || behavior == TileBehavior_Door;
}

bool WorldImpl::CollidesTile( TileBehavior behavior )
{
    return behavior >= TileBehavior_FirstSolid;
}

TileCollision World::CollidesWithTileStill( int x, int y )
{
    return sWorld->CollidesWithTile( x, y, Dir_None, 0 );
}

TileCollision World::CollidesWithTileMoving( int x, int y, Direction dir, bool isPlayer )
{
    int offset;

    if ( dir == Dir_Right )
        offset = 0x10;
    else if ( dir == Dir_Down )
        offset = 8;
    else if ( isPlayer )
        offset = -8;
    else
        offset = -0x10;

    TileCollision collision = sWorld->CollidesWithTile( x, y, dir, offset );

    // Special tile collision in top right corner of OW. Go thru the wall.
    if (   isPlayer 
        && sWorld->infoBlock.LevelNumber == 0
        && sWorld->curRoomId == 0x1F
        && Util::IsVertical( dir )
        && x == 0x80
        && y < 0x56 )
        collision.Collides = false;

    return collision;
}

TileCollision WorldImpl::CollidesWithTile( 
    int x, int y, Direction dir, int offset )
{
    y += 0xB;

    if ( Util::IsVertical( dir ) )
    {
        if ( dir == Dir_Up || y < 0xDD )
            y += offset;
    }
    else
    {
        if ( (dir == Dir_Left && x >= 0x10) || (dir == Dir_Right && x < 0xF0) )
            x += offset;
    }

    TileBehavior behavior = TileBehavior_FirstWalkable;
    uint8_t fineRow = (y - TileMapBaseY) / 8;
    uint8_t fineCol1 = x / 8;
    uint8_t fineCol2;
    uint8_t hitFineCol = fineCol1;

    if ( Util::IsVertical( dir ) )
        fineCol2 = (x + 8) / 8;
    else
        fineCol2 = fineCol1;

    for ( uint8_t c = fineCol1; c <= fineCol2; c++ )
    {
        TileBehavior curBehavior = GetTileBehavior( fineRow, c );

        if ( curBehavior == TileBehavior_Water && state.play.allowWalkOnWater )
            curBehavior = TileBehavior_GenericWalkable;

        if ( curBehavior > behavior )
        {
            behavior = curBehavior;
            hitFineCol = c;
        }
    }

    TileCollision collision = { CollidesTile( behavior ), behavior, hitFineCol, fineRow };
    return collision;
}

TileCollision World::PlayerCoversTile( int x, int y )
{
    return sWorld->PlayerCoversTile( x, y );
}

TileCollision WorldImpl::PlayerCoversTile( int x, int y )
{
    y += 3;

    TileBehavior behavior = TileBehavior_FirstWalkable;
    uint8_t fineRow1 = (y - TileMapBaseY) / 8;
    uint8_t fineRow2 = (y + 15 - TileMapBaseY) / 8;
    uint8_t fineCol1 = x / 8;
    uint8_t fineCol2 = (x + 15) / 8;
    uint8_t hitFineCol = fineCol1;
    uint8_t hitFineRow = fineRow1;

    for ( uint8_t r = fineRow1; r <= fineRow2; r++ )
    {
        for ( uint8_t c = fineCol1; c <= fineCol2; c++ )
        {
            TileBehavior curBehavior = GetTileBehavior( r, c );

            if ( curBehavior == TileBehavior_Water && state.play.allowWalkOnWater )
                curBehavior = TileBehavior_GenericWalkable;

            // TODO: this isn't the best way to check covered tiles
            //       but it'll do for now.
            if ( curBehavior > behavior )
            {
                behavior = curBehavior;
                hitFineCol = c;
                hitFineRow = r;
            }
        }
    }

    TileCollision collision = { false, behavior, hitFineCol, hitFineRow };
    return collision;
}

void World::OnPushedBlock()
{
    sWorld->OnPushedBlock();
}

void WorldImpl::OnPushedBlock()
{
    Sound::PlayEffect( SEffect_secret );

    if ( IsOverworld() )
    {
        if ( !GotShortcut( curRoomId ) )
        {
            if ( FindSparseFlag( Sparse_Shortcut, curRoomId ) )
            {
                TakeShortcut();
                ShowShortcutStairs( curRoomId, curTileMapIndex );
            }
        }
    }
    else
    {
        UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
        int secret = uwRoomAttrs.GetSecret();

        if ( secret == Secret_BlockDoor )
        {
            triggerShutters = true;
        }
        else if ( secret == Secret_BlockStairs )
        {
            AddUWRoomStairs();
        }
    }
}

void World::OnActivatedArmos( int x, int y )
{
    sWorld->OnActivatedArmos( x, y );
}

void WorldImpl::OnActivatedArmos( int x, int y )
{
    const SparsePos2*   pos = FindSparsePos2( Sparse_ArmosStairs, curRoomId );

    if ( pos != nullptr && x == pos->x && y == pos->y )
    {
        SetMobXY( x, y, Mob_Stairs );
        Sound::PlayEffect( SEffect_secret );
    }
    else
    {
        SetMobXY( x, y, Mob_Ground );
    }

    if ( !GotItem() )
    {
        const SparseRoomItem* roomItem = FindSparseItem( Sparse_ArmosItem, curRoomId );

        if ( roomItem != nullptr && x == roomItem->x && y == roomItem->y )
        {
            ItemObj* itemObj = new ItemObj( roomItem->itemId, roomItem->x, roomItem->y, true );
            objects[ItemSlot] = itemObj;
        }
    }
}

void World::OnTouchedPowerTriforce()
{
    sWorld->OnTouchedPowerTriforce();
}

void WorldImpl::OnTouchedPowerTriforce()
{
    powerTriforceFanfare = true;
    player->SetState( Player::Paused );
    player->SetObjectTimer( 0xC0 );

    const static uint8_t palette[] = { 0, 0x0F, 0x10, 0x30 };
    Graphics::SetPaletteIndexed( LevelFgPalette, palette );
    Graphics::UpdatePalettes();
}

void WorldImpl::CheckPowerTriforceFanfare()
{
    if ( !powerTriforceFanfare )
        return;

    if ( player->GetObjectTimer() == 0 )
    {
        powerTriforceFanfare = false;
        player->SetState( Player::Idle );
        World::AddItem( Item_PowerTriforce );
        SetPilePalette();
        Graphics::UpdatePalettes();
        Sound::PlaySong( Song_level9, Sound::MainSongStream, true );
    }
    else
    {
        uint timer = player->GetObjectTimer();
        if ( timer & 4 )
            SetFlashPalette();
        else
            SetLevelPalette();
    }
}

void WorldImpl::AdjustInventory()
{
    if ( profile.SelectedItem == 0 )
        profile.SelectedItem = ItemSlot_Boomerang;

    for ( int i = 0; i < 10; i++ )
    {
        if (   profile.SelectedItem == ItemSlot_Arrow
            || profile.SelectedItem == ItemSlot_Bow )
        {
            if (   profile.Items[ItemSlot_Arrow] != 0
                && profile.Items[ItemSlot_Bow] != 0 )
                break;
        }
        else
        {
            if ( profile.Items[profile.SelectedItem] != 0 )
                break;
        }

        switch ( profile.SelectedItem )
        {
        case 0x07: profile.SelectedItem = 0x0F; break;
        case 0x0F: profile.SelectedItem = 0x06; break;
        case 0x01: profile.SelectedItem = 0x1B; break;
        case 0x1B: profile.SelectedItem = 0x08; break;
        default:   profile.SelectedItem--; break;
        }
    }
}

void WorldImpl::WarnLowHPIfNeeded()
{
    if ( profile.Hearts >= 0x100 )
        return;

    Sound::PlayEffect( SEffect_low_hp );
}

void WorldImpl::PlayAmbientSounds()
{
    bool playedSound = false;

    if ( IsOverworld() )
    {
        if ( GetMode() == Mode_Play )
        {
            OWRoomAttrs& owRoomAttrs = (OWRoomAttrs&) roomAttrs[curRoomId];
            if ( owRoomAttrs.HasAmbientSound() )
            {
                Sound::PlayEffect( SEffect_sea, true, Sound::AmbientInstance );
                playedSound = true;
            }
        }
    }
    else
    {
        if ( curUWBlockFlags[infoBlock.BossRoomId].GetObjCount() == 0 )
        {
            UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
            int ambientSound = uwRoomAttrs.GetAmbientSound();
            if ( ambientSound != 0 )
            {
                int id = SEffect_boss_roar1 + ambientSound - 1;
                Sound::PlayEffect( id, true, Sound::AmbientInstance );
                playedSound = true;
            }
        }
    }

    if ( !playedSound )
        Sound::StopEffects();
}

void WorldImpl::ShowShortcutStairs( int roomId, int tileMapIndex )
{
    OWRoomAttrs& owRoomAttrs = (OWRoomAttrs&) roomAttrs[roomId];
    int index = owRoomAttrs.GetShortcutStairsIndex();
    int pos = infoBlock.ShortcutPosition[index];
    int row;
    int col;
    GetRoomCoord( pos, row, col );
    SetMob( row * 2, col * 2, Mob_Stairs );
}

void WorldImpl::DrawMap( int roomId, int mapIndex, int offsetX, int offsetY )
{
    Graphics::Begin();

    int outerPalette = roomAttrs[roomId].GetOuterPalette();
    int innerPalette = roomAttrs[roomId].GetInnerPalette();
    TileMap*    map = &tileMaps[mapIndex];

    if ( IsUWCellar( roomId ) 
        || IsPlayingCave() )
    {
        outerPalette = 3;
        innerPalette = 2;
    }

    int firstRow = 0;
    int lastRow = Rows;
    int tileOffsetY = offsetY;

    int firstCol = 0;
    int lastCol = Columns;
    int tileOffsetX = offsetX;

    if ( offsetY < 0 )
    {
        firstRow = -offsetY / TileHeight;
        tileOffsetY = -(-offsetY % TileHeight);
    }
    else if ( offsetY > 0 )
    {
        lastRow = Rows - offsetY / TileHeight;
    }
    else if ( offsetX < 0 )
    {
        firstCol = -offsetX / TileWidth;
        tileOffsetX = -(-offsetX % TileWidth);
    }
    else if ( offsetX > 0 )
    {
        lastCol = Columns - offsetX / TileWidth;
    }

    int endCol = startCol + colCount;
    int endRow = startRow + rowCount;

    int y = TileMapBaseY + tileOffsetY;

    if ( IsUWMain( roomId ) )
    {
        Graphics::DrawBitmap( 
            wallsBmp, 
            0, 0, 
            TileMapWidth, TileMapHeight, 
            offsetX, TileMapBaseY + offsetY, 
            outerPalette, 0 );
    }

    for ( int r = firstRow; r < lastRow; r++, y += TileHeight )
    {
        if ( r < startRow || r >= endRow )
            continue;

        int x = tileOffsetX;

        for ( int c = firstCol; c < lastCol; c++, x += TileWidth )
        {
            if ( c < startCol || c >= endCol )
                continue;

            int tileRef = map->tileRefs[r][c];
            int srcX = (tileRef & 0x0F) * TileWidth;
            int srcY = ((tileRef & 0xF0) >> 4) * TileHeight;
            int palette;

            if ( r < 4 || r >= 18 || c < 4 || c >= 28 )
                palette = outerPalette;
            else
                palette = innerPalette;

            Graphics::DrawTile( 
                Sheet_Background, 
                srcX, srcY, 
                TileWidth, TileHeight, 
                x, y, 
                palette, 0 );
        }
    }

    if ( IsUWMain( roomId ) )
        DrawDoors( roomId, false, offsetX, offsetY );

    Graphics::End();
}

void WorldImpl::DrawDoors( int roomId, bool above, int offsetX, int offsetY )
{
    int outerPalette = roomAttrs[roomId].GetOuterPalette();
    int baseY = above ? DoorOverlayBaseY : DoorUnderlayBaseY;
    UWRoomAttrs& uwRoomAttr = (UWRoomAttrs&) roomAttrs[roomId];

    for ( int i = 0; i < 4; i++ )
    {
        int doorDir = Util::GetOrdDirection( i );
        int doorType = uwRoomAttr.GetDoor( i );
        bool doorState = GetDoorState( roomId, doorDir );
        if ( tempShutterDoorDir != 0 && roomId == tempShutterRoomId && doorType == DoorType_Shutter )
        {
            if ( doorDir == tempShutterDoorDir )
                doorState = true;
        }
        if ( doorType == DoorType_Shutter && tempShutters && tempShuttersRoomId == roomId )
            doorState = true;
        int doorFace = GetDoorStateFace( doorType, doorState );
        Graphics::DrawBitmap(
            doorsBmp,
            DoorWidth * doorFace,
            doorSrcYs[i] + baseY,
            DoorWidth,
            DoorHeight,
            doorPos[i].X + offsetX,
            doorPos[i].Y + offsetY,
            outerPalette,
            0 );
    }
}

Profile& World::GetProfile()
{
    return sWorld->profile;
}

uint8_t World::GetItem( int itemSlot )
{
    return sWorld->profile.Items[itemSlot];
}

void World::SetItem( int itemSlot, int value )
{
    sWorld->profile.Items[itemSlot] = value;
}

void WorldImpl::PostRupeeChange( uint8_t value, int itemSlot )
{
    uint8_t curValue = profile.Items[itemSlot];
    uint8_t newValue = curValue + value;

    if ( newValue < curValue )
        newValue = 255;

    profile.Items[itemSlot] = newValue;
}

void World::PostRupeeWin( uint8_t value )
{
    sWorld->PostRupeeChange( value, ItemSlot_RupeesToAdd );
}

void World::PostRupeeLoss( uint8_t value )
{
    sWorld->PostRupeeChange( value, ItemSlot_RupeesToSubtract );
}

void World::FillHearts( int heartValue )
{
    sWorld->FillHearts( heartValue );
}

void WorldImpl::FillHearts( int heartValue )
{
    unsigned int maxHeartValue = profile.Items[ItemSlot_HeartContainers] << 8;

    profile.Hearts += heartValue;

    if ( profile.Hearts >= maxHeartValue )
        profile.Hearts = maxHeartValue - 1;
}

void World::AddItem( int itemId )
{
    sWorld->AddItem( itemId );
}

void WorldImpl::AddItem( int itemId )
{
    if ( itemId >= Item_None )
        return;

    PlayItemSound( itemId );

    const EquipValue& equip = sItemToEquipment[itemId];
    unsigned int slot = equip.Slot;
    unsigned int value = equip.Value;

    if (   itemId == Item_Heart 
        || itemId == Item_Fairy )
    {
        unsigned int heartValue = value << 8;
        FillHearts( heartValue );
    }
    else
    {
        if ( slot == ItemSlot_Bombs )
        {
            value += profile.Items[ItemSlot_Bombs];
            if ( value > profile.Items[ItemSlot_MaxBombs] )
                value = profile.Items[ItemSlot_MaxBombs];
        }
        else if ( slot == ItemSlot_RupeesToAdd
            ||    slot == ItemSlot_Keys
            ||    slot == ItemSlot_HeartContainers )
        {
            value += profile.Items[slot];
            if ( value > 255 )
                value = 255;
        }
        else if ( itemId == Item_Compass )
        {
            if ( sWorld->infoBlock.LevelNumber < 9 )
            {
                unsigned int bit = 1 << (sWorld->infoBlock.LevelNumber - 1);
                value = profile.Items[ItemSlot_Compass] | bit;
                slot = ItemSlot_Compass;
            }
        }
        else if ( itemId == Item_Map )
        {
            if ( sWorld->infoBlock.LevelNumber < 9 )
            {
                unsigned int bit = 1 << (sWorld->infoBlock.LevelNumber - 1);
                value = profile.Items[ItemSlot_Map] | bit;
                slot = ItemSlot_Map;
            }
        }
        else if ( itemId == Item_TriforcePiece )
        {
            unsigned int bit = 1 << (sWorld->infoBlock.LevelNumber - 1);
            value = profile.Items[ItemSlot_TriforcePieces] | bit;
        }

        profile.Items[slot] = value;

        if ( slot == ItemSlot_Ring )
        {
            SetPlayerColor();
            Graphics::UpdatePalettes();
        }
        if ( slot == ItemSlot_HeartContainers )
            FillHearts( 0x100 );
    }
}

void World::DecrementItem( int itemSlot )
{
    sWorld->DecrementItem( itemSlot );
}

void WorldImpl::DecrementItem( int itemSlot )
{
    if ( profile.Items[itemSlot] != 0 )
        profile.Items[itemSlot]--;
}

bool World::HasCurrentMap()
{
    return HasCurrentLevelItem( ItemSlot_Map, ItemSlot_Map9 );
}

bool World::HasCurrentCompass()
{
    return HasCurrentLevelItem( ItemSlot_Compass, ItemSlot_Compass9 );
}

bool World::HasCurrentLevelItem( int itemSlot1To8, int itemSlot9 )
{
    return sWorld->HasCurrentLevelItem( itemSlot1To8, itemSlot9 );
}

bool WorldImpl::HasCurrentLevelItem( int itemSlot1To8, int itemSlot9 )
{
    if ( sWorld->infoBlock.LevelNumber == 0 )
        return false;

    if ( sWorld->infoBlock.LevelNumber < 9 )
    {
        int itemValue = profile.Items[itemSlot1To8];
        int bit = 1 << (sWorld->infoBlock.LevelNumber - 1);
        return (itemValue & bit) != 0;
    }

    return profile.Items[itemSlot9] != 0;
}

int World::GetRoomId()
{
    return sWorld->curRoomId;
}

DoorType World::GetDoorType( Direction dir )
{
    return GetDoorType( sWorld->curRoomId, dir );
}

DoorType World::GetDoorType( int roomId, Direction dir )
{
    int dirOrd = Util::GetDirectionOrd( dir );
    UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) sWorld->roomAttrs[roomId];
    return (DoorType) uwRoomAttrs.GetDoor( dirOrd );
}

bool World::GetEffectiveDoorState( int roomId, int doorDir )
{
    return sWorld->GetEffectiveDoorState( roomId, doorDir );
}

bool WorldImpl::GetEffectiveDoorState( int roomId, int doorDir )
{
    // TODO: the original game does it a little different, by looking at $EE.
    return sWorld->GetDoorState( roomId, doorDir )
        || (GetDoorType( (Direction) doorDir ) == DoorType_Shutter
            && sWorld->tempShutters && roomId == sWorld->tempShuttersRoomId)
        || (sWorld->tempShutterDoorDir == doorDir && roomId == sWorld->tempShutterRoomId);
}

bool WorldImpl::GetEffectiveDoorState( int doorDir )
{
    return GetEffectiveDoorState( sWorld->curRoomId, doorDir );
}

bool World::GetEffectiveDoorState( int doorDir )
{
    return sWorld->GetEffectiveDoorState( doorDir );
}

UWRoomFlags& World::GetUWRoomFlags( int curRoomId )
{
    return sWorld->curUWBlockFlags[curRoomId];
}

const World::LevelInfoBlock* World::GetLevelInfo()
{
    return &sWorld->infoBlock;
}

bool World::IsOverworld()
{
    return sWorld->infoBlock.LevelNumber == 0;
}

bool World::DoesRoomSupportLadder()
{
    return sWorld->FindSparseFlag( Sparse_Ladder, GetRoomId() );
}

int World::GetTileAction( int tileRef )
{
    uint8_t attr = sWorld->tileAttrs[tileRef];
    return TileAttr::GetAction( attr );
}

bool World::IsUWMain( int roomId )
{
    return !IsOverworld() && (sWorld->roomAttrs[roomId].GetUniqueRoomId() < 0x3E);
}

bool WorldImpl::IsUWCellar( int roomId )
{
    return !IsOverworld() && (roomAttrs[roomId].GetUniqueRoomId() >= 0x3E);
}

bool World::IsUWCellar()
{
    return sWorld->IsUWCellar( GetRoomId() );
}

bool WorldImpl::GotShortcut( int roomId )
{
    return profile.OverworldFlags[roomId].GetShortcutState();
}

bool WorldImpl::GotSecret()
{
    return profile.OverworldFlags[curRoomId].GetSecretState();
}

Util::Array<uint8_t> World::GetShortcutRooms()
{
    const uint8_t*  valueArray = sWorld->sparseRoomAttrs.GetItem( Sparse_Shortcut );

    Util::Array<uint8_t> array( valueArray[0], &valueArray[2] );
    // elemSize is at 1, but we don't need it.
    return array;
}

void WorldImpl::TakeShortcut()
{
    profile.OverworldFlags[curRoomId].SetShortcutState();
}

void World::TakeSecret()
{
    sWorld->profile.OverworldFlags[sWorld->curRoomId].SetSecretState();
}

bool World::GotItem()
{
    return GotItem( sWorld->curRoomId );
}

bool World::GotItem( int roomId )
{
    if ( IsOverworld() )
    {
        return sWorld->profile.OverworldFlags[roomId].GetItemState();
    }
    else
    {
        return sWorld->curUWBlockFlags[roomId].GetItemState();
    }
}

void World::MarkItem()
{
    if ( IsOverworld() )
    {
        sWorld->profile.OverworldFlags[sWorld->curRoomId].SetItemState();
    }
    else
    {
        sWorld->curUWBlockFlags[sWorld->curRoomId].SetItemState();
    }
}

void World::LiftItem( int itemId, uint16_t timer )
{
    sWorld->LiftItem( itemId, timer );
}

void WorldImpl::LiftItem( int itemId, uint16_t timer )
{
    if ( !IsPlaying() )
        return;

    if ( itemId == Item_None || itemId == 0 )
    {
        state.play.liftItemTimer = 0;
        state.play.liftItemId = 0;
        return;
    }

    state.play.liftItemTimer = timer;
    state.play.liftItemId = itemId;

    player->SetState( Player::Paused );
}

bool World::IsLiftingItem()
{
    if ( !IsPlaying() )
        return false;

    return sWorld->state.play.liftItemId != 0;
}

void World::OpenShutters()
{
    sWorld->tempShutters = true;
    sWorld->tempShuttersRoomId = sWorld->curRoomId;
    Sound::PlayEffect( SEffect_door );

    for ( int i = 0; i < Doors; i++ )
    {
        Direction dir = Util::GetOrdDirection( i );
        DoorType type = GetDoorType( dir );

        if ( type == DoorType_Shutter )
            sWorld->UpdateDoorTileBehavior( i );
    }
}

void World::IncrementKilledObjectCount( bool allowBombDrop )
{
    sWorld->worldKillCount++;

    if ( sWorld->helpDropCounter < 0xA )
    {
        sWorld->helpDropCounter++;
        if ( sWorld->helpDropCounter == 0xA )
        {
            if ( allowBombDrop )
                sWorld->helpDropValue++;
        }
    }
}

// $7B67
void World::ResetKilledObjectCount()
{
    sWorld->worldKillCount = 0;
    sWorld->helpDropCounter = 0;
    sWorld->helpDropValue = 0;
}

void World::IncrementRoomKillCount()
{
    sWorld->roomKillCount++;
}

void World::SetBombItemDrop()
{
    sWorld->helpDropCounter = 0xA;
    sWorld->helpDropValue = 0xA;
}

int World::GetRoomObjCount()
{
    return sWorld->roomObjCount;
}

void World::SetRoomObjCount( int value )
{
    sWorld->roomObjCount = value;
}

int  World::GetRoomObjId()
{
    return sWorld->roomObjId;
}

void World::SetObservedPlayerPos( int x, int y )
{
    sWorld->fakePlayerPos.X = x;
    sWorld->fakePlayerPos.Y = y;
}

void World::SetPersonWallY( int y )
{
    sWorld->state.play.personWallY = y;
}

int  World::GetFadeStep()
{
    return sWorld->darkRoomFadeStep;
}

void World::BeginFadeIn()
{
    if ( sWorld->darkRoomFadeStep > 0 )
        sWorld->brightenRoom = true;
}

void World::FadeIn()
{
    sWorld->FadeIn();
}

void WorldImpl::FadeIn()
{
    if ( darkRoomFadeStep == 0 )
    {
        brightenRoom = false;
        return;
    }

    int& timer = GetObjectTimer( FadeTimerSlot );

    if ( timer == 0 )
    {
        darkRoomFadeStep--;
        timer = 10;

        for ( int i = 0; i < 2; i++ )
        {
            Graphics::SetPaletteIndexed( i + 2, infoBlock.DarkPaletteSeq[darkRoomFadeStep][i] );
        }
        Graphics::UpdatePalettes();
    }
}

bool WorldImpl::UseKey()
{
    if ( GetItem( ItemSlot_MagicKey ) != 0 )
        return true;

    int keyCount = GetItem( ItemSlot_Keys );

    if ( keyCount > 0 )
    {
        keyCount--;
        SetItem( ItemSlot_Keys, keyCount );
        return true;
    }

    return false;
}

bool WorldImpl::GetDoorState( int roomId, int door )
{
    return curUWBlockFlags[roomId].GetDoorState( door );
}

void WorldImpl::SetDoorState( int roomId, int door )
{
    curUWBlockFlags[roomId].SetDoorState( door );
}

bool WorldImpl::IsRoomInHistory()
{
    for ( int i = 0; i < RoomHistoryLength; i++ )
    {
        if ( roomHistory[i] == curRoomId )
            return true;
    }
    return false;
}

void WorldImpl::AddRoomToHistory()
{
    int i = 0;

    for ( ; i < RoomHistoryLength; i++ )
    {
        if ( roomHistory[i] == curRoomId )
            break;
    }

    if ( i == RoomHistoryLength )
    {
        roomHistory[nextRoomHistorySlot] = curRoomId;
        nextRoomHistorySlot++;
        if ( nextRoomHistorySlot >= RoomHistoryLength )
            nextRoomHistorySlot = 0;
    }
}

bool WorldImpl::FindSparseFlag( int attrId, int roomId )
{
    return nullptr != Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const WorldImpl::SparsePos* WorldImpl::FindSparsePos( int attrId, int roomId )
{
    return (SparsePos*) Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const WorldImpl::SparsePos2* WorldImpl::FindSparsePos2( int attrId, int roomId )
{
    return (SparsePos2*) Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const SparseRoomItem* WorldImpl::FindSparseItem( int attrId, int roomId )
{
    return (SparseRoomItem*) Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const ObjectAttr* WorldImpl::GetObjectAttrs()
{
    return (ObjectAttr*) extraData.GetItem( Extra_ObjAttrs );
}

ObjectAttr World::GetObjectAttrs( int type )
{
    const ObjectAttr* objAttrs = sWorld->GetObjectAttrs();
    return objAttrs[type];
}

int World::GetObjectMaxHP( int type )
{
    const HPAttr* hpAttrs = (HPAttr*) sWorld->extraData.GetItem( Extra_HitPoints );
    int index = type / 2;
    return hpAttrs[index].GetHP( type );
}

int World::GetPlayerDamage( int type )
{
    const uint8_t* damageAttrs = (uint8_t*) sWorld->extraData.GetItem( Extra_PlayerDamage );
    const uint8_t damageByte = damageAttrs[type];
    return ((damageByte & 0xF) << 8) | (damageByte & 0xF0);
}

void WorldImpl::LoadRoom( int roomId, int tileMapIndex )
{
    if ( IsUWCellar( roomId ) )
    {
        LoadCellarContext();
        prevRoomWasCellar = true;
    }
    else if ( prevRoomWasCellar )
    {
        LoadUnderworldContext();
        prevRoomWasCellar = false;
    }

    curRoomId = roomId;
    curTileMapIndex = tileMapIndex;

    LoadMap( roomId, tileMapIndex );

    if ( IsOverworld() )
    {
        if ( GotShortcut( roomId ) )
        {
            if ( FindSparseFlag( Sparse_Shortcut, roomId ) )
            {
                ShowShortcutStairs( roomId, tileMapIndex );
            }
        }

        if ( !GotItem() )
        {
            const SparseRoomItem* roomItem = FindSparseItem( Sparse_Item, roomId );

            if ( roomItem != nullptr )
            {
                ItemObj* itemObj = new ItemObj( roomItem->itemId, roomItem->x, roomItem->y, true );
                objects[ItemSlot] = itemObj;
            }
        }
    }
    else
    {
        if ( !GotItem() )
        {
            UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[roomId];

            if (   uwRoomAttrs.GetSecret() != Secret_FoesItem
                && uwRoomAttrs.GetSecret() != Secret_LastBoss )
                AddUWRoomItem( roomId );
        }
    }
}

void World::AddUWRoomItem()
{
    sWorld->AddUWRoomItem( sWorld->curRoomId );
}

void WorldImpl::AddUWRoomItem( int roomId )
{
    UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[roomId];
    int itemId = uwRoomAttrs.GetItemId();

    if ( itemId != Item_None )
    {
        int     posIndex = uwRoomAttrs.GetItemPositionIndex();
        Point   pos = GetRoomItemPosition( infoBlock.ShortcutPosition[posIndex] );

        if ( itemId == Item_TriforcePiece )
            pos.X = TriforcePieceX;

        ItemObj* itemObj = new ItemObj( itemId, pos.X, pos.Y, true );
        objects[ItemSlot] = itemObj;

        if (   uwRoomAttrs.GetSecret() == Secret_FoesItem
            || uwRoomAttrs.GetSecret() == Secret_LastBoss )
            Sound::PlayEffect( SEffect_room_item );
    }
}

Direction World::GetDoorwayDir()
{
    return sWorld->doorwayDir;
}

void World::SetDoorwayDir( Direction dir )
{
    sWorld->doorwayDir = dir;
}

int  World::GetFromUnderground()
{
    return sWorld->fromUnderground;
}

void World::SetFromUnderground( int value )
{
    sWorld->fromUnderground = value;
}

int  World::GetActiveShots()
{
    return sWorld->activeShots;
}

void World::SetActiveShots( int count )
{
    sWorld->activeShots = count;
}

Direction World::GetShuttersPassedDirs()
{
    return sWorld->shuttersPassedDirs;
}

void World::SetShuttersPassedDirs( Direction dir )
{
    sWorld->shuttersPassedDirs = dir;
}

int  World::GetTriggeredDoorCmd()
{
    return sWorld->triggeredDoorCmd;
}

void World::SetTriggeredDoorCmd( int value )
{
    sWorld->triggeredDoorCmd = value;
}

Direction World::GetTriggeredDoorDir()
{
    return sWorld->triggeredDoorDir;
}

void World::SetTriggeredDoorDir( Direction dir )
{
    sWorld->triggeredDoorDir = dir;
}

void WorldImpl::LoadCaveRoom( int uniqueRoomId )
{
    curTileMapIndex = 0;

    LoadLayout( uniqueRoomId, 0, TileScheme::Overworld );
}

void WorldImpl::LoadMap( int roomId, int tileMapIndex )
{
    TileScheme  tileScheme;
    int         uniqueRoomId = roomAttrs[roomId].GetUniqueRoomId();

    if ( IsOverworld() )
    {
        tileScheme = TileScheme::Overworld;
    }
    else if ( uniqueRoomId >= 0x3E )
    {
        tileScheme = TileScheme::UnderworldCellar;
        uniqueRoomId -= 0x3E;
    }
    else
    {
        tileScheme = TileScheme::UnderworldMain;
    }

    LoadLayout( uniqueRoomId, tileMapIndex, tileScheme );

    if ( tileScheme == TileScheme::UnderworldMain )
    {
        for ( int i = 0; i < Doors; i++ )
        {
            UpdateDoorTileBehavior( roomId, tileMapIndex, i );
        }
    }
}

void WorldImpl::LoadOWMob( TileMap* map, int row, int col, int mob )
{
    int primary = primaryMobs.GetItems()[mob];

    if ( primary == 0xFF )
    {
        int index = mob * 4;
        const uint8_t* secondaries = secondaryMobs.GetItems();
        map->tileRefs[row  ][col  ] = secondaries[index+0];
        map->tileRefs[row  ][col+1] = secondaries[index+2];
        map->tileRefs[row+1][col  ] = secondaries[index+1];
        map->tileRefs[row+1][col+1] = secondaries[index+3];
    }
    else
    {
        map->tileRefs[row  ][col  ] = primary;
        map->tileRefs[row  ][col+1] = primary+2;
        map->tileRefs[row+1][col  ] = primary+1;
        map->tileRefs[row+1][col+1] = primary+3;
    }
}

void WorldImpl::LoadUWMob( TileMap* map, int row, int col, int mob )
{
    int primary = primaryMobs.GetItems()[mob];

    if ( primary < 0x70 || primary > 0xF2 )
    {
        map->tileRefs[row  ][col  ] = primary;
        map->tileRefs[row  ][col+1] = primary;
        map->tileRefs[row+1][col  ] = primary;
        map->tileRefs[row+1][col+1] = primary;
    }
    else
    {
        map->tileRefs[row  ][col  ] = primary;
        map->tileRefs[row  ][col+1] = primary+2;
        map->tileRefs[row+1][col  ] = primary+1;
        map->tileRefs[row+1][col+1] = primary+3;
    }
}

void WorldImpl::LoadLayout( int uniqueRoomId, int tileMapIndex, TileScheme tileScheme )
{
    const int MaxColumnStartOffset = (colCount/2 - 1) * rowCount/2;

    RoomCols*   columns = &roomCols[uniqueRoomId];
    TileMap*    map = &tileMaps[tileMapIndex];
    int         rowEnd = startRow + rowCount;
    bool        owLayoutFormat;

    owLayoutFormat =   tileScheme == TileScheme::Overworld 
                    || tileScheme == TileScheme::UnderworldCellar;

    switch ( tileScheme )
    {
    case TileScheme::Overworld: loadMobFunc = &WorldImpl::LoadOWMob; break;
    case TileScheme::UnderworldMain: loadMobFunc = &WorldImpl::LoadUWMob; break;
    case TileScheme::UnderworldCellar: loadMobFunc = &WorldImpl::LoadOWMob; break;
    }

    for ( int i = 0; i < colCount/2; i++ )
    {
        uint8_t columnDesc = columns->ColumnDesc[i];
        uint8_t tableIndex = (columnDesc & 0xF0) >> 4;
        uint8_t columnIndex = (columnDesc & 0x0F);

        const uint8_t* table = colTables.GetItem( tableIndex );
        int k = 0;
        int j = 0;

        for ( j = 0; j <= MaxColumnStartOffset; j++ )
        {
            uint8_t t = table[j];

            if ( (t & 0x80) != 0 )
            {
                if ( k == columnIndex )
                    break;
                k++;
            }
        }

        assert( j <= MaxColumnStartOffset );

        int c = startCol + i*2;

        for ( int r = startRow; r < rowEnd; j++ )
        {
            uint8_t t = table[j];
            uint8_t tileRef;

            if ( owLayoutFormat )
                tileRef = t & 0x3F;
            else
                tileRef = t & 0x7;

            (this->*loadMobFunc)( map, r, c, tileRef );

            uint8_t attr = tileAttrs[tileRef];
            int action = TileAttr::GetAction( attr );
            TileActionFunc actionFunc = nullptr;

            if ( action != 0 )
            {
                actionFunc = sActionFuncs[action];
                (this->*actionFunc)( r, c, TInteract_Load );
            }

            r += 2;

            if ( owLayoutFormat )
            {
                if ( (t & 0x40) != 0 && r < rowEnd )
                {
                    (this->*loadMobFunc)( map, r, c, tileRef );

                    if ( actionFunc != nullptr )
                        (this->*actionFunc)( r, c, TInteract_Load );
                    r += 2;
                }
            }
            else
            {
                int repeat = (t >> 4) & 0x7;
                for ( int m = 0; m < repeat && r < rowEnd; m++ )
                {
                    (this->*loadMobFunc)( map, r, c, tileRef );

                    if ( actionFunc != nullptr )
                        (this->*actionFunc)( r, c, TInteract_Load );
                    r += 2;
                }
            }
        }
    }

    if ( IsUWMain( curRoomId ) )
    {
        UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
        if ( uwRoomAttrs.HasBlock() )
        {
            for ( int c = startCol; c < startCol + colCount; c += 2 )
            {
                uint8_t tileRef = tileMaps[curTileMapIndex].tileRefs[UWBlockRow][c];
                if ( tileRef == Tile_Block )
                {
                    TileActionFunc actionFunc = sActionFuncs[TileAction_Block];
                    (this->*actionFunc)( UWBlockRow, c, TInteract_Load );
                    break;
                }
            }
        }
    }

    uint8_t* pTile = &map->tileRefs[0][0];
    uint8_t* pBehavior = &map->tileBehaviors[0][0];
    for ( int i = 0; i < (Rows * Columns); i++ )
    {
        uint8_t t = *pTile++;
        *pBehavior++ = tileBehaviors[t];
    }

    PatchTileBehaviors();
}

void WorldImpl::PatchTileBehaviors()
{
    PatchTileBehavior( ghostCount, ghostCells, TileBehavior_Ghost0 );
    PatchTileBehavior( armosCount, armosCells, TileBehavior_Armos0 );
}

void WorldImpl::PatchTileBehavior( int count, MobPatchCells cells, TileBehavior baseBehavior )
{
    for ( int i = 0; i < count; i++ )
    {
        int row = cells[i].Row;
        int col = cells[i].Col;
        TileBehavior behavior = (TileBehavior) (baseBehavior + 15 - i);
        tileMaps[curTileMapIndex].tileBehaviors[row    ][col    ] = behavior;
        tileMaps[curTileMapIndex].tileBehaviors[row    ][col + 1] = behavior;
        tileMaps[curTileMapIndex].tileBehaviors[row + 1][col    ] = behavior;
        tileMaps[curTileMapIndex].tileBehaviors[row + 1][col + 1] = behavior;
    }
}

void WorldImpl::UpdateDoorTileBehavior( int doorOrd )
{
    return UpdateDoorTileBehavior( curRoomId, curTileMapIndex, doorOrd );
}

void WorldImpl::UpdateDoorTileBehavior( int roomId, int tileMapIndex, int doorOrd )
{
    TileMap*    map = &tileMaps[tileMapIndex];
    Direction   dir = Util::GetOrdDirection( doorOrd );
    Cell        corner = doorCorners[doorOrd];
    DoorType    type = GetDoorType( roomId, dir );
    bool        state = GetEffectiveDoorState( roomId, dir );
    TileBehavior behavior = state
        ? doorBehaviors[type].Open
        : doorBehaviors[type].Closed;

    map->tileBehaviors[corner.Row    ][corner.Col    ] = behavior;
    map->tileBehaviors[corner.Row    ][corner.Col + 1] = behavior;
    map->tileBehaviors[corner.Row + 1][corner.Col    ] = behavior;
    map->tileBehaviors[corner.Row + 1][corner.Col + 1] = behavior;

    if ( behavior == TileBehavior_Doorway )
    {
        corner = behindDoorCorners[doorOrd];
        map->tileBehaviors[corner.Row    ][corner.Col    ] = behavior;
        map->tileBehaviors[corner.Row    ][corner.Col + 1] = behavior;
        map->tileBehaviors[corner.Row + 1][corner.Col    ] = behavior;
        map->tileBehaviors[corner.Row + 1][corner.Col + 1] = behavior;
    }
}

void WorldImpl::GotoPlay( PlayState::RoomType roomType )
{
    switch ( roomType )
    {
    case PlayState::Regular:  curMode = Mode_Play;        break;
    case PlayState::Cave:     curMode = Mode_PlayCave;    break;
    case PlayState::Cellar:   curMode = Mode_PlayCellar;  break;
    default:
        assert( false );
        curMode = Mode_Play;
        break;
    }
    curColorSeqNum = 0;
    tempShutters = false;
    roomObjCount = 0;
    roomObjId = 0;
    roomKillCount = 0;
    roomAllDead = false;
    madeRoomItem = false;
    enablePersonFireballs = false;
    ghostCount = 0;
    armosCount = 0;

    state.play.substate = PlayState::Active;
    state.play.animatingRoomColors = false;
    state.play.allowWalkOnWater = false;
    state.play.uncoveredRecorderSecret = false;
    state.play.roomType = roomType;
    state.play.liftItemTimer = 0;
    state.play.liftItemId = 0;
    state.play.personWallY = 0;

    if ( FindSparseFlag( Sparse_Dock, curRoomId ) )
    {
        int slot = FindEmptyMonsterSlot();
        Dock* dock = new Dock( 0, 0 );
        objects[slot] = dock;
    }

    // Set the level's level foreground palette before making objects,
    // so that if we make a boss, we won't override a palette that it might set.
    SetLevelFgPalette();
    Graphics::UpdatePalettes();
    PlayAmbientSounds();

    Direction dir = player->GetFacing();

    ClearRoomItemData();
    ClearRoomMonsterData();
    InitObjectTimers();
    InitStunTimers();
    InitPlaceholderTypes();
    MakeObjects( dir );
    MakeWhirlwind();
    AddRoomToHistory();
    MoveRoomItem();

    if ( !IsOverworld() )
        curUWBlockFlags[curRoomId].SetVisitState();
}

void WorldImpl::UpdatePlay()
{
    if ( state.play.substate == PlayState::Active )
    {
        if ( brightenRoom )
        {
            FadeIn();
            DecrementObjectTimers();
            DecrementStunTimers();
            return;
        }

        if ( submenu != 0 )
        {
            if ( Input::IsButtonPressing( InputButtons::Select ) )
            {
                submenu = 0;
                submenuOffsetY = 0;
                GotoContinueQuestion();
            }
            else
            {
                UpdateSubmenu();
            }
            return;
        }

        if ( pause == 0 )
        {
            if ( Input::IsButtonPressing( InputButtons::Select ) )
            {
                pause = 1;
                Sound::Pause();
                return;
            }
            else if ( Input::IsButtonPressing( InputButtons::Start ) )
            {
                submenu = 1;
                return;
            }
        }
        else if ( pause == 1 )
        {
            if ( Input::IsButtonPressing( InputButtons::Select ) )
            {
                pause = 0;
                Sound::Unpause();
            }
            return;
        }

        DecrementObjectTimers();
        DecrementStunTimers();

        if ( objectTimers[FluteMusicSlot] != 0 )
            return;

        if ( pause == 2 )
        {
            FillHeartsStep();
            return;
        }

        if ( state.play.animatingRoomColors )
            UpdateRoomColors();

        if ( IsUWMain( curRoomId ) )
            CheckBombables();

        UpdateRupees();
        UpdateLiftItem();

        curObjSlot = PlayerSlot;
        player->DecInvincibleTimer();
        player->Update();

        // The player's update might have changed the world's state.
        if ( !IsPlaying() )
            return;

        UpdateObservedPlayerPos();

        for ( curObjSlot = MaxObjects - 1; curObjSlot >= 0; curObjSlot-- )
        {
            Object* obj = objects[curObjSlot];
            if ( obj != nullptr && !obj->IsDeleted() )
            {
                if ( obj->DecoratedUpdate() )
                    HandleNormalObjectDeath();
            }
            else if ( placeholderTypes[curObjSlot] != 0 )
            {
                PutEdgeObject();
            }
        }

        DeleteDeadObjects();

        CheckSecrets();
        CheckShutters();
        UpdateDoors2();
        UpdateStatues();
        MoveRoomItem();
        CheckPowerTriforceFanfare();
        AdjustInventory();
        WarnLowHPIfNeeded();

// TEST:
#if 1
        if ( Input::IsKeyPressing( ALLEGRO_KEY_H ) )
        {
            OpenShutters();
        }

        if ( Input::IsKeyPressing( ALLEGRO_KEY_EQUALS ) )
        {
            profile.Items[ItemSlot_RupeesToAdd] += 0x20;
        }
        else if ( Input::IsKeyPressing( ALLEGRO_KEY_MINUS ) )
        {
            profile.Items[ItemSlot_RupeesToSubtract] += 0x20;
        }
#endif
    }
}

void WorldImpl::UpdateSubmenu()
{
    if ( submenu == 1 )
    {
        menu.Enable();
        submenu++;
        statusBar.EnableFeatures( StatusBar::Feature_Equipment, false );
    }
    else if ( submenu == 7 )
    {
        submenuOffsetY += 3;
        if ( submenuOffsetY >= Submenu::Height )
        {
            menu.Activate();
            submenu++;
        }
    }
    else if ( submenu == 8 )
    {
        if ( Input::IsButtonPressing( InputButtons::Start ) )
        {
            menu.Deactivate();
            submenu++;
        }
    }
    else if ( submenu == 9 )
    {
        submenuOffsetY -= 3;
        if ( submenuOffsetY == 0 )
        {
            menu.Disable();
            submenu = 0;
            statusBar.EnableFeatures( StatusBar::Feature_Equipment, true );
        }
    }
    else
    {
        submenu++;
    }

    if ( submenu != 0 )
        menu.Update();
}

void WorldImpl::CheckShutters()
{
    if ( triggerShutters )
    {
        triggerShutters = false;

        int dirs = 0;

        for ( int i = 0; i < 4; i++ )
        {
            Direction dir = Util::GetOrdDirection( i );

            if ( GetDoorType( dir ) == DoorType_Shutter 
                && !GetEffectiveDoorState( dir ) )
            {
                dirs |= dir;
            }
        }

        if ( dirs != 0 && triggeredDoorCmd == 0 )
        {
            triggeredDoorCmd = 6;
            triggeredDoorDir = (Direction) (triggeredDoorDir | 0x10);
        }
    }
}

void WorldImpl::UpdateDoors2()
{
    if ( GetMode() == Mode_EndLevel
        || objectTimers[DoorSlot] != 0
        || triggeredDoorCmd == 0 )
        return;

    if ( (triggeredDoorCmd & 1) == 0 )
    {
        triggeredDoorCmd++;
        objectTimers[DoorSlot] = 8;
    }
    else
    {
        if ( (triggeredDoorDir & 0x10) != 0 )
        {
            OpenShutters();
        }

        int d = 1;

        for ( int i = 0; i < 4; i++, d <<= 1 )
        {
            if ( (triggeredDoorDir & d) == 0 )
                continue;

            Direction dir = (Direction) d;
            DoorType type = GetDoorType( dir );

            if (   type == DoorType_Bombable
                || type == DoorType_Key
                || type == DoorType_Key2 )
            {
                if ( !GetDoorState( curRoomId, dir ) )
                {
                    Direction oppositeDir = Util::GetOppositeDir( dir );
                    int nextRoomId = GetNextRoomId( curRoomId, dir );

                    SetDoorState( curRoomId, dir );
                    SetDoorState( nextRoomId, oppositeDir );
                    if ( type != DoorType_Bombable )
                        Sound::PlayEffect( SEffect_door );
                    UpdateDoorTileBehavior( i );
                }
            }
        }

        triggeredDoorCmd = 0;
        triggeredDoorDir = Dir_None;
    }
}

int WorldImpl::GetNextTeleportingRoomIndex()
{
    Direction facing = player->GetFacing();
    bool growing = facing == Dir_Up || facing == Dir_Right;

    uint pieces = GetItem( ItemSlot_TriforcePieces );
    uint index  = teleportingRoomIndex;
    uint mask   = 1 << teleportingRoomIndex;

    if ( pieces == 0 )
        return 0;

    do
    {
        if ( growing )
        {
            index = (index + 1) & 7;
            mask <<= 1;
            if ( mask >= 0x100 )
                mask = 1;
        }
        else
        {
            index = (index - 1) & 7;
            mask >>= 1;
            if ( mask == 0 )
                mask = 0x80;
        }
    } while ( (pieces & mask) == 0 );

    return index;
}

void WorldImpl::UpdateRoomColors()
{
    if ( state.play.timer == 0 )
    {
        state.play.animatingRoomColors = false;
        const SparsePos* posAttr = FindSparsePos( Sparse_Recorder, curRoomId );
        if ( posAttr != nullptr )
        {
            int row;
            int col;
            GetRoomCoord( posAttr->pos, row, col );
            SetMob( row * 2, col * 2, Mob_Stairs );
            Sound::PlayEffect( SEffect_secret );
        }
        return;
    }

    if ( (state.play.timer % 8) == 0 )
    {
        const ColorSeq* colorSeq = (ColorSeq*) extraData.GetItem( Extra_PondColors );
        if ( curColorSeqNum < colorSeq->Length - 1 )
        {
            if ( curColorSeqNum == colorSeq->Length - 2 )
            {
                state.play.allowWalkOnWater = true;
            }

            int colorIndex = colorSeq->Colors[curColorSeqNum];
            curColorSeqNum++;
            Graphics::SetColorIndexed( 3, 3, colorIndex );
            Graphics::UpdatePalettes();
        }
    }

    state.play.timer--;
}

void WorldImpl::CheckBombables()
{
    UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];

    for ( int iBomb = FirstBombSlot; iBomb < LastBombSlot; iBomb++ )
    {
        Bomb* bomb = (Bomb*) objects[iBomb];
        if ( bomb == nullptr
            || bomb->GetLifetimeState() != Bomb::Fading )
            continue;

        int bombX = bomb->GetX() + 8;
        int bombY = bomb->GetY() + 8;

        for ( int iDoor = 0; iDoor < 4; iDoor++ )
        {
            int doorType = uwRoomAttrs.GetDoor( iDoor );
            if ( doorType == DoorType_Bombable )
            {
                int doorDir = Util::GetOrdDirection( iDoor );
                bool doorState = GetDoorState( curRoomId, doorDir );
                if ( !doorState )
                {
                    if (   abs( bombX - doorMiddles[iDoor].X ) < UWBombRadius 
                        && abs( bombY - doorMiddles[iDoor].Y ) < UWBombRadius )
                    {
                        triggeredDoorCmd = 6;
                        triggeredDoorDir = (Direction) doorDir;
                        break;
                    }
                }
            }
        }
    }
}

bool World::HasLivingObjects()
{
    return !sWorld->roomAllDead;
}

bool WorldImpl::CalcHasLivingObjects()
{
    for ( int i = MonsterSlot1; i < MonsterSlotEnd; i++ )
    {
        Object* obj = objects[i];
        if ( obj != nullptr )
        {
            ObjType type = obj->GetType();
            if (    type < Obj_Bubble1
                || (type > Obj_Bubble3 && type < Obj_Trap) )
                return true;
        }
    }

    return false;
}

void WorldImpl::CheckSecrets()
{
    if ( IsOverworld() )
        return;

    if ( !roomAllDead )
    {
        if ( !CalcHasLivingObjects() )
        {
            player->SetParalyzed( false );
            roomAllDead = true;
        }
    }

    UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
    int secret = uwRoomAttrs.GetSecret();

    switch ( secret )
    {
    case Secret_Ringleader:
        if (   objects[MonsterSlot1] == nullptr 
            || objects[MonsterSlot1]->GetType() >= Obj_Person_End )
            KillAllObjects();
        break;

    case Secret_LastBoss:
        if ( GetItem( ItemSlot_PowerTriforce ) != 0 )
            triggerShutters = true;
        break;

    // ORIGINAL: BlockDoor and BlockStairs are handled here.

    case Secret_FoesItem:
        if ( roomAllDead )
        {
            if ( !madeRoomItem && !GotItem() )
            {
                madeRoomItem = true;
                AddUWRoomItem( curRoomId );
            }
        }
        // fall thru

    case Secret_FoesDoor:
        if ( roomAllDead )
            triggerShutters = true;
        break;
    }
}

void WorldImpl::AddUWRoomStairs()
{
    SetMobXY( 0xD0, 0x60, Mob_UW_Stairs );
}

void WorldImpl::KillAllObjects()
{
    for ( int i = MonsterSlot1 + 1; i < MonsterSlotEnd; i++ )
    {
        Object* obj = objects[i];
        if (   obj != nullptr 
            && obj->GetType() < Obj_Person_End 
            && obj->GetDecoration() == 0 )
        {
            obj->SetDecoration( 0x10 );
        }
    }
}

void World::EnablePersonFireballs()
{
    sWorld->enablePersonFireballs = true;
}

void WorldImpl::MoveRoomItem()
{
    Object* foe = objects[MonsterSlot1];
    if ( foe == nullptr )
        return;

    Object* item = objects[ItemSlot];
    ObjType type = foe->GetType();

    if ( item != nullptr
        && (type == Obj_LikeLike 
         || type == Obj_Stalfos 
         || type == Obj_Gibdo) )
    {
        item->SetX( foe->GetX() );
        item->SetY( foe->GetY() );
    }
}

void WorldImpl::UpdateStatues()
{
    static const int fireballLayouts[] = { 0x24, 0x23 };

    if ( IsOverworld() )
        return;

    int pattern = -1;

    if ( enablePersonFireballs )
    {
        pattern = 2;
    }
    else
    {
        UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
        int layoutId = uwRoomAttrs.GetUniqueRoomId();

        for ( int i = 0; i < _countof( fireballLayouts ); i++ )
        {
            if ( fireballLayouts[i] == layoutId )
            {
                pattern = i;
                break;
            }
        }
    }

    if ( pattern >= 0 )
        Statues::Update( pattern );
}

void WorldImpl::OnLeavePlay()
{
    if ( lastMode == Mode_Play )
        SaveObjectCount();
}

void WorldImpl::ClearLevelData()
{
    curColorSeqNum = 0;
    darkRoomFadeStep = 0;
    curMazeStep = 0;
    tempShutterRoomId = 0;
    tempShutterDoorDir = 0;
    tempShuttersRoomId = 0;
    tempShutters = false;
    prevRoomWasCellar = false;
    whirlwindTeleporting = 0;

    roomKillCount = false;
    roomAllDead = false;
    madeRoomItem = false;
    enablePersonFireballs = false;
}

static bool IsRecurringFoe( int type )
{
    return type <  Obj_1Dodongo
        || type == Obj_RedLamnola
        || type == Obj_BlueLamnola
        || type >= Obj_Trap
        ;
}

void WorldImpl::SaveObjectCount()
{
    if ( IsOverworld() )
    {
        OWRoomFlags& flags = profile.OverworldFlags[curRoomId];
        int savedCount = flags.GetObjCount();
        int count;

        if ( roomKillCount >= roomObjCount )
        {
            count = 7;
        }
        else
        {
            count = (roomKillCount & 7) + savedCount;
            if ( count > 7 )
                count = 7;
        }

        flags.SetObjCount( count );
    }
    else
    {
        UWRoomFlags& flags = curUWBlockFlags[curRoomId];
        int count;

        if ( roomObjCount != 0 )
        {
            if ( roomKillCount == 0
                || IsRecurringFoe( roomObjId ) )
            {
                if ( roomKillCount < roomObjCount )
                {
                    levelKillCounts[curRoomId] += roomKillCount;
                    if ( levelKillCounts[curRoomId] < 3 )
                        count = levelKillCounts[curRoomId];
                    else
                        count = 2;
                    flags.SetObjCount( count );
                    return;
                }
            }
        }

        levelKillCounts[curRoomId] = 0xF;
        flags.SetObjCount( 3 );
    }
}

void WorldImpl::CalcObjCountToMake( int& objId, int& count )
{
    if ( IsOverworld() )
    {
        OWRoomFlags& flags = profile.OverworldFlags[curRoomId];

        if ( !IsRoomInHistory() && (flags.GetObjCount() == 7) )
        {
            flags.SetObjCount( 0 );
        }
        else
        {
            if ( flags.GetObjCount() == 7 )
            {
                objId = 0;
                count = 0;
            }
            else if ( flags.GetObjCount() != 0 )
            {
                int savedCount = flags.GetObjCount();
                if ( count < savedCount )
                {
                    objId = 0;
                    count = 0;
                }
                else
                {
                    count -= savedCount;
                }
            }
        }
    }
    else // Is Underworld
    {
        UWRoomFlags& flags = curUWBlockFlags[curRoomId];

        if ( IsRoomInHistory() || flags.GetObjCount() != 3 )
        {
            if ( count < levelKillCounts[curRoomId] )
            {
                objId = 0;
                count = 0;
            }
            else
            {
                count -= levelKillCounts[curRoomId];
            }
        }
        else
        {
            if ( IsRecurringFoe( objId ) )
            {
                flags.SetObjCount( 0 );
                levelKillCounts[curRoomId] = 0;
            }
            else
            {
                objId = 0;
                count = 0;
            }
        }
    }
}

void WorldImpl::UpdateObservedPlayerPos()
{
    // ORIGINAL: This happens after player updates and before player items update.

    if ( !giveFakePlayerPos )
    {
        fakePlayerPos.X = player->GetX();
        fakePlayerPos.Y = player->GetY();
    }

    // ORIGINAL: This happens after player items update and before the rest of objects update.

    if ( stunTimers[ObservedPlayerTimerSlot] == 0 )
    {
        stunTimers[ObservedPlayerTimerSlot] = Util::GetRandom( 8 );

        giveFakePlayerPos = !giveFakePlayerPos;
        if ( giveFakePlayerPos )
        {
            if ( fakePlayerPos.X == player->GetX() )
            {
                fakePlayerPos.X = fakePlayerPos.X ^ 0xFF;
                fakePlayerPos.Y = fakePlayerPos.Y ^ 0xFF;
            }
        }
    }
}

void WorldImpl::UpdateRupees()
{
    if ( (GetFrameCounter() & 1) == 0 )
    {
        unsigned int rupeesToAdd = profile.Items[ItemSlot_RupeesToAdd];
        unsigned int rupeesToSubtract = profile.Items[ItemSlot_RupeesToSubtract];

        if ( rupeesToAdd > 0 && rupeesToSubtract == 0 )
        {
            if ( profile.Items[ItemSlot_Rupees] < 255 )
                profile.Items[ItemSlot_Rupees]++;
            else
                profile.Items[ItemSlot_RupeesToAdd] = 0;

            Sound::PlayEffect( SEffect_character );
        }
        else if ( rupeesToAdd == 0 && rupeesToSubtract > 0 )
        {
            if ( profile.Items[ItemSlot_Rupees] > 0 )
                profile.Items[ItemSlot_Rupees]--;
            else
                profile.Items[ItemSlot_RupeesToSubtract] = 0;

            Sound::PlayEffect( SEffect_character );
        }

        if ( profile.Items[ItemSlot_RupeesToAdd] > 0 )
            profile.Items[ItemSlot_RupeesToAdd]--;

        if ( profile.Items[ItemSlot_RupeesToSubtract] > 0 )
            profile.Items[ItemSlot_RupeesToSubtract]--;
    }
}

void WorldImpl::UpdateLiftItem()
{
    if ( state.play.liftItemId == 0 )
        return;

    state.play.liftItemTimer--;

    if ( state.play.liftItemTimer == 0 )
    {
        state.play.liftItemId = 0;
        player->SetState( Player::Idle );
    }
    else
    {
        player->SetState( Player::Paused );
    }
}

void WorldImpl::DrawPlay()
{
    if ( submenu != 0 )
    {
        DrawSubmenu();
        return;
    }

    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    ClearScreen();
    DrawRoom();
    Graphics::ResetClip();

    Object* objOverPlayer = nullptr;

    DrawObjects( &objOverPlayer );

    if ( IsLiftingItem() )
        DrawLinkLiftingItem( state.play.liftItemId );
    else
        player->Draw();

    if ( objOverPlayer != nullptr )
        objOverPlayer->DecoratedDraw();

    if ( IsUWMain( curRoomId ) )
        DrawDoors( curRoomId, true, 0, 0 );
}

void WorldImpl::DrawSubmenu()
{
    Graphics::SetClip( 0, TileMapBaseY + submenuOffsetY, TileMapWidth, TileMapHeight - submenuOffsetY );
    ClearScreen();
    DrawMap( curRoomId, curTileMapIndex, 0, submenuOffsetY );
    Graphics::ResetClip();

    if ( IsUWMain( curRoomId ) )
        DrawDoors( curRoomId, true, 0, submenuOffsetY );

    menu.Draw( submenuOffsetY );
}

void WorldImpl::DrawObjects( Object** objOverPlayer )
{
    for ( int i = 0; i < MaxObjects; i++ )
    {
        curObjSlot = i;

        Object* obj = objects[i];
        if ( obj != nullptr && !obj->IsDeleted() )
        {
            if ( !obj->GetFlags().GetDrawAbovePlayer()
                || objOverPlayer == nullptr 
                || *objOverPlayer != nullptr )
                obj->DecoratedDraw();
            else
                *objOverPlayer = obj;
        }
    }
}

void DrawZeldaLiftingTriforce( int x, int y )
{
    SpriteImage image;
    image.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B3_Zelda_Lift );
    image.Draw( Sheet_Boss, x, y, PlayerPalette );

    DrawItem( Item_TriforcePiece, x, y - 0x10, 0 );
}

void WorldImpl::DrawLinkLiftingItem( int itemId )
{
    int animIndex = (itemId == Item_TriforcePiece) ? Anim_PI_LinkLiftHeavy : Anim_PI_LinkLiftLight;
    SpriteImage image;
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, animIndex );
    image.Draw( Sheet_PlayerAndItems, player->GetX(), player->GetY(), PlayerPalette );

    DrawItem( itemId, player->GetX(), player->GetY() - 0x10, 0 );
}

void WorldImpl::MakeObjects( Direction entryDir )
{
    if ( IsUWCellar( curRoomId ) )
    {
        MakeCellarObjects();
        return;
    }
    else if ( state.play.roomType == PlayState::Cave )
    {
        MakeCaveObjects();
        return;
    }

    RoomAttrs& roomAttr = roomAttrs[curRoomId];
    int slot = MonsterSlot1;
    int objId = roomAttr.MonsterListId;
    bool edgeObjects = false;

    if ( objId >= Obj_Person1 && objId < Obj_Person_End
        || objId == Obj_Grumble )
    {
        MakeUnderworldPerson( objId );
        return;
    }

    if ( IsOverworld() )
    {
        OWRoomAttrs& owRoomAttrs = (OWRoomAttrs&) roomAttrs[curRoomId];
        edgeObjects = owRoomAttrs.MonstersEnter();
    }

    int count = roomAttr.GetMonsterCount();

    if ( objId >= Obj_1Dodongo && objId < 0x62 )
        count = 1;

    CalcObjCountToMake( objId, count );
    roomObjCount = count;

    if ( objId > 0 && count > 0 )
    {
        bool            isList = objId >= 0x62;
        uint8_t         repeatedIds[MaxMonsters];
        const uint8_t*  list = nullptr;

        if ( isList )
        {
            int listId = objId - 0x62;
            list = objLists.GetItem( listId );
        }
        else
        {
            memset( repeatedIds, objId, count );
            list = repeatedIds;
        }

        int dirOrd = Util::GetDirectionOrd( entryDir );
        const SpotSeq* spotSeq = (SpotSeq*) extraData.GetItem( Extra_SpawnSpots );
        int spotsLen = spotSeq->Length / 4;
        const RSpot* dirSpots = &spotSeq->Spots[spotsLen * dirOrd];

        for ( int i = 0; i < count; i++, slot++ )
        {
            // An earlier objects that's made might make some objects in slots after it.
            // Maybe MakeMonster should take a reference to the current index.
            if ( objects[slot] != nullptr )
                continue;

            curObjSlot = slot;

            int type = list[slot];
            int x, y;

            if ( edgeObjects 
                && type != Obj_Zora
                && type != Obj_Armos
                && type != Obj_StandingFire
                && type != Obj_Whirlwind
                )
            {
                placeholderTypes[slot] = type;
            }
            else if ( FindSpawnPos( type, dirSpots, spotsLen, x, y ) )
            {
                Object* obj = MakeMonster( (ObjType) type, x, y );
                objects[slot] = obj;
            }
        }
    }

    if ( objects[MonsterSlot1] != nullptr )
        roomObjId = objects[MonsterSlot1]->GetType();

    if ( IsOverworld() )
    {
        OWRoomAttrs& owRoomAttr = (OWRoomAttrs&) roomAttr;
        if ( owRoomAttr.HasZora() )
        {
            curObjSlot = slot;

            Object* zora = MakeMonster( Obj_Zora, 0, 0 );
            SetObject( slot, zora );
            slot++;
        }
    }
}

void WorldImpl::MakeCellarObjects()
{
    static const int startXs[] = { 0x20, 0x60, 0x90, 0xD0 };
    const int startY = 0x9D;

    for ( int i = 0; i < 4; i++ )
    {
        curObjSlot = i;

        Object* keese = MakeMonster( Obj_BlueKeese, startXs[i], startY );
        SetObject( i, keese );
    }
}

void WorldImpl::MakeCaveObjects()
{
    OWRoomAttrs& owRoomAttrs = (OWRoomAttrs&) roomAttrs[curRoomId];
    int caveIndex = owRoomAttrs.GetCaveId() - FirstCaveIndex;

    const CaveSpecList* caves = (CaveSpecList*) extraData.GetItem( Extra_Caves );

    if ( caveIndex >= caves->Count )
        return;

    const CaveSpec*     cave = &caves->Specs[caveIndex];
    int type = Obj_Cave1 + caveIndex;

    MakePersonRoomObjects( type, cave );
}

void WorldImpl::MakeUnderworldPerson( int objId )
{
    CaveSpec    cave = { 0 };

    cave.Items[0] = Item_None;
    cave.Items[1] = Item_None;
    cave.Items[2] = Item_None;

    UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
    Secret secret = (Secret) uwRoomAttrs.GetSecret();

    if ( objId == Obj_Grumble )
    {
        cave.StringId = String_Grumble;
        cave.DwellerType = Obj_FriendlyMoblin;
    }
    else if ( secret == Secret_MoneyOrLife )
    {
        cave.StringId = String_MoneyOrLife;
        cave.DwellerType = Obj_OldMan;
        cave.Items[0] = Item_HeartContainer;
        cave.Prices[0] = 1;
        cave.Items[2] = Item_Rupee;
        cave.Prices[2] = 50;
        cave.SetShowNegative();
        cave.SetShowItems();
        cave.SetSpecial();
        cave.SetPickUp();
    }
    else
    {
        const LevelPersonStrings* stringIdTables = (LevelPersonStrings*) 
            extraData.GetItem( Extra_LevelPersonStringIds );

        int levelIndex      = infoBlock.EffectiveLevelNumber - 1;
        int levelTableIndex = levelGroups[levelIndex];
        int stringSlot      = objId - Obj_Person1;
        int stringId        = stringIdTables->StringIds[levelTableIndex][stringSlot];

        cave.DwellerType = Obj_OldMan;
        cave.StringId = stringId;

        if ( stringId == String_MoreBombs )
        {
            cave.Items[1] = Item_Rupee;
            cave.Prices[1] = 100;
            cave.SetShowNegative();
            cave.SetShowItems();
            cave.SetSpecial();
            cave.SetPickUp();
        }
    }

    MakePersonRoomObjects( objId, &cave );
}

void WorldImpl::MakePersonRoomObjects( int type, const CaveSpec* spec )
{
    static const int fireXs[] = { 0x48, 0xA8 };

    if ( spec->DwellerType != Obj_None )
    {
        curObjSlot = 0;
        Object* person = MakePerson( type, spec, 0x78, 0x80 );
        SetObject( 0, person );
    }

    for ( int i = 0; i < 2; i++ )
    {
        curObjSlot++;
        StandingFire* fire = new StandingFire( fireXs[i], 0x80 );
        SetObject( curObjSlot, fire );
    }
}

void WorldImpl::MakeWhirlwind()
{
    if ( whirlwindTeleporting != 0 )
    {
        int y = teleportYs[teleportingRoomIndex];

        whirlwindTeleporting = 2;

        Whirlwind* whirlwind = new Whirlwind( 0, y );
        SetObject( WhirlwindSlot, whirlwind );

        player->SetState( Player::Paused );
        player->SetX( whirlwind->GetX() );
        player->SetY( 0xF8 );
    }
}

bool WorldImpl::FindSpawnPos( int type, const RSpot* spots, int len, int& x, int& y )
{
    const ObjectAttr* objAttrs = GetObjectAttrs();

    int playerX = player->GetX();
    int playerY = player->GetY();
    bool noWorldCollision = !objAttrs[type].GetWorldCollision();

    int i;
    for ( i = 0; i < MaxObjListSize; i++ )
    {
        GetRSpotCoord( spots[spotIndex], x, y );
        spotIndex = (spotIndex + 1) % len;

        if ( (playerX != x || playerY != y)
            && (noWorldCollision || !CollidesWithTileStill( x, y )) )
            break;
    }

    if ( i == 9 )
        return false;

    return true;
}

void WorldImpl::PutEdgeObject()
{
    if ( stunTimers[EdgeObjTimerSlot] != 0 )
        return;

    stunTimers[EdgeObjTimerSlot] = Util::GetRandom( 4 ) + 2;

    int x = edgeX;
    int y = edgeY;

    for ( ; ; )
    {
        if ( x == 0 )
            y += 0x10;
        else if ( x == 0xF0 )
            y -= 0x10;

        if ( y == 0x40 )
            x -= 0x10;
        else if ( y == 0xE0 )
            x += 0x10;

        int row = (y / 8) - 8;
        int col = (x / 8);
        TileBehavior behavior = GetTileBehavior( row, col );

        if ( (behavior != TileBehavior_Sand) && !CollidesTile( behavior ) )
            break;
        if ( y == edgeY && x == edgeX )
            break;
    }

    edgeX = x;
    edgeY = y;

    if (   abs( player->GetX() - x ) >= 0x22 
        || abs( player->GetY() - y ) >= 0x22 )
    {
        Object* obj = MakeMonster( (ObjType) placeholderTypes[curObjSlot], x, y - 3 );
        objects[curObjSlot] = obj;
        placeholderTypes[curObjSlot] = 0;
        obj->SetDecoration( 0 );
    }
}

void WorldImpl::HandleNormalObjectDeath()
{
    Object* obj = objects[curObjSlot];
    int x = obj->GetX();
    int y = obj->GetY();
    int type = obj->GetType();

    delete obj;
    objects[curObjSlot] = nullptr;

    if (   type != Obj_ChildGel
        && type != Obj_RedKeese
        && type != Obj_DeadDummy )
    {
        int cycle = worldKillCycle + 1;
        if ( cycle == 10 )
            cycle = 0;
        worldKillCycle = cycle;

        if ( type != Obj_Zora )
            roomKillCount++;
    }

    TryDroppingItem( type, x, y );
}

void WorldImpl::TryDroppingItem( int origType, int x, int y )
{
    static const uint8_t classBases[] = { 0, 10, 20, 30 };
    static const uint8_t classRates[] = { 0x50, 0x98, 0x68, 0x68 };
    static const uint8_t dropItems[] = 
    {
        0x22, 0x18, 0x22, 0x18, 0x23, 0x18, 0x22, 0x22, 0x18, 0x18, 0x0F, 0x18, 0x22, 0x18, 0x0F, 0x22, 
        0x21, 0x18, 0x18, 0x18, 0x22, 0x00, 0x18, 0x21, 0x18, 0x22, 0x00, 0x18, 0x00, 0x22, 0x22, 0x22, 
        0x23, 0x18, 0x22, 0x23, 0x22, 0x22, 0x22, 0x18
    };

    if ( curObjSlot == MonsterSlot1 && (origType == Obj_Stalfos || origType == Obj_Gibdo) )
        return;

    const ObjectAttr* objAttrs = GetObjectAttrs();
    int objClass = objAttrs[origType].GetItemDropClass();
    if ( objClass == 0 )
        return;
    objClass--;

    int itemId;

    if ( worldKillCount == 0x10 )
    {
        itemId = Item_Fairy;
        helpDropCounter = 0;
        helpDropValue = 0;
    }
    else if ( helpDropCounter >= 0xA )
    {
        if ( helpDropValue == 0 )
            itemId = Item_5Rupees;
        else
            itemId = Item_Bomb;
        helpDropCounter = 0;
        helpDropValue = 0;
    }
    else
    {
        int r = Util::GetRandom( 256 );
        int rate = classRates[objClass];

        if ( r >= rate )
            return;

        int classIndex = classBases[objClass] + worldKillCycle;
        itemId = dropItems[classIndex];
    }

    Object* obj = MakeItem( itemId, x, y, false );
    objects[curObjSlot] = obj;
}

void WorldImpl::FillHeartsStep()
{
    Sound::PlayEffect( SEffect_character );

    Profile& profile = World::GetProfile();
    int maxHeartsValue = profile.GetMaxHeartsValue();

    World::FillHearts( 6 );

    if ( profile.Hearts == maxHeartsValue )
    {
        pause = 0;
        World::SetSwordBlocked( false );
    }
}

void WorldImpl::GotoScroll( Direction dir )
{
    assert( dir != Dir_None );

    state.scroll.curRoomId = curRoomId;
    state.scroll.scrollDir = dir;
    state.scroll.substate = ScrollState::Start;
    curMode = Mode_Scroll;
}

void WorldImpl::GotoScroll( Direction dir, int currentRoomId )
{
    GotoScroll( dir );
    state.scroll.curRoomId = currentRoomId;
}

bool WorldImpl::CalcMazeStayPut( Direction dir )
{
    if ( !IsOverworld() )
        return false;

    bool stayPut = false;
    const SparseMaze* maze 
        = (SparseMaze*) Util::FindSparseAttr( sparseRoomAttrs, Sparse_Maze, curRoomId );
    if ( maze != nullptr )
    {
        if ( dir != maze->exitDir )
        {
            if ( dir == maze->path[curMazeStep] )
            {
                curMazeStep++;
                if ( curMazeStep == _countof( maze->path ) )
                {
                    curMazeStep = 0;
                    Sound::PlayEffect( SEffect_secret );
                }
                else
                    stayPut = true;
            }
            else
            {
                curMazeStep = 0;
                stayPut = true;
            }
        }
        else
        {
            curMazeStep = 0;
        }
    }
    return stayPut;
}

void WorldImpl::UpdateScroll()
{
    UpdateFunc update = sScrollFuncs[state.scroll.substate];
    (this->*update)();
}

void WorldImpl::UpdateScroll_Start()
{
    int roomRow;
    int roomCol;

    GetWorldCoord( state.scroll.curRoomId, roomRow, roomCol );
    Util::MoveSimple( roomCol, roomRow, state.scroll.scrollDir, 1 );

    int nextRoomId;
    if ( CalcMazeStayPut( state.scroll.scrollDir ) )
        nextRoomId = state.scroll.curRoomId;
    else
        nextRoomId = MakeRoomId( roomRow, roomCol );

    state.scroll.nextRoomId = nextRoomId;
    state.scroll.substate = ScrollState::AnimatingColors;
}

void WorldImpl::UpdateScroll_AnimatingColors()
{
    if ( curColorSeqNum == 0 )
    {
        state.scroll.substate = ScrollState::LoadRoom;
        return;
    }

    if ( (GetFrameCounter() & 4) != 0 )
    {
        curColorSeqNum--;

        const ColorSeq* colorSeq = (ColorSeq*) extraData.GetItem( Extra_PondColors );
        int color = colorSeq->Colors[curColorSeqNum];
        Graphics::SetColorIndexed( 3, 3, color );
        Graphics::UpdatePalettes();

        if ( curColorSeqNum == 0 )
            state.scroll.substate = ScrollState::LoadRoom;
    }
}

void WorldImpl::UpdateScroll_FadeOut()
{
    if ( state.scroll.timer == 0 )
    {
        for ( int i = 0; i < 2; i++ )
        {
            Graphics::SetPaletteIndexed( i + 2, infoBlock.DarkPaletteSeq[darkRoomFadeStep][i] );
        }
        Graphics::UpdatePalettes();

        darkRoomFadeStep++;

        if ( darkRoomFadeStep == 4 )
        {
            state.scroll.substate = ScrollState::Scroll;
            state.scroll.timer = ScrollState::StateTime;
        }
        else
        {
            state.scroll.timer = 9;
        }
    }
    else
    {
        state.scroll.timer--;
    }
}

void WorldImpl::UpdateScroll_LoadRoom()
{
    if ( state.scroll.scrollDir == Dir_Down 
        && !IsOverworld()
        && curRoomId == infoBlock.StartRoomId )
    {
        GotoLoadLevel( 0 );
        return;
    }

    state.scroll.offsetX = 0;
    state.scroll.offsetY = 0;
    state.scroll.speedX = 0;
    state.scroll.speedY = 0;
    state.scroll.oldMapToNewMapDistX = 0;
    state.scroll.oldMapToNewMapDistY = 0;

    switch ( state.scroll.scrollDir )
    {
    case Dir_Left:
        state.scroll.offsetX = -TileMapWidth;
        state.scroll.speedX = ScrollSpeed;
        state.scroll.oldMapToNewMapDistX = TileMapWidth;
        break;

    case Dir_Right:
        state.scroll.offsetX = TileMapWidth;
        state.scroll.speedX = -ScrollSpeed;
        state.scroll.oldMapToNewMapDistX = -TileMapWidth;
        break;

    case Dir_Up:
        state.scroll.offsetY = -TileMapHeight;
        state.scroll.speedY = ScrollSpeed;
        state.scroll.oldMapToNewMapDistY = TileMapHeight;
        break;

    case Dir_Down:
        state.scroll.offsetY = TileMapHeight;
        state.scroll.speedY = -ScrollSpeed;
        state.scroll.oldMapToNewMapDistY = -TileMapHeight;
        break;
    }

    state.scroll.oldRoomId = curRoomId;

    int nextRoomId = state.scroll.nextRoomId;
    int nextTileMapIndex = (curTileMapIndex + 1) % 2;
    state.scroll.oldTileMapIndex = curTileMapIndex;

    tempShutterRoomId = nextRoomId;
    tempShutterDoorDir = Util::GetOppositeDir( state.scroll.scrollDir );

    LoadRoom( nextRoomId, nextTileMapIndex );

    UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[nextRoomId];
    if ( uwRoomAttrs.IsDark() && darkRoomFadeStep == 0 )
    {
        state.scroll.substate = ScrollState::FadeOut;
        state.scroll.timer = 9;
    }
    else
    {
        state.scroll.substate = ScrollState::Scroll;
        state.scroll.timer = ScrollState::StateTime;
    }
}

void WorldImpl::UpdateScroll_Scroll()
{
    if ( state.scroll.timer > 0 )
    {
        state.scroll.timer--;
        return;
    }

    if ( state.scroll.offsetX == 0 && state.scroll.offsetY == 0 )
    {
        GotoEnter( state.scroll.scrollDir );
        if ( IsOverworld() && state.scroll.nextRoomId == 0x0F )
            Sound::PlayEffect( SEffect_secret );
        return;
    }

    state.scroll.offsetX += state.scroll.speedX;
    state.scroll.offsetY += state.scroll.speedY;

    Limits playerLimits = Player::GetPlayerLimits();

    if ( state.scroll.speedX != 0 )
    {
        int x = player->GetX() + state.scroll.speedX;
        if ( x < playerLimits[1] )
            x = playerLimits[1];
        else if ( x > playerLimits[0] )
            x = playerLimits[0];
        player->SetX( x );
    }
    else
    {
        int y = player->GetY() + state.scroll.speedY;
        if ( y < playerLimits[3] )
            y = playerLimits[3];
        else if ( y > playerLimits[2] )
            y = playerLimits[2];
        player->SetY( y );
    }

    player->GetAnimator()->Advance();
}

void WorldImpl::DrawScroll()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    ClearScreen();

    if ( state.scroll.substate == ScrollState::Scroll 
        || state.scroll.substate == ScrollState::FadeOut )
    {
        int oldMapOffsetX = state.scroll.offsetX + state.scroll.oldMapToNewMapDistX;
        int oldMapOffsetY = state.scroll.offsetY + state.scroll.oldMapToNewMapDistY;

        DrawMap( curRoomId, curTileMapIndex, state.scroll.offsetX, state.scroll.offsetY );
        DrawMap( state.scroll.oldRoomId, state.scroll.oldTileMapIndex, oldMapOffsetX, oldMapOffsetY );
    }
    else
    {
        DrawMap( curRoomId, curTileMapIndex, 0, 0 );
    }

    Graphics::ResetClip();

    if ( IsOverworld() )
        player->Draw();
}

void WorldImpl::GotoLeave( Direction dir )
{
    assert( dir != Dir_None );

    state.leave.curRoomId = curRoomId;
    state.leave.scrollDir = dir;
    state.leave.timer = LeaveState::StateTime;
    curMode = Mode_Leave;
}

void WorldImpl::GotoLeave( Direction dir, int currentRoomId )
{
    GotoLeave( dir );
    state.leave.curRoomId = currentRoomId;
}

void WorldImpl::UpdateLeave()
{
    Limits playerLimits = Player::GetPlayerLimits();
    int dirOrd = Util::GetDirectionOrd( player->GetFacing() );
    uint coord = Util::IsVertical( player->GetFacing() ) ? player->GetY() : player->GetX();

    if ( coord != playerLimits[dirOrd] )
    {
        player->MoveLinear( state.leave.scrollDir, Player::WalkSpeed );
        player->GetAnimator()->Advance();
        return;
    }

    if ( state.leave.timer == 0 )
    {
        player->GetAnimator()->AdvanceFrame();
        GotoScroll( state.leave.scrollDir, state.leave.curRoomId );
        return;
    }
    state.leave.timer--;
}

void WorldImpl::DrawLeave()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    DrawRoomNoObjects();
    Graphics::ResetClip();
}

void WorldImpl::GotoEnter( Direction dir )
{
    state.enter.substate = EnterState::Start;
    state.enter.scrollDir = dir;
    state.enter.timer = 0;
    state.enter.playerPriority = SpritePri_AboveBg;
    state.enter.playerSpeed = Player::WalkSpeed;
    state.enter.gotoPlay = false;
    curMode = Mode_Enter;
}

void WorldImpl::MovePlayer( Direction dir, int speed, int& fraction )
{
    fraction += speed;
    int carry = fraction >> 8;
    fraction &= 0xFF;

    int x = player->GetX();
    int y = player->GetY();
    Util::MoveSimple( x, y, dir, carry );

    player->SetX( x );
    player->SetY( y );
}

void WorldImpl::UpdateEnter()
{
    UpdateFunc update = sEnterFuncs[state.enter.substate];
    (this->*update)();

    if ( state.enter.gotoPlay )
    {
        Direction origShutterDoorDir = (Direction) tempShutterDoorDir;
        tempShutterDoorDir = Dir_None;
        if ( IsUWMain( curRoomId )
            && (origShutterDoorDir != Dir_None)
            && GetDoorType( curRoomId, origShutterDoorDir ) == DoorType_Shutter )
        {
            Sound::PlayEffect( SEffect_door );
            int doorOrd = Util::GetDirectionOrd( origShutterDoorDir );
            UpdateDoorTileBehavior( doorOrd );
        }

        statusBar.EnableFeatures( StatusBar::Feature_All, true );
        if ( IsOverworld() && fromUnderground != 0 )
            Sound::PlaySong( infoBlock.Song, Sound::MainSongStream, true );
        GotoPlay();
        return;
    }
    player->GetAnimator()->Advance();
}

void WorldImpl::UpdateEnter_Start()
{
    triggeredDoorCmd = 0;
    triggeredDoorDir = Dir_None;

    if ( IsOverworld() )
    {
        TileBehavior behavior = GetTileBehaviorXY( player->GetX(), player->GetY() + 3 );
        if ( behavior == TileBehavior_Cave )
        {
            player->SetY( player->GetY() + MobTileHeight );
            player->SetFacing( Dir_Down );

            state.enter.playerFraction = 0;
            state.enter.playerSpeed = 0x40;
            state.enter.playerPriority = SpritePri_BelowBg;
            state.enter.scrollDir = Dir_Up;
            state.enter.targetX = player->GetX();
            state.enter.targetY = player->GetY() - 0x10;
            state.enter.substate = EnterState::WalkCave;

            Sound::StopAll();
            Sound::PlayEffect( SEffect_stairs );
        }
        else
        {
            state.enter.substate = EnterState::Wait;
            state.enter.timer = EnterState::StateTime;
        }
    }
    else if ( state.enter.scrollDir != Dir_None )
    {
        UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
        Direction oppositeDir = Util::GetOppositeDir( state.enter.scrollDir );
        int door = Util::GetDirectionOrd( oppositeDir );
        int doorType = uwRoomAttrs.GetDoor( door );
        int distance;

        if ( doorType == DoorType_Shutter || doorType == DoorType_Bombable )
            distance = MobTileWidth * 2;
        else
            distance = MobTileWidth;

        state.enter.targetX = player->GetX();
        state.enter.targetY = player->GetY();
        Util::MoveSimple( 
            state.enter.targetX, 
            state.enter.targetY, 
            state.enter.scrollDir, 
            distance );

        if ( !uwRoomAttrs.IsDark() && darkRoomFadeStep > 0 )
        {
            state.enter.substate = EnterState::FadeIn;
            state.enter.timer = 9;
        }
        else
        {
            state.enter.substate = EnterState::Walk;
        }

        player->SetFacing( state.enter.scrollDir );
    }
    else
    {
        state.enter.substate = EnterState::Wait;
        state.enter.timer = EnterState::StateTime;
    }

    if ( IsUWMain( curRoomId ) )
        doorwayDir = state.enter.scrollDir;
    else
        doorwayDir = Dir_None;
}

void WorldImpl::UpdateEnter_Wait()
{
    state.enter.timer--;
    if ( state.enter.timer == 0 )
        state.enter.gotoPlay = true;
}

void WorldImpl::UpdateEnter_FadeIn()
{
    if ( darkRoomFadeStep == 0 )
    {
        state.enter.substate = EnterState::Walk;
    }
    else
    {
        if ( state.enter.timer == 0 )
        {
            darkRoomFadeStep--;
            state.enter.timer = 9;

            for ( int i = 0; i < 2; i++ )
            {
                Graphics::SetPaletteIndexed( i + 2, infoBlock.DarkPaletteSeq[darkRoomFadeStep][i] );
            }
            Graphics::UpdatePalettes();
        }
        else
        {
            state.enter.timer--;
        }
    }
}

void WorldImpl::UpdateEnter_Walk()
{
    if (   player->GetX() == state.enter.targetX 
        && player->GetY() == state.enter.targetY )
        state.enter.gotoPlay = true;
    else
        player->MoveLinear( state.enter.scrollDir, state.enter.playerSpeed );
}

void WorldImpl::UpdateEnter_WalkCave()
{
    if (   player->GetX() == state.enter.targetX 
        && player->GetY() == state.enter.targetY )
        state.enter.gotoPlay = true;
    else
        MovePlayer( state.enter.scrollDir, state.enter.playerSpeed, state.enter.playerFraction );
}

void WorldImpl::DrawEnter()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );

    if ( state.enter.substate != EnterState::Start )
        DrawRoomNoObjects( state.enter.playerPriority );

    Graphics::ResetClip();
}

void WorldImpl::GotoLoadLevel( int level, bool restartOW )
{
    state.loadLevel.level = level;
    state.loadLevel.substate = LoadLevelState::Load;
    state.loadLevel.timer = 0;
    state.loadLevel.restartOW = restartOW;

    curMode = Mode_LoadLevel;
}

void WorldImpl::SetPlayerExitPosOW( int roomId )
{
    int             row, col;
    OWRoomAttrs&    owRoomAttrs = (OWRoomAttrs&) roomAttrs[roomId];
    RSpot           exitRPos = owRoomAttrs.GetExitPosition();

    col = exitRPos & 0xF;
    row = (exitRPos >> 4) + 4;

    player->SetX( col * MobTileWidth );
    player->SetY( row * MobTileHeight + 0xD );
}

const uint8_t* World::GetString( int stringId )
{
    return sWorld->GetString( stringId );
}

const uint8_t* WorldImpl::GetString( int stringId )
{
    return textTable.GetItem( stringId );
}

void WorldImpl::UpdateLoadLevel()
{
    if ( state.loadLevel.substate == LoadLevelState::Load )
    {
        state.loadLevel.timer = LoadLevelState::StateTime;
        state.loadLevel.substate = LoadLevelState::Wait;

        int origLevel = infoBlock.LevelNumber;
        int origRoomId = curRoomId;

        Sound::StopAll();
        statusBarVisible = false;
        LoadLevel( state.loadLevel.level );

        // Let the Unfurl game mode load the room and reset colors.

        if ( state.loadLevel.level == 0 )
        {
            curRoomId = savedOWRoomId;
            savedOWRoomId = -1;
            fromUnderground = 2;
        }
        else
        {
            curRoomId = infoBlock.StartRoomId;
            if ( origLevel == 0 )
                savedOWRoomId = origRoomId;
        }
    }
    else if ( state.loadLevel.substate == LoadLevelState::Wait )
    {
        if ( state.loadLevel.timer == 0 )
        {
            GotoUnfurl( state.loadLevel.restartOW );
            return;
        }

        state.loadLevel.timer--;
    }
}

void WorldImpl::DrawLoadLevel()
{
    Graphics::SetClip( 0, 0, StdViewWidth, StdViewHeight );
    ClearScreen();
    Graphics::ResetClip();
}

void WorldImpl::GotoUnfurl( bool restartOW )
{
    state.unfurl.substate = UnfurlState::Start;
    state.unfurl.timer = UnfurlState::StateTime;
    state.unfurl.stepTimer = 0;
    state.unfurl.left = 0x80;
    state.unfurl.right = 0x80;
    state.unfurl.restartOW = restartOW;

    ClearLevelData();

    curMode = Mode_Unfurl;
}

void WorldImpl::UpdateUnfurl()
{
    if ( state.unfurl.substate == UnfurlState::Start )
    {
        state.unfurl.substate = UnfurlState::Unfurl;
        statusBarVisible = true;
        statusBar.EnableFeatures( StatusBar::Feature_All, false );

        if ( infoBlock.LevelNumber == 0 && !state.unfurl.restartOW )
        {
            LoadRoom( curRoomId, 0 );
            SetPlayerExitPosOW( curRoomId );
        }
        else
        {
            LoadRoom( infoBlock.StartRoomId, 0 );
            player->SetX( StartX );
            player->SetY( infoBlock.StartY );
        }

        for ( int i = 0; i < LevelInfoBlock::LevelPaletteCount; i++ )
        {
            Graphics::SetPaletteIndexed( i, infoBlock.Palettes[i] );
        }

        SetPlayerColor();
        Graphics::UpdatePalettes();
        return;
    }

    if ( state.unfurl.timer > 0 )
    {
        state.unfurl.timer--;
        return;
    }

    if ( state.unfurl.left == 0 )
    {
        statusBar.EnableFeatures( StatusBar::Feature_EquipmentAndMap, true );
        if ( !IsOverworld() )
            Sound::PlaySong( infoBlock.Song, Sound::MainSongStream, true );
        GotoEnter( Dir_Up );
        return;
    }

    if ( state.unfurl.stepTimer == 0 )
    {
        state.unfurl.left -= 8;
        state.unfurl.right += 8;
        state.unfurl.stepTimer = 4;
    }
    else
    {
        state.unfurl.stepTimer--;
    }
}

void WorldImpl::DrawUnfurl()
{
    if ( state.unfurl.substate == UnfurlState::Start )
        return;

    int width = state.unfurl.right - state.unfurl.left;

    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    ClearScreen();
    Graphics::ResetClip();

    Graphics::SetClip( state.unfurl.left, TileMapBaseY, width, TileMapHeight );
    DrawRoomNoObjects( SpritePri_None );
    Graphics::ResetClip();
}

void World::EndLevel()
{
    sWorld->GotoEndLevel();
}

void WorldImpl::GotoEndLevel()
{
    state.endLevel.substate = EndLevelState::Start;
    curMode = Mode_EndLevel;
}

void WorldImpl::UpdateEndLevel()
{
    UpdateFunc update = sEndLevelFuncs[state.endLevel.substate];
    (this->*update)();
}

void WorldImpl::UpdateEndLevel_Start()
{
    state.endLevel.substate = EndLevelState::Wait1;
    state.endLevel.timer = EndLevelState::Wait1Time;

    state.endLevel.left = 0;
    state.endLevel.right = World::TileMapWidth;
    state.endLevel.stepTimer = 4;

    statusBar.EnableFeatures( StatusBar::Feature_Equipment, false );
    Sound::PlaySong( Song_triforce, Sound::MainSongStream, false );
}

void WorldImpl::UpdateEndLevel_Wait()
{
    if ( state.endLevel.timer == 0 )
    {
        if ( state.endLevel.substate == EndLevelState::Wait3 )
            GotoLoadLevel( 0 );
        else
        {
            state.endLevel.substate = (EndLevelState::Substate) (state.endLevel.substate + 1);
            if ( state.endLevel.substate == EndLevelState::Flash )
                state.endLevel.timer = EndLevelState::FlashTime;
        }
    }
    else
        state.endLevel.timer--;
}

void WorldImpl::UpdateEndLevel_Flash()
{
    if ( state.endLevel.timer == 0 )
        state.endLevel.substate = (EndLevelState::Substate) (state.endLevel.substate + 1);
    else
    {
        int step = state.endLevel.timer & 0x7;
        if ( step == 0 )
            SetFlashPalette();
        else if ( step == 3 )
            SetLevelPalette();
        state.endLevel.timer--;
    }
}

void WorldImpl::UpdateEndLevel_FillHearts()
{
    int maxHeartValue = profile.GetMaxHeartsValue();

    Sound::PlayEffect( SEffect_character );

    if ( profile.Hearts == maxHeartValue )
    {
        state.endLevel.substate = (EndLevelState::Substate) (state.endLevel.substate + 1);
        state.endLevel.timer = EndLevelState::Wait2Time;
    }
    else
    {
        FillHearts( 6 );
    }
}

void WorldImpl::UpdateEndLevel_Furl()
{
    if ( state.endLevel.left == WorldMidX )
    {
        state.endLevel.substate = (EndLevelState::Substate) (state.endLevel.substate + 1);
        state.endLevel.timer = EndLevelState::Wait3Time;
    }
    else if ( state.endLevel.stepTimer == 0 )
    {
        state.endLevel.left += 8;
        state.endLevel.right -= 8;
        state.endLevel.stepTimer = 4;
    }
    else
    {
        state.endLevel.stepTimer--;
    }
}

void WorldImpl::DrawEndLevel()
{
    int left    = 0;
    int width   = TileMapWidth;

    if ( state.endLevel.substate >= EndLevelState::Furl )
    {
        left = state.endLevel.left;
        width = state.endLevel.right - state.endLevel.left;

        Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
        ClearScreen();
        Graphics::ResetClip();

    }

    Graphics::SetClip( left, TileMapBaseY, width, TileMapHeight );
    DrawRoomNoObjects( SpritePri_None );
    Graphics::ResetClip();

    DrawLinkLiftingItem( Item_TriforcePiece );
}

void World::WinGame()
{
    sWorld->GotoWinGame();
}

void WorldImpl::GotoWinGame()
{
    state.winGame.substate = WinGameState::Start;
    state.winGame.timer = 162;
    state.winGame.left = 0;
    state.winGame.right = World::TileMapWidth;
    state.winGame.stepTimer = 0;
    state.winGame.npcVisual = WinGameState::Npc_Stand;

    curMode = Mode_WinGame;
}

void WorldImpl::UpdateWinGame()
{
    UpdateFunc update = sWinGameFuncs[state.winGame.substate];
    (this->*update)();
}

void WorldImpl::UpdateWinGame_Start()
{
    if ( state.winGame.timer > 0 )
    {
        state.winGame.timer--;
    }
    else if ( state.winGame.left == WorldMidX )
    {
        state.winGame.substate = WinGameState::Text1;
        statusBar.EnableFeatures( StatusBar::Feature_EquipmentAndMap, false );

        // A959
        const static uint8_t str1[] = {
    0x1d, 0x11, 0x0a, 0x17, 0x14, 0x1c, 0x24, 0x15, 0x12, 0x17, 0x14, 0x28, 0x22, 0x18, 0x1e, 0x2a,
    0x1b, 0x8e, 0x64, 0x1d, 0x11, 0x0e, 0x24, 0x11, 0x0e, 0x1b, 0x18, 0x24, 0x18, 0x0f, 0x24, 0x11,
    0x22, 0x1b, 0x1e, 0x15, 0x0e, 0xec
        };
        textBox1 = new TextBox( str1 );
    }
    else if ( state.winGame.stepTimer == 0 )
    {
        state.winGame.left += 8;
        state.winGame.right -= 8;
        state.winGame.stepTimer = 4;
    }
    else
    {
        state.winGame.stepTimer--;
    }
}

void WorldImpl::UpdateWinGame_Text1()
{
    textBox1->Update();
    if ( textBox1->IsDone() )
    {
        state.winGame.substate = WinGameState::Stand;
        state.winGame.timer = 76;
    }
}

void WorldImpl::UpdateWinGame_Stand()
{
    state.winGame.timer--;
    if ( state.winGame.timer == 0 )
    {
        state.winGame.substate = WinGameState::Hold1;
        state.winGame.timer = 64;
    }
}

void WorldImpl::UpdateWinGame_Hold1()
{
    state.winGame.npcVisual = WinGameState::Npc_Lift;
    state.winGame.timer--;
    if ( state.winGame.timer == 0 )
    {
        state.winGame.substate = WinGameState::Colors;
        state.winGame.timer = 127;
    }
}

void WorldImpl::UpdateWinGame_Colors()
{
    state.winGame.timer--;
    if ( state.winGame.timer == 0 )
    {
        state.winGame.substate = WinGameState::Hold2;
        state.winGame.timer = 131;
        Sound::PlaySong( Song_ending, Sound::MainSongStream, true );
    }
}

void WorldImpl::UpdateWinGame_Hold2()
{
    state.winGame.timer--;
    if ( state.winGame.timer == 0 )
    {
        // AB07
        const static uint8_t str2[] = {
    0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 
    0x0f, 0x12, 0x17, 0x0a, 0x15, 0x15, 0x22, 0x28, 
    0xa5, 0x65,
    0x19, 0x0e, 0x0a, 0x0c, 0x0e, 0x24, 0x1b, 0x0e,
    0x1d, 0x1e, 0x1b, 0x17, 0x1c, 0x24, 0x1d, 0x18, 0x24, 0x11, 0x22, 0x1b, 0x1e, 0x15, 0x0e, 0x2c,
    0xa5, 0x65, 0x65, 0x25, 0x25,
    0x1d, 0x11, 0x12, 0x1c, 0x24, 0x0e, 0x17, 0x0d, 0x1c, 0x24, 0x1d, 0x11, 0x0e, 0x24, 0x1c, 0x1d,
    0x18, 0x1b, 0x22, 0x2c, 0xe5
        };

        state.winGame.substate = WinGameState::Text2;
        textBox2 = new TextBox( str2, 8 );
        textBox2->SetY( WinGameState::TextBox2Top );
    }
}

void WorldImpl::UpdateWinGame_Text2()
{
    textBox2->Update();
    if ( textBox2->IsDone() )
    {
        state.winGame.substate = WinGameState::Hold3;
        state.winGame.timer = 129;
    }
}

void WorldImpl::UpdateWinGame_Hold3()
{
    state.winGame.timer--;
    if ( state.winGame.timer == 0 )
    {
        state.winGame.substate = WinGameState::NoObjects;
        state.winGame.timer = 32;
    }
}

void WorldImpl::UpdateWinGame_NoObjects()
{
    state.winGame.npcVisual = WinGameState::Npc_None;
    state.winGame.timer--;
    if ( state.winGame.timer == 0 )
    {
        credits = new Credits();
        state.winGame.substate = WinGameState::Credits;
    }
}

void WorldImpl::UpdateWinGame_Credits()
{
    TextBox** boxes[] = { &textBox1, &textBox2 };
    int startYs[] = { TextBox::StartY, WinGameState::TextBox2Top };

    for ( int i = 0; i < _countof( boxes ); i++ )
    {
        TextBox* box = *boxes[i];
        if ( box != nullptr )
        {
            int textToCreditsY = Credits::StartY - startYs[i];
            box->SetY( credits->GetTop() - textToCreditsY );
            int bottom = box->GetY() + box->GetHeight();
            if ( bottom <= 0 )
            {
                delete box;
                *boxes[i] = nullptr;
            }
        }
    }

    credits->Update();
    if ( credits->IsDone() )
    {
        if ( Input::IsButtonPressing( InputButtons::Start ) )
        {
            delete credits;
            credits = nullptr;
            delete player;
            player = nullptr;
            DeleteObjects();
            submenuOffsetY = 0;
            statusBarVisible = false;
            statusBar.EnableFeatures( StatusBar::Feature_All, true );

            uint8_t name[MaxNameLength];
            uint8_t nameLength = profile.NameLength;
            uint8_t deaths = profile.Deaths;
            memcpy( name, profile.Name, nameLength );
            memset( &profile, 0, sizeof profile );
            memcpy( profile.Name, name, nameLength );
            profile.NameLength = nameLength;
            profile.Deaths = deaths;
            profile.Quest = 1;
            profile.Items[ItemSlot_HeartContainers] = DefaultHearts;
            profile.Items[ItemSlot_MaxBombs] = DefaultBombs;
            SaveFolder::WriteProfile( profileSlot, profile );

            Sound::StopAll();
            GotoFileMenu();
        }
    }
    else
    {
        int statusTop = credits->GetTop() - Credits::StartY;
        int statusBottom = statusTop + StatusBar::StatusBarHeight;
        if ( statusBottom > 0 )
            submenuOffsetY = statusTop;
        else
            submenuOffsetY = -StatusBar::StatusBarHeight;
    }
}

void WorldImpl::DrawWinGame()
{
    ALLEGRO_COLOR backColor;

    Graphics::SetClip( 0, 0, StdViewWidth, StdViewHeight );
    if ( state.winGame.substate == WinGameState::Colors )
    {
        int sysColors[] = { 0x0F, 0x2A, 0x16, 0x12 };
        int frame = state.winGame.timer & 3;
        int sysColor = sysColors[frame];
        ClearScreen( sysColor );
        backColor = Graphics::GetSystemColor( sysColor );
    }
    else
    {
        ClearScreen();
        backColor = al_map_rgb( 0, 0, 0 );
    }
    Graphics::ResetClip();

    statusBar.Draw( submenuOffsetY, backColor );

    if ( state.winGame.substate == WinGameState::Start )
    {
        int left    = state.winGame.left;
        int width   = state.winGame.right - state.winGame.left;

        Graphics::SetClip( left, TileMapBaseY, width, TileMapHeight );
        DrawRoomNoObjects( SpritePri_None );
        Graphics::ResetClip();

        player->Draw();
        DrawObjects();
    }
    else
    {
        Object* zelda = objects[MonsterSlot1];

        if ( state.winGame.npcVisual == WinGameState::Npc_Stand )
        {
            zelda->Draw();
            player->Draw();
        }
        else if ( state.winGame.npcVisual == WinGameState::Npc_Lift )
        {
            DrawZeldaLiftingTriforce( zelda->GetX(), zelda->GetY() );
            DrawLinkLiftingItem( Item_TriforcePiece );
        }

        if ( credits != nullptr )
            credits->Draw();
        if ( textBox1 != nullptr )
            textBox1->Draw();
        if ( textBox2 != nullptr )
            textBox2->Draw();
    }
}

void WorldImpl::GotoStairs( TileBehavior behavior )
{
    state.stairs.substate = StairsState::Start;
    state.stairs.tileBehavior = behavior;
    state.stairs.playerPriority = SpritePri_AboveBg;

    curMode = Mode_Stairs;
}

void WorldImpl::UpdateStairsState()
{
    if ( state.stairs.substate == StairsState::Start )
    {
        state.stairs.playerPriority = SpritePri_BelowBg;

        if ( IsOverworld() )
            Sound::StopAll();

        if ( state.stairs.tileBehavior == TileBehavior_Cave )
        {
            player->SetFacing( Dir_Up );

            state.stairs.targetX = player->GetX();
            state.stairs.targetY = player->GetY() + 0x10;
            state.stairs.scrollDir = Dir_Down;
            state.stairs.playerSpeed = 0x40;
            state.stairs.playerFraction = 0;

            state.stairs.substate = StairsState::WalkCave;
            Sound::PlayEffect( SEffect_stairs );
        }
        else
        {
            state.stairs.substate = StairsState::Walk;
        }
    }
    else if ( state.stairs.substate == StairsState::Walk )
    {
        if ( IsOverworld() )
        {
            OWRoomAttrs& owRoomAttrs = (OWRoomAttrs&) roomAttrs[curRoomId];
            int cave = owRoomAttrs.GetCaveId();

            if ( cave <= 9 )
                GotoLoadLevel( cave );
            else
                GotoPlayCave();
        }
        else
            GotoPlayCellar();
    }
    else if ( state.stairs.substate == StairsState::WalkCave )
    {
        if (  player->GetX() == state.stairs.targetX 
           && player->GetY() == state.stairs.targetY )
        {
            OWRoomAttrs& owRoomAttrs = (OWRoomAttrs&) roomAttrs[curRoomId];
            int cave = owRoomAttrs.GetCaveId();

            if ( cave <= 9 )
                GotoLoadLevel( cave );
            else
                GotoPlayCave();
        }
        else
        {
            MovePlayer( state.stairs.scrollDir, state.stairs.playerSpeed, state.stairs.playerFraction );
            player->GetAnimator()->Advance();
        }
    }
}

void WorldImpl::DrawStairsState()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    DrawRoomNoObjects( state.stairs.playerPriority );
    Graphics::ResetClip();
}

void WorldImpl::GotoPlayCellar()
{
    state.playCellar.substate = PlayCellarState::Start;
    state.playCellar.playerPriority = SpritePri_None;

    curMode = Mode_InitPlayCellar;
}

void WorldImpl::UpdatePlayCellar()
{
    UpdateFunc update = sPlayCellarFuncs[state.playCellar.substate];
    (this->*update)();
}

void WorldImpl::UpdatePlayCellar_Start()
{
    state.playCellar.substate = PlayCellarState::FadeOut;
    state.playCellar.fadeTimer = 11;
    state.playCellar.fadeStep = 0;
}

void WorldImpl::UpdatePlayCellar_FadeOut()
{
    if ( state.playCellar.fadeTimer == 0 )
    {
        for ( int i = 0; i < LevelInfoBlock::FadePals; i++ )
        {
            int step = state.playCellar.fadeStep;
            Graphics::SetPaletteIndexed( i + 2, infoBlock.OutOfCellarPaletteSeq[step][i] );
        }
        Graphics::UpdatePalettes();
        state.playCellar.fadeTimer = 9;
        state.playCellar.fadeStep++;

        if ( state.playCellar.fadeStep == LevelInfoBlock::FadeLength )
            state.playCellar.substate = PlayCellarState::LoadRoom;
    }
    else
    {
        state.playCellar.fadeTimer--;
    }
}

void WorldImpl::UpdatePlayCellar_LoadRoom()
{
    bool isLeft;
    int roomId = FindCellarRoomId( curRoomId, isLeft );

    if ( roomId >= 0 )
    {
        int x = isLeft ? 0x30 : 0xC0;

        LoadRoom( roomId, 0 );

        player->SetX( x );
        player->SetY( 0x44 );
        player->SetFacing( Dir_Down );

        state.playCellar.targetY = 0x60;
        state.playCellar.substate = PlayCellarState::FadeIn;
        state.playCellar.fadeTimer = 35;
        state.playCellar.fadeStep = 3;
    }
    else
    {
        GotoPlay();
    }
}

void WorldImpl::UpdatePlayCellar_FadeIn()
{
    if ( state.playCellar.fadeTimer == 0 )
    {
        for ( int i = 0; i < LevelInfoBlock::FadePals; i++ )
        {
            int step = state.playCellar.fadeStep;
            Graphics::SetPaletteIndexed( i + 2, infoBlock.InCellarPaletteSeq[step][i] );
        }
        Graphics::UpdatePalettes();
        state.playCellar.fadeTimer = 9;
        state.playCellar.fadeStep--;

        if ( state.playCellar.fadeStep < 0 )
            state.playCellar.substate = PlayCellarState::Walk;
    }
    else
    {
        state.playCellar.fadeTimer--;
    }
}

void WorldImpl::UpdatePlayCellar_Walk()
{
    state.playCellar.playerPriority = SpritePri_AboveBg;

    if ( player->GetY() == state.playCellar.targetY )
    {
        fromUnderground = 1;
        GotoPlay( PlayState::Cellar );
    }
    else
    {
        player->MoveLinear( Dir_Down, Player::WalkSpeed );
        player->GetAnimator()->Advance();
    }
}

void WorldImpl::DrawPlayCellar()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    DrawRoomNoObjects( state.playCellar.playerPriority );
    Graphics::ResetClip();
}

void WorldImpl::GotoLeaveCellar()
{
    state.leaveCellar.substate = LeaveCellarState::Start;

    curMode = Mode_LeaveCellar;
}

void WorldImpl::UpdateLeaveCellar()
{
    UpdateFunc update = sLeaveCellarFuncs[state.leaveCellar.substate];
    (this->*update)();
}

void WorldImpl::UpdateLeaveCellar_Start()
{
    if ( IsOverworld() )
    {
        state.leaveCellar.substate = LeaveCellarState::Wait;
        state.leaveCellar.timer = 29;
    }
    else
    {
        state.leaveCellar.substate = LeaveCellarState::FadeOut;
        state.leaveCellar.fadeTimer = 11;
        state.leaveCellar.fadeStep = 0;
    }
}

void WorldImpl::UpdateLeaveCellar_FadeOut()
{
    if ( state.leaveCellar.fadeTimer == 0 )
    {
        for ( int i = 0; i < LevelInfoBlock::FadePals; i++ )
        {
            int step = state.leaveCellar.fadeStep;
            Graphics::SetPaletteIndexed( i + 2, infoBlock.InCellarPaletteSeq[step][i] );
        }
        Graphics::UpdatePalettes();
        state.leaveCellar.fadeTimer = 9;
        state.leaveCellar.fadeStep++;

        if ( state.leaveCellar.fadeStep == LevelInfoBlock::FadeLength )
            state.leaveCellar.substate = LeaveCellarState::LoadRoom;
    }
    else
    {
        state.leaveCellar.fadeTimer--;
    }
}

void WorldImpl::UpdateLeaveCellar_LoadRoom()
{
    UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
    int nextRoomId;

    if ( player->GetX() < 0x80 )
        nextRoomId = uwRoomAttrs.GetLeftCellarExitRoomId();
    else
        nextRoomId = uwRoomAttrs.GetRightCellarExitRoomId();

    LoadRoom( nextRoomId, 0 );

    player->SetX( 0x60 );
    player->SetY( 0xA0 );
    player->SetFacing( Dir_Down );

    state.leaveCellar.substate = LeaveCellarState::FadeIn;
    state.leaveCellar.fadeTimer = 35;
    state.leaveCellar.fadeStep = 3;
}

void WorldImpl::UpdateLeaveCellar_FadeIn()
{
    if ( state.leaveCellar.fadeTimer == 0 )
    {
        for ( int i = 0; i < LevelInfoBlock::FadePals; i++ )
        {
            int step = state.leaveCellar.fadeStep;
            Graphics::SetPaletteIndexed( i + 2, infoBlock.OutOfCellarPaletteSeq[step][i] );
        }
        Graphics::UpdatePalettes();
        state.leaveCellar.fadeTimer = 9;
        state.leaveCellar.fadeStep--;

        if ( state.leaveCellar.fadeStep < 0 )
            state.leaveCellar.substate = LeaveCellarState::Walk;
    }
    else
    {
        state.leaveCellar.fadeTimer--;
    }
}

void WorldImpl::UpdateLeaveCellar_Walk()
{
    GotoEnter( Dir_None );
}

void WorldImpl::UpdateLeaveCellar_Wait()
{
    if ( state.leaveCellar.timer == 0 )
    {
        state.leaveCellar.substate = LeaveCellarState::LoadOverworldRoom;
    }
    else
    {
        state.leaveCellar.timer--;
    }
}

void WorldImpl::UpdateLeaveCellar_LoadOverworldRoom()
{
    for ( int i = 0; i < 2; i++ )
    {
        Graphics::SetPaletteIndexed( i + 2, infoBlock.Palettes[i + 2] );
    }
    Graphics::UpdatePalettes();

    LoadRoom( curRoomId, 0 );
    SetPlayerExitPosOW( curRoomId );
    GotoEnter( Dir_None );
    player->SetFacing( Dir_Down );
}

void WorldImpl::DrawLeaveCellar()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );

    if ( state.leaveCellar.substate == LeaveCellarState::Start )
        ;
    else if ( state.leaveCellar.substate == LeaveCellarState::Wait 
        || state.leaveCellar.substate == LeaveCellarState::LoadOverworldRoom )
        ClearScreen();
    else
        DrawRoomNoObjects( SpritePri_None );

    Graphics::ResetClip();
}

void WorldImpl::GotoPlayCave()
{
    state.playCave.substate = PlayCaveState::Start;

    curMode = Mode_InitPlayCave;
}

void WorldImpl::UpdatePlayCave()
{
    UpdateFunc update = sPlayCaveFuncs[state.playCave.substate];
    (this->*update)();
}

void WorldImpl::UpdatePlayCave_Start()
{
    state.playCave.substate = PlayCaveState::Wait;
    state.playCave.timer = 27;
}

void WorldImpl::UpdatePlayCave_Wait()
{
    if ( state.playCave.timer == 0 )
        state.playCave.substate = PlayCaveState::LoadRoom;
    else
        state.playCave.timer--;
}

void WorldImpl::UpdatePlayCave_LoadRoom()
{
    const PaletteSet* paletteSet = (PaletteSet*) extraData.GetItem( Extra_CavePalettes );
    int caveLayout;

    if ( FindSparseFlag( Sparse_Shortcut, curRoomId ) )
        caveLayout = Cave_Shortcut;
    else
        caveLayout = Cave_Items;

    LoadCaveRoom( caveLayout );

    state.playCave.substate = PlayCaveState::Walk;
    state.playCave.targetY = 0xD5;

    player->SetX( 0x70 );
    player->SetY( 0xDD );
    player->SetFacing( Dir_Up );

    for ( int i = 0; i < 2; i++ )
    {
        Graphics::SetPaletteIndexed( i + 2, paletteSet->Palettes[i] );
    }
    Graphics::UpdatePalettes();
}

void WorldImpl::UpdatePlayCave_Walk()
{
    if ( player->GetY() == state.playCave.targetY )
    {
        fromUnderground = 1;
        GotoPlay( PlayState::Cave );
    }
    else
    {
        player->MoveLinear( Dir_Up, Player::WalkSpeed );
        player->GetAnimator()->Advance();
    }
}

void WorldImpl::DrawPlayCave()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );

    if ( state.playCave.substate == PlayCaveState::Wait 
        || state.playCave.substate == PlayCaveState::LoadRoom )
    {
        ClearScreen();
    }
    else if ( state.playCave.substate == PlayCaveState::Walk )
    {
        DrawRoomNoObjects();
    }

    Graphics::ResetClip();
}

void WorldImpl::GotoDie()
{
    state.death.substate = DeathState::Start;

    curMode = Mode_Death;
}

void WorldImpl::UpdateDie()
{
    // ORIGINAL: Some of these are handled with object timers.
    if ( state.death.timer > 0 )
        state.death.timer--;

    UpdateFunc update = sDeathFuncs[state.death.substate];
    (this->*update)();
}

void WorldImpl::UpdateDie_Start()
{
    player->SetInvincibilityTimer( 0x10 );
    state.death.timer = 0x20;
    state.death.substate = DeathState::Flash;
    Sound::StopEffects();
    Sound::PlaySong( Song_death, Sound::MainSongStream, false );
}

void WorldImpl::UpdateDie_Flash()
{
    player->DecInvincibleTimer();

    if ( state.death.timer == 0 )
    {
        state.death.timer = 6;
        state.death.substate = DeathState::Wait1;
    }
}

void WorldImpl::UpdateDie_Wait1()
{
    // TODO: the last 2 frames make the whole play area use palette 3.

    if ( state.death.timer == 0 )
    {
        static const uint8_t redPals[2][4] = 
        {
            { 0x0F, 0x17, 0x16, 0x26 },
            { 0x0F, 0x17, 0x16, 0x26 }
        };

        SetLevelPalettes( redPals );

        state.death.step = 16;
        state.death.timer = 0;
        state.death.substate = DeathState::Turn;
    }
}

void WorldImpl::UpdateDie_Turn()
{
    if ( state.death.step == 0 )
    {
        state.death.step = 4;
        state.death.timer = 0;
        state.death.substate = DeathState::Fade;
    }
    else
    {
        if ( state.death.timer == 0 )
        {
            state.death.timer = 5;
            state.death.step--;

            static const Direction dirs[] = 
            {
                Dir_Down,
                Dir_Left,
                Dir_Up,
                Dir_Right,
            };

            Direction dir = dirs[state.death.step & 3];
            player->SetFacing( dir );
        }
    }
}

void WorldImpl::UpdateDie_Fade()
{
    if ( state.death.step == 0 )
    {
        state.death.substate = DeathState::GrayLink;
    }
    else
    {
        if ( state.death.timer == 0 )
        {
            state.death.timer = 10;
            state.death.step--;

            int seq = 3 - state.death.step;

            SetLevelPalettes( infoBlock.DeathPaletteSeq[seq] );
        }
    }
}

void WorldImpl::UpdateDie_GrayLink()
{
    static const uint8_t grayPal[4] = { 0, 0x10, 0x30, 0 };

    Graphics::SetPaletteIndexed( PlayerPalette, grayPal );
    Graphics::UpdatePalettes();

    state.death.substate = DeathState::Spark;
    state.death.timer = 0x18;
    state.death.step = 0;
}

void WorldImpl::UpdateDie_Spark()
{
    if ( state.death.timer == 0 )
    {
        if ( state.death.step == 0 )
        {
            state.death.timer = 10;
            Sound::PlayEffect( SEffect_character );
        }
        else if ( state.death.step == 1 )
        {
            state.death.timer = 4;
        }
        else
        {
            state.death.substate = DeathState::Wait2;
            state.death.timer = 46;
        }
        state.death.step++;
    }
}

void WorldImpl::UpdateDie_Wait2()
{
    if ( state.death.timer == 0 )
    {
        state.death.substate = DeathState::GameOver;
        state.death.timer = 0x60;
    }
}

void WorldImpl::UpdateDie_GameOver()
{
    if ( state.death.timer == 0 )
    {
        profile.Deaths++;
        GotoContinueQuestion();
    }
}

void WorldImpl::DrawDie()
{
    if ( state.death.substate < DeathState::GameOver )
    {
        Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
        DrawRoomNoObjects( SpritePri_None );
        Graphics::ResetClip();

        if ( state.death.substate == DeathState::Spark && state.death.step > 0 )
        {
            DrawSparkle( player->GetX(), player->GetY(), BlueFgPalette, state.death.step - 1 );
        }
        else if ( state.death.substate <= DeathState::Spark )
        {
            player->Draw();
        }
    }
    else
    {
        static const uint8_t GameOver[] = { 0x10, 0x0A, 0x16, 0x0E, 0x24, 0x18, 0x1F, 0x0E, 0x1B };
        DrawString( GameOver, sizeof GameOver, 0x60, 0x90, 0 );
    }
}

void WorldImpl::GotoContinueQuestion()
{
    state.continueQuestion.substate = ContinueState::Start;
    state.continueQuestion.selectedIndex = 0;

    curMode = Mode_ContinueQuestion;
}

void WorldImpl::UpdateContinueQuestion()
{
    if ( state.continueQuestion.substate == ContinueState::Start )
    {
        statusBarVisible = false;
        Sound::PlaySong( Song_game_over, Sound::MainSongStream, true );
        state.continueQuestion.substate = ContinueState::Idle;
    }
    else if ( state.continueQuestion.substate == ContinueState::Idle )
    {
        if ( Input::IsButtonPressing( InputButtons::Select ) )
        {
            state.continueQuestion.selectedIndex++;
            if ( state.continueQuestion.selectedIndex == 3 )
                state.continueQuestion.selectedIndex = 0;
        }
        else if ( Input::IsButtonPressing( InputButtons::Start ) )
        {
            state.continueQuestion.substate = ContinueState::Chosen;
            state.continueQuestion.timer = 0x40;
        }
    }
    else if ( state.continueQuestion.substate == ContinueState::Chosen )
    {
        if ( state.continueQuestion.timer == 0 )
        {
            statusBarVisible = true;
            Sound::StopAll();

            if ( state.continueQuestion.selectedIndex == 0 )
            {
                // So, that the OW song is played in the Enter mode.
                fromUnderground = 2;
                delete player;
                player = new Player();
                profile.Hearts = Profile::GetMaxHeartsValue( DefaultHearts );
                GotoUnfurl( true );
            }
            else if ( state.continueQuestion.selectedIndex == 1 )
            {
                SaveFolder::WriteProfile( profileSlot, profile );
                GotoFileMenu();
            }
            else if ( state.continueQuestion.selectedIndex == 2 )
            {
                GotoFileMenu();
            }
        }
        else
        {
            state.continueQuestion.timer--;
        }
    }
}

void WorldImpl::DrawContinueQuestion()
{
    static const uint8_t Continue[] = { 0x0C, 0x18, 0x17, 0x1D, 0x12, 0x17, 0x1E, 0x0E };
    static const uint8_t Save[]     = { 0x1C, 0x0A, 0x1F, 0x0E, 0x24, 0x24, 0x24, 0x24 };
    static const uint8_t Retry[]    = { 0x1B, 0x0E, 0x1D, 0x1B, 0x22, 0x24, 0x24, 0x24 };
    static const uint8_t* Strs[]    = { Continue, Save, Retry };

    ClearScreen();

    int y = 0x50;

    for ( int i = 0; i < 3; i++, y += 24 )
    {
        int pal = 0;
        if ( state.continueQuestion.substate == ContinueState::Chosen 
            && state.continueQuestion.selectedIndex == i )
            pal = (GetFrameCounter() / 4) & 1;

        DrawString( Strs[i], _countof( Continue ), 0x50, y, pal );
    }

    y = 0x50 + (state.continueQuestion.selectedIndex * 24 );
    DrawChar( Char_FullHeart, 0x40, y, RedFgPalette );
}

void WorldImpl::GotoFileMenu()
{
    auto summaries = std::make_shared<ProfileSummarySnapshot>();
    SaveFolder::ReadSummaries( *summaries.get() );
    GotoFileMenu( summaries );
}

void WorldImpl::GotoFileMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    if ( nextGameMenu != nullptr )
        delete nextGameMenu;

    nextGameMenu = new GameMenu( summaries );
    curMode = Mode_GameMenu;
}

void WorldImpl::GotoRegisterMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    if ( nextGameMenu != nullptr )
        delete nextGameMenu;

    nextGameMenu = new RegisterMenu( summaries );
    curMode = Mode_Register;
}

void WorldImpl::GotoEliminateMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    if ( nextGameMenu != nullptr )
        delete nextGameMenu;

    nextGameMenu = new EliminateMenu( summaries );
    curMode = Mode_Elimination;
}

void WorldImpl::UpdateGameMenu()
{
    gameMenu->Update();
}

void WorldImpl::UpdateRegisterMenu()
{
    gameMenu->Update();
}

void WorldImpl::UpdateEliminateMenu()
{
    gameMenu->Update();
}

void WorldImpl::DrawGameMenu()
{
    if ( gameMenu != nullptr )
        gameMenu->Draw();
}

int WorldImpl::FindCellarRoomId( int mainRoomId, bool& isLeft )
{
    for ( int i = 0; i < LevelInfoBlock::LevelCellarCount; i++ )
    {
        uint8_t cellarRoomId = infoBlock.CellarRoomIds[i];

        if ( cellarRoomId >= 0x80 )
            break;

        UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[cellarRoomId];

        if ( mainRoomId == uwRoomAttrs.GetLeftCellarExitRoomId() )
        {
            isLeft = true;
            return cellarRoomId;
        }
        else if ( mainRoomId == uwRoomAttrs.GetRightCellarExitRoomId() )
        {
            isLeft = false;
            return cellarRoomId;
        }
    }

    return -1;
}

void WorldImpl::DrawRoomNoObjects( SpritePriority playerPriority )
{
    ClearScreen();

    if ( playerPriority == SpritePri_BelowBg )
        player->Draw();

    DrawRoom();

    if ( playerPriority == SpritePri_AboveBg )
        player->Draw();

    if ( IsUWMain( curRoomId ) )
        DrawDoors( curRoomId, true, 0, 0 );
}

void WorldImpl::NoneTileAction( int row, int col, TileInteraction interaction )
{
    // Nothing to do. Should never be called.
}

void WorldImpl::PushTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    Rock*   rock = new Rock();
    rock->SetX( col * TileWidth );
    rock->SetY( TileMapBaseY + row * TileHeight );
    SetBlockObj( rock );
}

void WorldImpl::BombTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    if ( GotSecret() )
    {
        SetMob( row, col, Mob_Cave );
    }
    else
    {
        RockWall*   rockWall = new RockWall();
        rockWall->SetX( col * TileWidth );
        rockWall->SetY( TileMapBaseY + row * TileHeight );
        SetBlockObj( rockWall );
    }
}

void WorldImpl::BurnTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    if ( GotSecret() )
    {
        SetMob( row, col, Mob_Stairs );
    }
    else
    {
        Tree*   tree = new Tree();
        tree->SetX( col * TileWidth );
        tree->SetY( TileMapBaseY + row * TileHeight );
        SetBlockObj( tree );
    }
}

void WorldImpl::HeadstoneTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    Headstone*  headstone = new Headstone();
    headstone->SetX( col * TileWidth );
    headstone->SetY( TileMapBaseY + row * TileHeight );
    SetBlockObj( headstone );
}

void WorldImpl::LadderTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Touch )
        return;

    _RPT2( _CRT_WARN, "Touch water: %d, %d\n", row, col );
}

void WorldImpl::RaftTileAction( int row, int col, TileInteraction interaction )
{
    // TODO: instantiate the Dock here on Load interaction, and set its position.

    if ( interaction != TInteract_Cover )
        return;

    _RPT2( _CRT_WARN, "Cover dock: %d, %d\n", row, col );

    if ( World::GetItem( ItemSlot_Raft ) == 0 )
        return;
    if ( !FindSparseFlag( Sparse_Dock, curRoomId ) )
        return;
}

void WorldImpl::CaveTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Cover )
        return;

    if ( IsOverworld() )
    {
        TileBehavior behavior = GetTileBehavior( row, col );
        GotoStairs( behavior );
    }

    _RPT2( _CRT_WARN, "Cover cave: %d, %d\n", row, col );
}

void WorldImpl::StairsTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Cover )
        return;

    if ( GetMode() == Mode_Play )
        GotoStairs( TileBehavior_Stairs );

    _RPT2( _CRT_WARN, "Cover stairs: %d, %d\n", row, col );
}

void WorldImpl::GhostTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction == TInteract_Push )
        _RPT2( _CRT_WARN, "Push headstone: %d, %d\n", row, col );

    CommonMakeObjectAction( row, col, interaction, ghostCount, ghostCells, Obj_FlyingGhini );
}

void WorldImpl::ArmosTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction == TInteract_Push )
        _RPT2( _CRT_WARN, "Push armos: %d, %d\n", row, col );

    CommonMakeObjectAction( row, col, interaction, armosCount, armosCells, Obj_Armos );
}

void WorldImpl::CommonMakeObjectAction( int row, int col, TileInteraction interaction,
    int& patchCount, MobPatchCells patchCells, ObjType objType )
{
    if ( interaction == TInteract_Load )
    {
        if ( patchCount < 16 )
        {
            patchCells[patchCount].Row = row;
            patchCells[patchCount].Col = col;
            patchCount++;
        }
    }
    else if ( interaction == TInteract_Push )
    {
        int behavior = tileMaps[curTileMapIndex].tileBehaviors[row][col];

        if ( row > 0 && tileMaps[curTileMapIndex].tileBehaviors[row - 1][col] == behavior )
            row--;
        if ( col > 0 && tileMaps[curTileMapIndex].tileBehaviors[row][col - 1] == behavior )
            col--;

        MakeActivatedObject( objType, row, col );
    }
}

void WorldImpl::MakeActivatedObject( int type, int row, int col )
{
    row += BaseRows;

    int x = col * TileWidth;
    int y = row * TileHeight;

    for ( int i = LastMonsterSlot; i >= 0; i-- )
    {
        Object* obj = objects[i];
        if ( obj == nullptr || obj->GetType() != type )
            continue;

        int objCol = obj->GetX() / TileWidth;
        int objRow = obj->GetY() / TileHeight;

        if ( objCol == col && objRow == row )
            return;
    }

    int freeSlot = FindEmptyMonsterSlot();
    if ( freeSlot >= 0 )
    {
        Object* obj = MakeMonster( (ObjType) type, x, y );
        objects[freeSlot] = obj;
        obj->SetObjectTimer( 0x40 );
    }
}

void WorldImpl::BlockTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    Block*  block = new Block();
    block->SetX( col * TileWidth );
    block->SetY( TileMapBaseY + row * TileHeight );
    SetBlockObj( block );
}

void WorldImpl::DoorTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Push )
        return;

    // Based on $91D6 and old implementation Player::CheckDoor.

    _RPT2( _CRT_WARN, "Push door: %d, %d\n", row, col );

    DoorType    doorType = World::GetDoorType( player->GetMoving() );

    switch ( doorType )
    {
    case DoorType_FalseWall:
    case DoorType_FalseWall2:
        if ( player->GetObjectTimer() == 0 )
        {
            player->SetObjectTimer( 0x18 );
        }
        else if ( player->GetObjectTimer() == 1 )
        {
            World::LeaveRoom( player->GetFacing(), World::GetRoomId() );
            player->Stop();
        }
        break;

    case DoorType_Bombable:
        if ( World::GetEffectiveDoorState( player->GetMoving() ) )
        {
            World::LeaveRoom( player->GetFacing(), World::GetRoomId() );
            player->Stop();
        }
        break;

    case DoorType_Key:
    case DoorType_Key2:
        if ( triggeredDoorDir == Dir_None )
        {
            bool canOpen = false;

            if ( World::GetItem( ItemSlot_MagicKey ) != 0 )
            {
                canOpen = true;
            }
            else if ( World::GetItem( ItemSlot_Keys ) != 0 )
            {
                canOpen = true;
                World::DecrementItem( ItemSlot_Keys );
            }

            if ( canOpen )
            {
                // $8ADA
                World::SetTriggeredDoorDir( player->GetMoving() );
                World::SetTriggeredDoorCmd( 8 );
            }
        }
        break;
    }
}


//----------------------------------------------------------------------------

World::World()
{
}

void World::Init()
{
    sWorld = new WorldImpl();
    sWorld->Init();
}

void World::Uninit()
{
    delete sWorld;
    sWorld = nullptr;
}

void World::Start( int slot, const Profile& profile )
{
    sWorld->Start( slot, profile );
}

void World::Update()
{
    sWorld->Update();
}

void World::Draw()
{
    sWorld->Draw();
}
