/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "World.h"
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

const int UWBlockRow        = 5;

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

const bool doorFaceOpen[] = 
{
    true,
    false,
    false,
    false,
    true
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


World::TileActionFunc World::sActionFuncs[] = 
{
    &World::NoneTileAction,
    &World::PushTileAction,
    &World::BombTileAction,
    &World::BurnTileAction,
    &World::HeadstoneTileAction,
    &World::LadderTileAction,
    &World::RaftTileAction,
    &World::CaveTileAction,
    &World::StairsTileAction,
    &World::GhostTileAction,
    &World::ArmosTileAction,
    &World::BlockTileAction,
};

World* World::sWorld;

Player* player;
bool    giveFakePlayerPos;
int     playerPosTimer;
Point   fakePlayerPos;

Object* objects[MaxObjects];
Object* objectsToDelete[MaxObjects];
int     objectsToDeleteCount;
int     objectTimers[MaxObjects];
int     curObjSlot;
int     longTimer;
int     stunTimers[MaxObjects];
uint8_t placeholderTypes[MaxObjects];

Direction       doorwayDir;         // 53
int             triggeredDoorCmd;   // 54
Direction       triggeredDoorDir;   // 55
int             fromUnderground;    // 5A
int             activeShots;        // 34C
bool            triggerShutters;    // 4CE
bool            summonedWhirlwind;  // 508
bool            powerTriforceFanfare;   // 509
int             recorderUsed;       // 51B
bool            candleUsed;         // 513
Direction       shuttersPassedDirs; // 519
bool            brightenRoom;       // 51E
int             profileSlot;
Profile         profile;
UWRoomFlags*    curUWBlockFlags;


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

void ClearDeadObjectQueue()
{
    for ( int i = 0; i < objectsToDeleteCount; i++ )
    {
        delete objectsToDelete[i];
        objectsToDelete[i] = nullptr;
    }

    objectsToDeleteCount = 0;
}

void SetOnlyObject( int slot, Object* obj )
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

Ladder* GetLadderObj()
{
    return (Ladder*) objects[LadderSlot];
}

void SetLadderObj( Ladder* ladder )
{
    SetOnlyObject( LadderSlot, ladder );
}

void SetBlockObj( Object* block )
{
    SetOnlyObject( BlockSlot, block );
}

void DeleteObjects()
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

void CleanUpRoomItems()
{
    DeleteObjects();
    World::SetItem( ItemSlot_Clock, 0 );
}

void DeleteDeadObjects()
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

void InitObjectTimers()
{
    for ( int i = 0; i < MaxObjects; i++ )
    {
        objectTimers[i] = 0;
    }
}

void DecrementObjectTimers()
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

void InitStunTimers()
{
    longTimer = 0;
    for ( int i = 0; i < MaxObjects; i++ )
    {
        stunTimers[i] = 0;
    }
}

void DecrementStunTimers()
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

void InitPlaceholderTypes()
{
    memset( placeholderTypes, 0, sizeof placeholderTypes );
}

int FindEmptyMonsterSlot()
{
    for ( int i = LastMonsterSlot; i >= 0; i-- )
    {
        if ( objects[i] == nullptr )
            return i;
    }
    return -1;
}

void ClearRoomItemData()
{
    recorderUsed = 0;
    candleUsed = false;
    summonedWhirlwind = false;
    shuttersPassedDirs = Dir_None;
    brightenRoom = false;
    activeShots = 0;
}

static void SetPlayerColor()
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
    const World::LevelInfoBlock* infoBlock = World::Get()->GetLevelInfo();

    for ( int i = 2; i < BackgroundPalCount; i++ )
    {
        Graphics::SetPaletteIndexed( i, infoBlock->Palettes[i] );
    }

    Graphics::UpdatePalettes();
}

static void SetLevelFgPalette()
{
    const World::LevelInfoBlock* infoBlock = World::Get()->GetLevelInfo();
    Graphics::SetPaletteIndexed( LevelFgPalette, infoBlock->Palettes[LevelFgPalette] );
}


World::World()
    :   curRoomId( 0 ),
        curTileMapIndex( 0 ),
        lastMode( Mode_Demo ),
        curUpdate( &World::UpdatePlay ),
        curDraw( &World::DrawPlay ),
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
        nextGameMenu( nullptr )
{
    sWorld = this;
}

World::~World()
{
    delete player;
    player = nullptr;

    DeleteObjects();

    sWorld = nullptr;

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

void World::LoadOpenRoomContext()
{
    colCount = 16;
    rowCount = 11;
    startRow = 0;
    startCol = 0;
    tileTypeCount = 56;
    marginRight = OWMarginRight;
    marginLeft = OWMarginLeft;
    marginBottom = OWMarginBottom;
    marginTop = OWMarginTop;
}

void World::LoadClosedRoomContext()
{
    colCount = 12;
    rowCount = 7;
    startRow = 2;
    startCol = 2;
    tileTypeCount = 9;
    marginRight = UWMarginRight;
    marginLeft = UWMarginLeft;
    marginBottom = UWMarginBottom;
    marginTop = UWMarginTop;
}

void World::LoadMapResourcesFromDirectory( int uniqueRoomCount )
{
    Util::LoadList( directory.RoomCols, roomCols, uniqueRoomCount );

    Util::LoadResource( directory.ColTables, &colTables );

    Util::LoadList( directory.TileAttrs, tileAttrs, tileTypeCount );

    Graphics::LoadTileSheet( Sheet_Background, directory.TilesImage );
}

void World::LoadOverworldContext()
{
    LoadOpenRoomContext();
    LoadMapResourcesFromDirectory( 124 );
}

void World::LoadUnderworldContext()
{
    LoadClosedRoomContext();
    LoadMapResourcesFromDirectory( 64 );
}

void World::LoadCellarContext()
{
    LoadOpenRoomContext();

    Util::LoadList( "underworldCellarRoomCols.dat", roomCols, 2 );

    Util::LoadResource( "underworldCellarCols.tab", &colTables );

    Util::LoadList( "underworldCellarTileAttrs.dat", tileAttrs, tileTypeCount );

    Graphics::LoadTileSheet( Sheet_Background, "underworldCellarTiles.png" );
}

void World::LoadLevel( int level )
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
            memset( tileMaps[i].tileRefs, Tile_Wall, sizeof tileMaps[i].tileRefs );
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

void World::Init()
{
    int sysPal[SysPaletteLength];
    Util::LoadList( "pal.dat", sysPal, SysPaletteLength );
    Graphics::LoadSystemPalette( sysPal );

    Graphics::LoadTileSheet( Sheet_Font, "font.png" );
    Graphics::LoadTileSheet( Sheet_PlayerAndItems, "playerItem.png", "playerItemsSheet.tab" );

    Util::LoadResource( "text.tab", &textTable );

    GotoFileMenu();
}

void World::Start( int slot, const Profile& profile )
{
    ::profile = profile;
    ::profile.Hearts = Profile::GetMaxHeartsValue( DefaultHearts );
    profileSlot = slot;

    GotoLoadLevel( 0, true );
}

void World::Update()
{
    GameMode mode = GetMode();

    if ( lastMode != mode )
    {
        if ( IsPlaying( lastMode ) && mode != Mode_WinGame )
        {
            CleanUpRoomItems();
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

    (this->*curUpdate)();
}

void World::Draw()
{
    if ( statusBarVisible )
        statusBar.Draw( submenuOffsetY );
    (this->*curDraw)();
}

void World::DrawRoom()
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
    return sWorld->curUpdate == &UpdatePlay;
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
    return (curUpdate == &World::UpdatePlayCave)
        || (curUpdate == &World::UpdatePlay && state.play.roomType == PlayState::Cave);
}

GameMode World::GetMode()
{
    // TODO: at least store the mode in a field when you change mode.
    // TODO: long term, do it more like the original game.

    if ( sWorld->curUpdate == &World::UpdatePlayCellar )
        return Mode_PlayCellar;
    if ( sWorld->curUpdate == &World::UpdatePlayCave )
        return Mode_PlayCave;
    if ( sWorld->curUpdate == &World::UpdatePlay )
    {
        switch ( sWorld->state.play.roomType )
        {
        case PlayState::Regular:    return Mode_Play;
        case PlayState::Cellar:     return Mode_PlayCellar;
        case PlayState::Cave:       return Mode_PlayCave;
        }
    }
    if ( sWorld->curUpdate == &World::UpdateStairsState )
        return Mode_Stairs;
    if ( sWorld->curUpdate == &World::UpdateLeave )
        return Mode_Leave;
    if ( sWorld->curUpdate == &World::UpdateLeaveCellar )
        return Mode_LeaveCellar;
    if ( sWorld->curUpdate == &World::UpdateEnter )
        return Mode_Enter;
    if ( sWorld->curUpdate == &World::UpdateLoadLevel )
        return Mode_LoadLevel;
    if ( sWorld->curUpdate == &World::UpdateScroll )
        return Mode_Scroll;
    if ( sWorld->curUpdate == &World::UpdateUnfurl )
        return Mode_Unfurl;
    if ( sWorld->curUpdate == &World::UpdateWinGame )
        return Mode_WinGame;
    if ( sWorld->curUpdate == &World::UpdateDie )
        return Mode_Death;
    if ( sWorld->curUpdate == &World::UpdateEndLevel )
        return Mode_EndLevel;
    if ( sWorld->curUpdate == &World::UpdateGameMenu )
        return Mode_GameMenu;
    if ( sWorld->curUpdate == &World::UpdateRegisterMenu )
        return Mode_Register;
    if ( sWorld->curUpdate == &World::UpdateEliminateMenu )
        return Mode_Elimination;
    if ( sWorld->curUpdate == &World::UpdateContinueQuestion )
        return Mode_ContinueQuestion;

    assert( false );
    return Mode_Demo;
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
    return player;
}

Point World::GetObservedPlayerPos()
{
    return fakePlayerPos;
}

Ladder* World::GetLadder()
{
    return GetLadderObj();
}

void World::SetLadder( Ladder* ladder )
{
    SetLadderObj( ladder );
}

void World::UseRecorder()
{
    sWorld->UseRecorderImpl();
}

void World::UseRecorderImpl()
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

void World::SummonWhirlwind()
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

void World::MakeFluteSecret()
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
    return recorderUsed;
}

void World::SetRecorderUsed( int value )
{
    recorderUsed = value;
}

bool World::GetCandleUsed()
{
    return candleUsed;
}

void World::SetCandleUsed()
{
    candleUsed = true;
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

int World::GetMapTile( int row, int col )
{
    return tileMaps[curTileMapIndex].tileRefs[row][col];
}

void World::SetTile( int x, int y, int tileType )
{
    sWorld->SetTileImpl( x, y, tileType );
}

void World::SetTileImpl( int x, int y, int tileType )
{
    int col = x / TileWidth;
    int row = (y - TileMapBaseY) / TileHeight;

    if ( col < 0 || col >= Columns || row < 0 || row >= Rows )
        return;

    tileMaps[curTileMapIndex].tileRefs[row][col] = tileType;
}

int World::GetInnerPalette()
{
    return sWorld->GetInnerPaletteImpl();
}

int World::GetInnerPaletteImpl()
{
    return roomAttrs[curRoomId].GetInnerPalette();
}

Point World::GetRandomWaterTile()
{
    return sWorld->GetRandomWaterTileImpl();
}

Point World::GetRandomWaterTileImpl()
{
    uint8_t waterList[Rows * Columns];
    int waterCount = 0;

    for ( int r = 0; r < Rows; r++ )
    {
        for ( int c = 0; c < Columns; c++ )
        {
            int tileRef = tileMaps[curTileMapIndex].tileRefs[r][c];
            uint8_t attr = tileAttrs[tileRef];
            int action = TileAttr::GetAction( attr );

            if ( action == TileAction_Ladder )
            {
                waterList[waterCount] = (r << 4) | c;
                waterCount++;
            }
        }
    }

    assert( waterCount > 0 );

    int r = Util::GetRandom( waterCount );
    uint8_t pos = waterList[r];
    Point p = { pos & 0xF, (pos >> 4) + BaseRows };
    return p;
}

Object* World::GetObject( int slot )
{
    if ( slot == PlayerSlot )
        return player;
    return objects[slot];
}

void World::SetObject( int slot, Object* obj )
{
    SetOnlyObject( slot, obj );
}

int World::FindEmptyMonsterSlot()
{
    return ::FindEmptyMonsterSlot();
}

int World::FindEmptyFireSlot()
{
    for ( int i = FirstFireSlot; i < LastFireSlot; i++ )
    {
        if ( objects[i] == nullptr )
            return i;
    }
    return -1;
}

int World::GetCurrentObjectSlot()
{
    return curObjSlot;
}

void World::SetCurrentObjectSlot( int slot )
{
    curObjSlot = slot;
}

int& World::GetObjectTimer( int slot )
{
    return objectTimers[slot];
}

void World::SetObjectTimer( int slot, int value )
{
    objectTimers[slot] = value;
}

int  World::GetStunTimer( int slot )
{
    return stunTimers[slot];
}

void World::SetStunTimer( int slot, int value )
{
    stunTimers[slot] = value;
}

void World::PushTile( int row, int col )
{
    sWorld->InteractTile( row, col, TInteract_Push );
}

void World::TouchTile( int row, int col )
{
    sWorld->InteractTile( row, col, TInteract_Touch );
}

void World::CoverTile( int row, int col )
{
    sWorld->InteractTile( row, col, TInteract_Cover );
}

void World::InteractTile( int row, int col, TileInteraction interaction )
{
    if ( row < 0 || col < 0 || row >= Rows * 2 || col >= Columns * 2 )
        return;

    int coarseRow = row / 2;
    int coarseCol = col / 2;
    uint8_t t = tileMaps[curTileMapIndex].tileRefs[coarseRow][coarseCol];
    uint8_t attr = tileAttrs[t];
    int action = TileAttr::GetAction( attr );

    TileActionFunc actionFunc = sActionFuncs[action];
    (this->*actionFunc)( coarseRow, coarseCol, interaction );
}

bool World::CollidesTile( int row, int col )
{
    int tileRef = tileMaps[curTileMapIndex].tileRefs[row][col];
    uint8_t attr = tileAttrs[tileRef];
    return TileAttr::IsTileBlocked( attr );
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

TileCollision World::CollidesWithTile( 
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

    bool firstTile = true;
    uint8_t ref;
    bool collides = false;
    int fineRow = (y - TileMapBaseY) / 8;
    int coarseRow = fineRow / 2;
    int quadRow = fineRow % 2;

    int fineCol1 = x / 8;
    int fineCol2;
    int hitFineCol = fineCol1;

    if ( Util::IsVertical( dir ) )
        fineCol2 = (x + 8) / 8;
    else
        fineCol2 = x / 8;

    if ( !IsOverworld() && CollidesWithUWBorder( fineRow, fineCol1, fineCol2 ) )
    {
        // If you hit the border, then I don't think it's important to know the exact fineCol.
        TileCollision collision = { true, Tile_Wall, fineCol1, fineRow };
        return collision;
    }

    for ( int c = fineCol1; c <= fineCol2; c++ )
    {
        int coarseCol = c / 2;
        int quadCol = c % 2;
        int tileRef = GetMapTile( coarseRow, coarseCol );
        uint8_t attr = tileAttrs[tileRef];

        if ( firstTile )
        {
            ref = tileRef;
            firstTile = false;
        }

        if ( TileAttr::IsQuadrantBlocked( attr, quadRow, quadCol ) )
        {
            int action = TileAttr::GetAction( attr );
            if ( action == TileAction_Ladder && state.play.allowWalkOnWater )
                continue;
            if ( action != 0 )
            {
                TileCollision collision = { true, tileRef, c, fineRow };
                return collision;
            }
            collides = true;
            ref = tileRef;
            hitFineCol = c;
        }
    }

    TileCollision collision = { collides, ref, hitFineCol, fineRow };
    return collision;
}

bool World::CollidesWithUWBorder( int fineRow, int fineCol1, int fineCol2 )
{
    if ( (fineRow >= startRow * 2) && (fineRow  < (startRow + rowCount) * 2) 
        && (fineCol1 >= startCol * 2) && (fineCol2 < (startCol + colCount) * 2) )
        return false;

    // Coarse checks for collision are done here. All of the border's treated the same.
    // Finer checks for collision are done in Player.

    return true;
}

void World::OnPushedBlock()
{
    sWorld->OnPushedBlockImpl();
}

void World::OnPushedBlockImpl()
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
    sWorld->OnActivatedArmosImpl( x, y );
}

void World::OnActivatedArmosImpl( int x, int y )
{
    const SparsePos2*   pos = FindSparsePos2( Sparse_ArmosStairs, curRoomId );

    if ( pos != nullptr && x == pos->x && y == pos->y )
    {
        SetTileImpl( x, y, Tile_Stairs );
        Sound::PlayEffect( SEffect_secret );
    }
    else
    {
        SetTileImpl( x, y, Tile_Ground );
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
    powerTriforceFanfare = true;
    player->SetState( Player::Paused );
    player->SetObjectTimer( 0xC0 );

    const static uint8_t palette[] = { 0, 0x0F, 0x10, 0x30 };
    Graphics::SetPaletteIndexed( LevelFgPalette, palette );
    Graphics::UpdatePalettes();
}

void World::CheckPowerTriforceFanfare()
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

void World::AdjustInventory()
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

void World::WarnLowHPIfNeeded()
{
    if ( profile.Hearts >= 0x100 )
        return;

    Sound::PlayEffect( SEffect_low_hp );
}

void World::PlayAmbientSounds()
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

void World::ShowShortcutStairs( int roomId, int tileMapIndex )
{
    OWRoomAttrs& owRoomAttrs = (OWRoomAttrs&) roomAttrs[roomId];
    int index = owRoomAttrs.GetShortcutStairsIndex();
    int pos = infoBlock.ShortcutPosition[index];
    int row;
    int col;
    GetRoomCoord( pos, row, col );
    tileMaps[tileMapIndex].tileRefs[row][col] = Tile_Stairs;
}

void World::DrawMap( int roomId, int mapIndex, int offsetX, int offsetY )
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

            if ( r < 2 || r >= 9 || c < 2 || c >= 14 )
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

void World::DrawDoors( int roomId, bool above, int offsetX, int offsetY )
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

World* World::Get()
{
    return sWorld;
}

Profile& World::GetProfile()
{
    return profile;
}

uint8_t World::GetItem( int itemSlot )
{
    return profile.Items[itemSlot];
}

void World::SetItem( int itemSlot, int value )
{
    profile.Items[itemSlot] = value;
}

static void PostRupeeChange( uint8_t value, int itemSlot )
{
    uint8_t curValue = profile.Items[itemSlot];
    uint8_t newValue = curValue + value;

    if ( newValue < curValue )
        newValue = 255;

    profile.Items[itemSlot] = newValue;
}

void World::PostRupeeWin( uint8_t value )
{
    PostRupeeChange( value, ItemSlot_RupeesToAdd );
}

void World::PostRupeeLoss( uint8_t value )
{
    PostRupeeChange( value, ItemSlot_RupeesToSubtract );
}

void World::FillHearts( int heartValue )
{
    unsigned int maxHeartValue = profile.Items[ItemSlot_HeartContainers] << 8;

    profile.Hearts += heartValue;

    if ( profile.Hearts >= maxHeartValue )
        profile.Hearts = maxHeartValue - 1;
}

void World::AddItem( int itemId )
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

bool World::GetDoorState( int door )
{
    // TODO: the original game does it a little different, by looking at $EE.
    return sWorld->GetDoorState( sWorld->curRoomId, door )
        || (sWorld->GetDoorType( (Direction) door ) == DoorType_Shutter 
            && sWorld->tempShutters && sWorld->curRoomId == sWorld->tempShuttersRoomId)
        || (sWorld->tempShutterDoorDir == door && sWorld->curRoomId == sWorld->tempShutterRoomId);
}

UWRoomFlags& World::GetUWRoomFlags( int curRoomId )
{
    return curUWBlockFlags[curRoomId];
}

const World::LevelInfoBlock* World::GetLevelInfo()
{
    return &infoBlock;
}

bool World::IsOverworld()
{
    return sWorld->infoBlock.LevelNumber == 0;
}

bool World::DoesRoomSupportLadder()
{
    return sWorld->FindSparseFlag( Sparse_Ladder, sWorld->GetRoomId() );
}

int World::GetTileAction( int tileRef )
{
    uint8_t attr = sWorld->tileAttrs[tileRef];
    return TileAttr::GetAction( attr );
}

bool World::IsUWMain( int roomId )
{
    return !IsOverworld() && (roomAttrs[roomId].GetUniqueRoomId() < 0x3E);
}

bool World::IsUWCellar( int roomId )
{
    return !IsOverworld() && (roomAttrs[roomId].GetUniqueRoomId() >= 0x3E);
}

bool World::IsUWCellar()
{
    return sWorld->IsUWCellar( sWorld->GetRoomId() );
}

bool World::GotShortcut( int roomId )
{
    return profile.OverworldFlags[roomId].GetShortcutState();
}

bool World::GotSecret()
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

void World::TakeShortcut()
{
    profile.OverworldFlags[curRoomId].SetShortcutState();
}

void World::TakeSecret()
{
    profile.OverworldFlags[curRoomId].SetSecretState();
}

bool World::GotItem()
{
    return GotItem( curRoomId );
}

bool World::GotItem( int roomId )
{
    if ( IsOverworld() )
    {
        return profile.OverworldFlags[roomId].GetItemState();
    }
    else
    {
        return curUWBlockFlags[roomId].GetItemState();
    }
}

void World::MarkItem()
{
    if ( IsOverworld() )
    {
        profile.OverworldFlags[curRoomId].SetItemState();
    }
    else
    {
        curUWBlockFlags[curRoomId].SetItemState();
    }
}

void World::LiftItem( int itemId, uint16_t timer )
{
    if ( !IsPlaying() )
        return;

    if ( itemId == Item_None || itemId == 0 )
    {
        sWorld->state.play.liftItemTimer = 0;
        sWorld->state.play.liftItemId = 0;
        return;
    }

    sWorld->state.play.liftItemTimer = timer;
    sWorld->state.play.liftItemId = itemId;

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
    tempShutters = true;
    tempShuttersRoomId = curRoomId;
    Sound::PlayEffect( SEffect_door );
}

void World::IncrementKilledObjectCount( bool allowBombDrop )
{
    worldKillCount++;

    if ( helpDropCounter < 0xA )
    {
        helpDropCounter++;
        if ( helpDropCounter == 0xA )
        {
            if ( allowBombDrop )
                helpDropValue++;
        }
    }
}

// $7B67
void World::ResetKilledObjectCount()
{
    worldKillCount = 0;
    helpDropCounter = 0;
    helpDropValue = 0;
}

void World::IncrementRoomKillCount()
{
    roomKillCount++;
}

void World::SetBombItemDrop()
{
    helpDropCounter = 0xA;
    helpDropValue = 0xA;
}

int World::GetRoomObjCount()
{
    return roomObjCount;
}

void World::SetRoomObjCount( int value )
{
    roomObjCount = value;
}

int  World::GetRoomObjId()
{
    return roomObjId;
}

void World::SetObservedPlayerPos( int x, int y )
{
    fakePlayerPos.X = x;
    fakePlayerPos.Y = y;
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
        brightenRoom = true;
}

void World::FadeIn()
{
    sWorld->FadeInImpl();
}

void World::FadeInImpl()
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

bool World::UseKey()
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

bool World::GetDoorState( int roomId, int door )
{
    return curUWBlockFlags[roomId].GetDoorState( door );
}

void World::SetDoorState( int roomId, int door )
{
    curUWBlockFlags[roomId].SetDoorState( door );
}

bool World::IsRoomInHistory()
{
    for ( int i = 0; i < RoomHistoryLength; i++ )
    {
        if ( roomHistory[i] == curRoomId )
            return true;
    }
    return false;
}

void World::AddRoomToHistory()
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

bool World::FindSparseFlag( int attrId, int roomId )
{
    return nullptr != Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const World::SparsePos* World::FindSparsePos( int attrId, int roomId )
{
    return (SparsePos*) Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const World::SparsePos2* World::FindSparsePos2( int attrId, int roomId )
{
    return (SparsePos2*) Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const SparseRoomItem* World::FindSparseItem( int attrId, int roomId )
{
    return (SparseRoomItem*) Util::FindSparseAttr( sparseRoomAttrs, attrId, roomId );
}

const ObjectAttr* World::GetObjectAttrs()
{
    return (ObjectAttr*) extraData.GetItem( Extra_ObjAttrs );
}

ObjectAttr World::GetObjectAttrs( int type )
{
    const ObjectAttr* objAttrs = GetObjectAttrs();
    return objAttrs[type];
}

int World::GetObjectMaxHP( int type )
{
    const HPAttr* hpAttrs = (HPAttr*) extraData.GetItem( Extra_HitPoints );
    int index = type / 2;
    return hpAttrs[index].GetHP( type );
}

int World::GetPlayerDamage( int type )
{
    const uint8_t* damageAttrs = (uint8_t*) extraData.GetItem( Extra_PlayerDamage );
    const uint8_t damageByte = damageAttrs[type];
    return ((damageByte & 0xF) << 8) | (damageByte & 0xF0);
}

void World::LoadRoom( int roomId, int tileMapIndex )
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

void World::AddUWRoomItem( int roomId )
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
    return doorwayDir;
}

void World::SetDoorwayDir( Direction dir )
{
    doorwayDir = dir;
}

int  World::GetFromUnderground()
{
    return fromUnderground;
}

void World::SetFromUnderground( int value )
{
    fromUnderground = value;
}

int  World::GetActiveShots()
{
    return activeShots;
}

void World::SetActiveShots( int count )
{
    activeShots = count;
}

Direction World::GetShuttersPassedDirs()
{
    return shuttersPassedDirs;
}

void World::SetShuttersPassedDirs( Direction dir )
{
    shuttersPassedDirs = dir;
}

int  World::GetTriggeredDoorCmd()
{
    return triggeredDoorCmd;
}

void World::SetTriggeredDoorCmd( int value )
{
    triggeredDoorCmd = value;
}

Direction World::GetTriggeredDoorDir()
{
    return triggeredDoorDir;
}

void World::SetTriggeredDoorDir( Direction dir )
{
    triggeredDoorDir = dir;
}

void World::LoadCaveRoom( int uniqueRoomId )
{
    curTileMapIndex = 0;

    LoadLayout( uniqueRoomId, 0, true );
}

void World::LoadMap( int roomId, int tileMapIndex )
{
    bool        owTileFormat = false;
    int         uniqueRoomId = roomAttrs[roomId].GetUniqueRoomId();

    if ( IsOverworld() )
    {
        owTileFormat = true;
    }
    else if ( uniqueRoomId >= 0x3E )
    {
        owTileFormat = true;
        uniqueRoomId -= 0x3E;
    }

    LoadLayout( uniqueRoomId, tileMapIndex, owTileFormat );
}

void World::LoadLayout( int uniqueRoomId, int tileMapIndex, bool owTileFormat )
{
    const int MaxColumnStartOffset = (colCount - 1) * rowCount;

    RoomCols*   columns = &roomCols[uniqueRoomId];
    TileMap*    map = &tileMaps[tileMapIndex];
    int         actionFound = 0;
    int         actionRow = 0;
    int         actionCol = 0;
    int         rowEnd = startRow + rowCount;

    for ( int i = 0; i < colCount; i++ )
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

        int c = startCol + i;

        for ( int r = startRow; r < rowEnd; j++ )
        {
            uint8_t t = table[j];
            uint8_t tileRef;

            if ( owTileFormat )
                tileRef = t & 0x3F;
            else
                tileRef = t & 0x7;

            uint8_t attr = tileAttrs[tileRef];
            int action = TileAttr::GetAction( attr );

            if ( action != 0 && action <= LoadingTileActions )
            {
                actionFound = action;
                actionRow = r;
                actionCol = c;
            }

            map->tileRefs[r++][c] = tileRef;

            if ( owTileFormat )
            {
                if ( (t & 0x40) != 0 && r < rowEnd )
                    map->tileRefs[r++][c] = tileRef;
            }
            else
            {
                int repeat = (t >> 4) & 0x7;
                for ( int m = 0; m < repeat && r < rowEnd; m++ )
                {
                    map->tileRefs[r++][c] = tileRef;
                }
            }
        }
    }

    if ( IsUWMain( curRoomId ) )
    {
        UWRoomAttrs& uwRoomAttrs = (UWRoomAttrs&) roomAttrs[curRoomId];
        if ( uwRoomAttrs.HasBlock() )
        {
            for ( int c = startCol; c < startCol + colCount; c++ )
            {
                uint8_t tileRef = tileMaps[curTileMapIndex].tileRefs[UWBlockRow][c];
                if ( tileRef == Tile_Block )
                {
                    actionFound = TileAction_Block;
                    actionCol = c;
                    actionRow = UWBlockRow;
                    break;
                }
            }
        }
    }

    // Only 1 secret is allowed in each room. So, handle it after loading all the tiles.
    if ( actionFound != 0 )
    {
        TileActionFunc actionFunc = sActionFuncs[actionFound];
        (this->*actionFunc)( actionRow, actionCol, TInteract_Load );
    }
}

void World::GotoPlay( PlayState::RoomType roomType )
{
    curUpdate = &World::UpdatePlay;
    curDraw = &World::DrawPlay;
    curColorSeqNum = 0;
    tempShutters = false;
    roomObjCount = 0;
    roomObjId = 0;
    roomKillCount = 0;
    roomAllDead = false;
    madeRoomItem = false;
    enablePersonFireballs = false;

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

void World::UpdatePlay()
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
            UpdateSubmenu();
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

void World::UpdateSubmenu()
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

void World::CheckShutters()
{
    if ( triggerShutters )
    {
        triggerShutters = false;

        int dirs = 0;

        for ( int i = 0; i < 4; i++ )
        {
            Direction dir = Util::GetOrdDirection( i );

            if ( GetDoorType( dir ) == DoorType_Shutter 
                && !GetDoorState( dir ) )
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

void World::UpdateDoors2()
{
    if ( GetMode() == Mode_EndLevel
        || objectTimers[DoorSlot] != 0
        || triggeredDoorCmd == 0 )
        return;

    if ( (triggeredDoorCmd & 1) == 0 )
    {
        if ( triggeredDoorCmd == 2 )
            player->SetObjectTimer( 0x30 );

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
                }
            }
        }

        triggeredDoorCmd = 0;
        triggeredDoorDir = Dir_None;
    }
}

int World::GetNextTeleportingRoomIndex()
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

void World::UpdateRoomColors()
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
            tileMaps[curTileMapIndex].tileRefs[row][col] = Tile_Stairs;
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

void World::CheckBombables()
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

bool World::CalcHasLivingObjects()
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

void World::CheckSecrets()
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

void World::AddUWRoomStairs()
{
    SetTileImpl( 0xD0, 0x60, Tile_UW_Stairs );
}

void World::KillAllObjects()
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

void World::MoveRoomItem()
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

void World::UpdateStatues()
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

void World::OnLeavePlay()
{
    if ( lastMode == Mode_Play )
        SaveObjectCount();
}

void World::ClearLevelData()
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

void World::SaveObjectCount()
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

void World::CalcObjCountToMake( int& objId, int& count )
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

void World::UpdateObservedPlayerPos()
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

void World::UpdateRupees()
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

void World::UpdateLiftItem()
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

void World::DrawPlay()
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

    DrawObjects();

    if ( IsLiftingItem() )
        DrawLinkLiftingItem( state.play.liftItemId );
    else
        player->Draw();

    if ( IsUWMain( curRoomId ) )
        DrawDoors( curRoomId, true, 0, 0 );
}

void World::DrawSubmenu()
{
    Graphics::SetClip( 0, TileMapBaseY + submenuOffsetY, TileMapWidth, TileMapHeight - submenuOffsetY );
    ClearScreen();
    DrawMap( curRoomId, curTileMapIndex, 0, submenuOffsetY );
    Graphics::ResetClip();

    if ( IsUWMain( curRoomId ) )
        DrawDoors( curRoomId, true, 0, submenuOffsetY );

    menu.Draw( submenuOffsetY );
}

void World::DrawObjects()
{
    for ( int i = 0; i < MaxObjects; i++ )
    {
        curObjSlot = i;

        Object* obj = objects[i];
        if ( obj != nullptr && !obj->IsDeleted() )
            obj->DecoratedDraw();
    }
}

void DrawZeldaLiftingTriforce( int x, int y )
{
    SpriteImage image;
    image.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B3_Zelda_Lift );
    image.Draw( Sheet_Boss, x, y, PlayerPalette );

    DrawItem( Item_TriforcePiece, x, y - 0x10, 0 );
}

void World::DrawLinkLiftingItem( int itemId )
{
    int animIndex = (itemId == Item_TriforcePiece) ? Anim_PI_LinkLiftHeavy : Anim_PI_LinkLiftLight;
    SpriteImage image;
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, animIndex );
    image.Draw( Sheet_PlayerAndItems, player->GetX(), player->GetY(), PlayerPalette );

    DrawItem( itemId, player->GetX(), player->GetY() - 0x10, 0 );
}

void World::MakeObjects( Direction entryDir )
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

void World::MakeCellarObjects()
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

void World::MakeCaveObjects()
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

void World::MakeUnderworldPerson( int objId )
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

void World::MakePersonRoomObjects( int type, const CaveSpec* spec )
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

void World::MakeWhirlwind()
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

bool World::FindSpawnPos( int type, const RSpot* spots, int len, int& x, int& y )
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

void World::PutEdgeObject()
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

        int row = (y / 16) - 4;
        int col = (x / 16);
        int tileRef = GetMapTile( row, col );

        if ( (tileRef != Tile_Sand) && !CollidesTile( row, col ) )
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

void World::HandleNormalObjectDeath()
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

void World::TryDroppingItem( int origType, int x, int y )
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

void World::FillHeartsStep()
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

void World::GotoScroll( Direction dir )
{
    assert( dir != Dir_None );

    state.scroll.curRoomId = curRoomId;
    state.scroll.scrollDir = dir;
    state.scroll.substate = ScrollState::Start;
    curUpdate = &World::UpdateScroll;
    curDraw = &World::DrawScroll;
}

void World::GotoScroll( Direction dir, int currentRoomId )
{
    GotoScroll( dir );
    state.scroll.curRoomId = currentRoomId;
}

bool World::CalcMazeStayPut( Direction dir )
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

void World::UpdateScroll()
{
    if ( state.scroll.substate == ScrollState::Start )
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
    else if ( state.scroll.substate == ScrollState::Scroll )
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
    else if ( state.scroll.substate == ScrollState::AnimatingColors )
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
    else if ( state.scroll.substate == ScrollState::FadeOut )
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
    else if ( state.scroll.substate == ScrollState::LoadRoom )
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
}

void World::DrawScroll()
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

void World::GotoLeave( Direction dir )
{
    assert( dir != Dir_None );

    state.leave.curRoomId = curRoomId;
    state.leave.scrollDir = dir;
    state.leave.timer = LeaveState::StateTime;
    curUpdate = &World::UpdateLeave;
    curDraw = &World::DrawLeave;
}

void World::GotoLeave( Direction dir, int currentRoomId )
{
    GotoLeave( dir );
    state.leave.curRoomId = currentRoomId;
}

void World::UpdateLeave()
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

void World::DrawLeave()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    DrawRoomNoObjects();
    Graphics::ResetClip();
}

void World::GotoEnter( Direction dir )
{
    state.enter.substate = EnterState::Start;
    state.enter.scrollDir = dir;
    state.enter.timer = 0;
    state.enter.playerPriority = SpritePri_AboveBg;
    state.enter.playerSpeed = Player::WalkSpeed;
    curUpdate = &World::UpdateEnter;
    curDraw = &World::DrawEnter;
}

void MovePlayer( Direction dir, int speed, int& fraction )
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

void World::UpdateEnter()
{
    bool play = false;

    if ( state.enter.substate == EnterState::Start )
    {
        triggeredDoorCmd = 0;
        triggeredDoorDir = Dir_None;

        if ( IsOverworld() )
        {
            int row = (player->GetY() + 3 - TileMapBaseY) / TileHeight;
            int col = player->GetX() / TileWidth;

            uint8_t tileRef = tileMaps[curTileMapIndex].tileRefs[row][col];
            if ( tileRef == Tile_Cave )
            {
                player->SetY( player->GetY() + TileHeight );
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
                distance = TileWidth * 2;
            else
                distance = TileWidth;

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
        {
            doorwayDir = state.enter.scrollDir;
            triggeredDoorDir = Util::GetOppositeDir( doorwayDir );
            if ( GetDoorState( triggeredDoorDir ) )
            {
                triggeredDoorCmd = 2;
            }
            else
                triggeredDoorDir = Dir_None;
        }
        else
            doorwayDir = Dir_None;
    }
    else if ( state.enter.substate == EnterState::Wait )
    {
        state.enter.timer--;
        if ( state.enter.timer == 0 )
            play = true;
    }
    else if ( state.enter.substate == EnterState::FadeIn )
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
    else if ( state.enter.substate == EnterState::Walk )
    {
        if (   player->GetX() == state.enter.targetX 
            && player->GetY() == state.enter.targetY )
            play = true;
        else
            player->MoveLinear( state.enter.scrollDir, state.enter.playerSpeed );
    }
    else if ( state.enter.substate == EnterState::WalkCave )
    {
        if (   player->GetX() == state.enter.targetX 
            && player->GetY() == state.enter.targetY )
            play = true;
        else
            MovePlayer( state.enter.scrollDir, state.enter.playerSpeed, state.enter.playerFraction );
    }

    if ( play )
    {
        if ( IsUWMain( curRoomId ) 
            && (tempShutterDoorDir != Dir_None) 
            && GetDoorType( curRoomId, (Direction) tempShutterDoorDir ) == DoorType_Shutter )
        {
            Sound::PlayEffect( SEffect_door );
        }
        tempShutterDoorDir = Dir_None;
        statusBar.EnableFeatures( StatusBar::Feature_All, true );
        if ( IsOverworld() && fromUnderground != 0 )
            Sound::PlaySong( infoBlock.Song, Sound::MainSongStream, true );
        GotoPlay();
        return;
    }
    player->GetAnimator()->Advance();
}

void World::DrawEnter()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );

    if ( state.enter.substate != EnterState::Start )
        DrawRoomNoObjects( state.enter.playerPriority );

    Graphics::ResetClip();
}

void World::GotoLoadLevel( int level, bool restartOW )
{
    state.loadLevel.level = level;
    state.loadLevel.substate = LoadLevelState::Load;
    state.loadLevel.timer = 0;
    state.loadLevel.restartOW = restartOW;

    curUpdate = &World::UpdateLoadLevel;
    curDraw = &World::DrawLoadLevel;
}

void World::SetPlayerExitPosOW( int roomId )
{
    int             row, col;
    OWRoomAttrs&    owRoomAttrs = (OWRoomAttrs&) roomAttrs[roomId];
    RSpot           exitRPos = owRoomAttrs.GetExitPosition();

    col = exitRPos & 0xF;
    row = (exitRPos >> 4) + 4;

    player->SetX( col * TileWidth );
    player->SetY( row * TileHeight + 0xD );
}

const uint8_t* World::GetString( int stringId )
{
    return sWorld->GetStringImpl( stringId );
}

const uint8_t* World::GetStringImpl( int stringId )
{
    return textTable.GetItem( stringId );
}

void World::UpdateLoadLevel()
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

void World::DrawLoadLevel()
{
    Graphics::SetClip( 0, 0, StdViewWidth, StdViewHeight );
    ClearScreen();
    Graphics::ResetClip();
}

void World::GotoUnfurl( bool restartOW )
{
    state.unfurl.substate = UnfurlState::Start;
    state.unfurl.timer = UnfurlState::StateTime;
    state.unfurl.stepTimer = 0;
    state.unfurl.left = 0x80;
    state.unfurl.right = 0x80;
    state.unfurl.restartOW = restartOW;

    ClearLevelData();

    curUpdate = &World::UpdateUnfurl;
    curDraw = &World::DrawUnfurl;
}

void World::UpdateUnfurl()
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

void World::DrawUnfurl()
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

void World::GotoEndLevel()
{
    state.endLevel.substate = EndLevelState::Start;
    curUpdate = &World::UpdateEndLevel;
    curDraw = &World::DrawEndLevel;
}

void World::UpdateEndLevel()
{
    if ( state.endLevel.substate == EndLevelState::Start )
    {
        state.endLevel.substate = EndLevelState::Wait1;
        state.endLevel.timer = EndLevelState::Wait1Time;

        state.endLevel.left = 0;
        state.endLevel.right = World::TileMapWidth;
        state.endLevel.stepTimer = 4;

        statusBar.EnableFeatures( StatusBar::Feature_Equipment, false );
        Sound::PlaySong( Song_triforce, Sound::MainSongStream, false );
    }
    else if (  state.endLevel.substate == EndLevelState::Wait1
            || state.endLevel.substate == EndLevelState::Wait2
            || state.endLevel.substate == EndLevelState::Wait3 )
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
    else if ( state.endLevel.substate == EndLevelState::Flash )
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
    else if ( state.endLevel.substate == EndLevelState::FillHearts )
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
    else if ( state.endLevel.substate == EndLevelState::Furl )
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
}

void World::DrawEndLevel()
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

void World::GotoWinGame()
{
    state.winGame.substate = WinGameState::Start;
    state.winGame.timer = 162;
    state.winGame.left = 0;
    state.winGame.right = World::TileMapWidth;
    state.winGame.stepTimer = 0;
    state.winGame.npcVisual = WinGameState::Npc_Stand;

    curUpdate = &World::UpdateWinGame;
    curDraw = &World::DrawWinGame;
}

void World::UpdateWinGame()
{
    if ( state.winGame.substate == WinGameState::Start )
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
    else if ( state.winGame.substate == WinGameState::Text1 )
    {
        textBox1->Update();
        if ( textBox1->IsDone() )
        {
            state.winGame.substate = WinGameState::Stand;
            state.winGame.timer = 76;
        }
    }
    else if ( state.winGame.substate == WinGameState::Stand )
    {
        state.winGame.timer--;
        if ( state.winGame.timer == 0 )
        {
            state.winGame.substate = WinGameState::Hold1;
            state.winGame.timer = 64;
        }
    }
    else if ( state.winGame.substate == WinGameState::Hold1 )
    {
        state.winGame.npcVisual = WinGameState::Npc_Lift;
        state.winGame.timer--;
        if ( state.winGame.timer == 0 )
        {
            state.winGame.substate = WinGameState::Colors;
            state.winGame.timer = 127;
        }
    }
    else if ( state.winGame.substate == WinGameState::Colors )
    {
        state.winGame.timer--;
        if ( state.winGame.timer == 0 )
        {
            state.winGame.substate = WinGameState::Hold2;
            state.winGame.timer = 131;
            Sound::PlaySong( Song_ending, Sound::MainSongStream, true );
        }
    }
    else if ( state.winGame.substate == WinGameState::Hold2 )
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
    else if ( state.winGame.substate == WinGameState::Text2 )
    {
        textBox2->Update();
        if ( textBox2->IsDone() )
        {
            state.winGame.substate = WinGameState::Hold3;
            state.winGame.timer = 129;
        }
    }
    else if ( state.winGame.substate == WinGameState::Hold3 )
    {
        state.winGame.timer--;
        if ( state.winGame.timer == 0 )
        {
            state.winGame.substate = WinGameState::NoObjects;
            state.winGame.timer = 32;
        }
    }
    else if ( state.winGame.substate == WinGameState::NoObjects )
    {
        state.winGame.npcVisual = WinGameState::Npc_None;
        state.winGame.timer--;
        if ( state.winGame.timer == 0 )
        {
            credits = new Credits();
            state.winGame.substate = WinGameState::Credits;
        }
    }
    else if ( state.winGame.substate == WinGameState::Credits )
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
}

void World::DrawWinGame()
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

void World::GotoStairs( int tileRef )
{
    state.stairs.substate = StairsState::Start;
    state.stairs.tileRef = tileRef;
    state.stairs.playerPriority = SpritePri_AboveBg;

    curUpdate = &World::UpdateStairsState;
    curDraw = &World::DrawStairsState;
}

void World::UpdateStairsState()
{
    if ( state.stairs.substate == StairsState::Start )
    {
        state.stairs.playerPriority = SpritePri_BelowBg;

        if ( IsOverworld() )
            Sound::StopAll();

        if ( state.stairs.tileRef == Tile_Cave )
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

void World::DrawStairsState()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    DrawRoomNoObjects( state.stairs.playerPriority );
    Graphics::ResetClip();
}

void World::GotoPlayCellar()
{
    state.playCellar.substate = PlayCellarState::Start;
    state.playCellar.playerPriority = SpritePri_None;

    curUpdate = &World::UpdatePlayCellar;
    curDraw = &World::DrawPlayCellar;
}

void World::UpdatePlayCellar()
{
    if ( state.playCellar.substate == PlayCellarState::Start )
    {
        state.playCellar.substate = PlayCellarState::FadeOut;
        state.playCellar.fadeTimer = 11;
        state.playCellar.fadeStep = 0;
    }
    else if ( state.playCellar.substate == PlayCellarState::FadeOut )
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
    else if ( state.playCellar.substate == PlayCellarState::LoadRoom )
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
    else if ( state.playCellar.substate == PlayCellarState::FadeIn )
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
    else if ( state.playCellar.substate == PlayCellarState::Walk )
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
}

void World::DrawPlayCellar()
{
    Graphics::SetClip( 0, TileMapBaseY, TileMapWidth, TileMapHeight );
    DrawRoomNoObjects( state.playCellar.playerPriority );
    Graphics::ResetClip();
}

void World::GotoLeaveCellar()
{
    state.leaveCellar.substate = LeaveCellarState::Start;

    curUpdate = &World::UpdateLeaveCellar;
    curDraw = &World::DrawLeaveCellar;
}

void World::UpdateLeaveCellar()
{
    if ( state.leaveCellar.substate == LeaveCellarState::Start )
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
    else if ( state.leaveCellar.substate == LeaveCellarState::FadeOut )
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
    else if ( state.leaveCellar.substate == LeaveCellarState::LoadRoom )
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
    else if ( state.leaveCellar.substate == LeaveCellarState::FadeIn )
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
    else if ( state.leaveCellar.substate == LeaveCellarState::Walk )
    {
        GotoEnter( Dir_None );
    }
    else if ( state.leaveCellar.substate == LeaveCellarState::Wait )
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
    else if ( state.leaveCellar.substate == LeaveCellarState::LoadOverworldRoom )
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
}

void World::DrawLeaveCellar()
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

void World::GotoPlayCave()
{
    state.playCave.substate = PlayCaveState::Start;

    curUpdate = &World::UpdatePlayCave;
    curDraw = &World::DrawPlayCave;
}

void World::UpdatePlayCave()
{
    if ( state.playCave.substate == PlayCaveState::Start )
    {
        state.playCave.substate = PlayCaveState::Wait;
        state.playCave.timer = 27;
    }
    else if ( state.playCave.substate == PlayCaveState::Wait )
    {
        if ( state.playCave.timer == 0 )
            state.playCave.substate = PlayCaveState::LoadRoom;
        else
            state.playCave.timer--;
    }
    else if ( state.playCave.substate == PlayCaveState::LoadRoom )
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
    else if ( state.playCave.substate == PlayCaveState::Walk )
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
}

void World::DrawPlayCave()
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

void World::GotoDie()
{
    state.death.substate = DeathState::Start;

    curUpdate = &World::UpdateDie;
    curDraw = &World::DrawDie;
}

void World::UpdateDie()
{
    // ORIGINAL: Some of these are handled with object timers.
    if ( state.death.timer > 0 )
        state.death.timer--;

    if ( state.death.substate == DeathState::Start )
    {
        player->SetInvincibilityTimer( 0x10 );
        state.death.timer = 0x20;
        state.death.substate = DeathState::Flash;
        Sound::StopEffects();
        Sound::PlaySong( Song_death, Sound::MainSongStream, false );
    }
    else if ( state.death.substate == DeathState::Flash )
    {
        player->DecInvincibleTimer();

        if ( state.death.timer == 0 )
        {
            state.death.timer = 6;
            state.death.substate = DeathState::Wait1;
        }
    }
    else if ( state.death.substate == DeathState::Wait1 )
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
    else if ( state.death.substate == DeathState::Turn )
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
    else if ( state.death.substate == DeathState::Fade )
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
    else if ( state.death.substate == DeathState::GrayLink )
    {
        static const uint8_t grayPal[4] = { 0, 0x10, 0x30, 0 };

        Graphics::SetPaletteIndexed( PlayerPalette, grayPal );
        Graphics::UpdatePalettes();

        state.death.substate = DeathState::Spark;
        state.death.timer = 0x18;
        state.death.step = 0;
    }
    else if ( state.death.substate == DeathState::Spark )
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
    else if ( state.death.substate == DeathState::Wait2 )
    {
        if ( state.death.timer == 0 )
        {
            state.death.substate = DeathState::GameOver;
            state.death.timer = 0x60;
        }
    }
    else if ( state.death.substate == DeathState::GameOver )
    {
        if ( state.death.timer == 0 )
        {
            profile.Deaths++;
            GotoContinueQuestion();
        }
    }
}

void World::DrawDie()
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

void World::GotoContinueQuestion()
{
    state.continueQuestion.substate = ContinueState::Start;
    state.continueQuestion.selectedIndex = 0;

    curUpdate = &World::UpdateContinueQuestion;
    curDraw = &World::DrawContinueQuestion;
}

void World::UpdateContinueQuestion()
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

            if ( state.continueQuestion.selectedIndex == 0 )
            {
                Sound::StopAll();
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

void World::DrawContinueQuestion()
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

void World::GotoFileMenu()
{
    auto summaries = std::make_shared<ProfileSummarySnapshot>();
    SaveFolder::ReadSummaries( *summaries.get() );
    GotoFileMenu( summaries );
}

void World::GotoFileMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    if ( nextGameMenu != nullptr )
        delete nextGameMenu;

    nextGameMenu = new GameMenu( summaries );
    curUpdate = &World::UpdateGameMenu;
    curDraw = &World::DrawGameMenu;
}

void World::GotoRegisterMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    if ( nextGameMenu != nullptr )
        delete nextGameMenu;

    nextGameMenu = new RegisterMenu( summaries );
    curUpdate = &World::UpdateRegisterMenu;
    curDraw = &World::DrawGameMenu;
}

void World::GotoEliminateMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries )
{
    if ( nextGameMenu != nullptr )
        delete nextGameMenu;

    nextGameMenu = new EliminateMenu( summaries );
    curUpdate = &World::UpdateEliminateMenu;
    curDraw = &World::DrawGameMenu;
}

void World::UpdateGameMenu()
{
    gameMenu->Update();
}

void World::UpdateRegisterMenu()
{
    gameMenu->Update();
}

void World::UpdateEliminateMenu()
{
    gameMenu->Update();
}

void World::DrawGameMenu()
{
    if ( gameMenu != nullptr )
        gameMenu->Draw();
}

int World::FindCellarRoomId( int mainRoomId, bool& isLeft )
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

void World::DrawRoomNoObjects( SpritePriority playerPriority )
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

void World::NoneTileAction( int row, int col, TileInteraction interaction )
{
    // Nothing to do. Should never be called.
}

void World::PushTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    Rock*   rock = new Rock();
    rock->SetX( col * TileWidth );
    rock->SetY( TileMapBaseY + row * TileHeight );
    SetBlockObj( rock );
}

void World::BombTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    if ( GotSecret() )
    {
        tileMaps[curTileMapIndex].tileRefs[row][col] = Tile_Cave;
    }
    else
    {
        RockWall*   rockWall = new RockWall();
        rockWall->SetX( col * TileWidth );
        rockWall->SetY( TileMapBaseY + row * TileHeight );
        SetBlockObj( rockWall );
    }
}

void World::BurnTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    if ( GotSecret() )
    {
        tileMaps[curTileMapIndex].tileRefs[row][col] = Tile_Stairs;
    }
    else
    {
        Tree*   tree = new Tree();
        tree->SetX( col * TileWidth );
        tree->SetY( TileMapBaseY + row * TileHeight );
        SetBlockObj( tree );
    }
}

void World::HeadstoneTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    Headstone*  headstone = new Headstone();
    headstone->SetX( col * TileWidth );
    headstone->SetY( TileMapBaseY + row * TileHeight );
    SetBlockObj( headstone );
}

void World::LadderTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Touch )
        return;

    _RPT2( _CRT_WARN, "Touch water: %d, %d\n", row, col );
}

void World::RaftTileAction( int row, int col, TileInteraction interaction )
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

void World::CaveTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Cover )
        return;

    if ( IsOverworld() )
    {
        int t = GetMapTile( row, col );
        GotoStairs( t );
    }

    _RPT2( _CRT_WARN, "Cover cave: %d, %d\n", row, col );
}

void World::StairsTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Cover )
        return;

    if ( GetMode() == Mode_Play )
        GotoStairs( Tile_Stairs );

    _RPT2( _CRT_WARN, "Cover stairs: %d, %d\n", row, col );
}

void World::GhostTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Push )
        return;

    _RPT2( _CRT_WARN, "Push headstone: %d, %d\n", row, col );

    MakeActivatedObject( Obj_FlyingGhini, row, col );
}

void World::ArmosTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Push )
        return;

    _RPT2( _CRT_WARN, "Push armos: %d, %d\n", row, col );

    MakeActivatedObject( Obj_Armos, row, col );
}

void World::MakeActivatedObject( int type, int row, int col )
{
    row += 4;

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

void World::BlockTileAction( int row, int col, TileInteraction interaction )
{
    if ( interaction != TInteract_Load )
        return;

    Block*  block = new Block();
    block->SetX( col * TileWidth );
    block->SetY( TileMapBaseY + row * TileHeight );
    SetBlockObj( block );
}
