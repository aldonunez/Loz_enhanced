/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "RoomAttrs.h"
#include "StatusBar.h"
#include "Submenu.h"
#include "TileBehavior.h"

// TODO: Work around a compiler bug by including this file to define ObjectAttrs before 
//       WorldImpl::GetObjectAttrs. The problem shows up, because I changed GetObjectAttrs
//       from a instance method to a static method.
#include "Object.h"

class Player;
class Object;
class Ladder;
struct CaveSpec;
struct Profile;
struct SparseRoomItem;
struct ObjectAttr;
class Credits;
class TextBox;
class Menu;
struct ProfileSummarySnapshot;
struct UWRoomFlags;


enum SpritePriority
{
    SpritePri_None,
    SpritePri_AboveBg,
    SpritePri_BelowBg,
};

enum
{
    Mob_Cave        = 0x0C,
    Mob_Ground      = 0x0E,
    Mob_Stairs      = 0x12,
    Mob_Rock        = 0x13,
    Mob_Headstone   = 0x14,

    Mob_Block       = 0,
    Mob_Tile        = 1,
    Mob_UW_Stairs   = 4,

    Tile_Rock       = 0xC8,
    Tile_Headstone  = 0xBC,
    Tile_Block      = 0xB0,
    Tile_WallEdge   = 0xF6,
};

enum CollisionResponse
{
    Collision_Unknown,
    Collision_Free,
    Collision_Blocked
};

enum ObjSlot
{
    MonsterSlot1,
    MonsterSlot11 = MonsterSlot1 + 10,
    BufferSlot,
    PlayerSwordSlot,
    PlayerSwordShotSlot,
    BoomerangSlot,
    // ORIGINAL: There are two slots that bombs and fires share.
    BombSlot,
    BombSlot2,
    FireSlot,
    FireSlot2,
    LadderSlot,
    FoodSlot,
    ArrowSlot,
    ItemSlot,
    FluteMusicSlot,
    // ORIGINAL: The player is first.
    PlayerSlot,
    DoorSlot,
    MaxObjects,

    FirstBombSlot = BombSlot,
    LastBombSlot = BombSlot2 + 1,

    FirstFireSlot = FireSlot,
    LastFireSlot = FireSlot2 + 1,

    LastMonsterSlot = MonsterSlot11,
    MonsterSlotEnd = MonsterSlot11 + 1,
    MaxMonsters = MonsterSlotEnd - MonsterSlot1,
    MaxObjListSize = 9,

    // Simple synonyms
    WhirlwindSlot = MonsterSlot1 + 8,
    BlockSlot = BufferSlot,

    // Object timers
    FadeTimerSlot = BufferSlot,

    // Long timers
    EdgeObjTimerSlot = BufferSlot + 2,
    RedLeeverClassTimerSlot = ArrowSlot,
    NoSwordTimerSlot = BoomerangSlot,
    ObservedPlayerTimerSlot = PlayerSwordSlot,
};

enum GameMode
{
    Mode_Demo,
    Mode_GameMenu,
    Mode_LoadLevel,
    Mode_Unfurl,
    Mode_Enter,
    Mode_Play,
    Mode_Leave,
    Mode_Scroll,
    Mode_ContinueQuestion,
    Mode_PlayCellar,
    Mode_LeaveCellar,
    Mode_PlayCave,
    Mode_PlayShortcuts,
    Mode_UnknownD__,
    Mode_Register,
    Mode_Elimination,
    Mode_Stairs,
    Mode_Death,
    Mode_EndLevel,
    Mode_WinGame,

    Mode_InitPlayCellar,
    Mode_InitPlayCave,

    Mode_Max,
};

enum DoorType
{
    DoorType_Open,
    DoorType_None,
    DoorType_FalseWall,
    DoorType_FalseWall2,
    DoorType_Bombable,
    DoorType_Key,
    DoorType_Key2,
    DoorType_Shutter
};

struct TileCollision
{
    bool            Collides;
    TileBehavior    TileBehavior;
    uint8_t         FineCol;
    uint8_t         FineRow;

    operator bool() const { return Collides; }
};


class World
{
public:
    static const int MobRows = 11;
    static const int MobColumns = 16;
    static const int Rows = 22;
    static const int Columns = 32;
    static const int BaseRows = 8;
    static const int MobTileWidth = 16;
    static const int MobTileHeight = 16;
    static const int TileWidth = 8;
    static const int TileHeight = 8;
    static const int TileMapWidth = Columns * TileWidth;
    static const int TileMapHeight = Rows * TileHeight;
    static const int TileMapBaseY = 64;

    static const int WorldLimitLeft = 0;
    static const int WorldLimitRight = TileMapWidth;
    static const int WorldLimitTop = TileMapBaseY;
    static const int WorldLimitBottom = WorldLimitTop + TileMapHeight;

    static const int WorldMidX = WorldLimitLeft + TileMapWidth / 2;

    static const int WorldWidth = 16;
    static const int WorldHeight = 8;

    struct LevelInfoBlock
    {
        static const int LevelPaletteCount = 8;
        static const int LevelShortcutCount = 4;
        static const int LevelCellarCount = 10;
        static const int FadeLength = 4;
        static const int FadePals = 2;
        static const int MapLength = 16;

        uint8_t     Palettes[LevelPaletteCount][PaletteLength];
        uint8_t     StartY;
        uint8_t     StartRoomId;
        uint8_t     TriforceRoomId;
        uint8_t     BossRoomId;
        uint8_t     Song;
        uint8_t     LevelNumber;
        uint8_t     EffectiveLevelNumber;
        uint8_t     DrawnMapOffset;
        uint8_t     CellarRoomIds[LevelCellarCount];
        uint8_t     ShortcutPosition[LevelShortcutCount];
        uint8_t     DrawnMap[MapLength];
        uint8_t     Padding[2];
        uint8_t     OutOfCellarPaletteSeq[FadeLength][FadePals][PaletteLength];
        uint8_t     InCellarPaletteSeq[FadeLength][FadePals][PaletteLength];
        uint8_t     DarkPaletteSeq[FadeLength][FadePals][PaletteLength];
        uint8_t     DeathPaletteSeq[FadeLength][FadePals][PaletteLength];
    };

protected:
    World();

public:
    static void Init();
    static void Uninit();
    static void Start( int slot, const Profile& profile );

    static void Update();
    static void Draw();

    static void PauseFillHearts();
    static void LeaveRoom( Direction dir, int currentRoomId );
    static void LeaveCellar();
    static void LeaveCellarByShortcut( int targetRoomId );
    static void Die();
    static void UnfurlLevel();
    static void ChooseFile( const std::shared_ptr<ProfileSummarySnapshot>& summaries );
    static void RegisterFile( const std::shared_ptr<ProfileSummarySnapshot>& summaries );
    static void EliminateFile( const std::shared_ptr<ProfileSummarySnapshot>& summaries );
    static TileCollision CollidesWithTileStill( int x, int y );
    static TileCollision CollidesWithTileMoving( int x, int y, Direction dir, bool isPlayer );
    static void OnPushedBlock();
    static void OnActivatedArmos( int x, int y );
    static void OnTouchedPowerTriforce();
    static bool IsPlaying();
    static bool IsPlaying( GameMode mode );
    static GameMode GetMode();
    static int GetMarginRight();
    static int GetMarginLeft();
    static int GetMarginBottom();
    static int GetMarginTop();
    static void PushTile( int row, int col );
    static void TouchTile( int row, int col );
    static void CoverTile( int row, int col );
    static Player* GetPlayer();
    static Point GetObservedPlayerPos();
    static Ladder* GetLadder();
    static void SetLadder( Ladder* ladder );
    static void UseRecorder();
    static void SetMobXY( int x, int y, int mob );
    static int GetInnerPalette();
    static Cell GetRandomWaterTile();
    static Object* GetObject( int slot );
    static void SetObject( int slot, Object* obj );
    static int FindEmptyMonsterSlot();
    static int FindEmptyFireSlot();
    static int GetCurrentObjectSlot();
    static void SetCurrentObjectSlot( int slot );
    static int& GetObjectTimer( int slot );
    static void SetObjectTimer( int slot, int value );
    static int GetStunTimer( int slot );
    static void SetStunTimer( int slot, int value );
    static int GetRecorderUsed();
    static void SetRecorderUsed( int value );
    static bool GetCandleUsed();
    static void SetCandleUsed();
    static int  GetWhirlwindTeleporting();
    static void SetWhirlwindTeleporting( int value );
    static const uint8_t* GetString( int stringId );
    static bool IsSwordBlocked();
    static void SetSwordBlocked( bool value );

    static Profile& GetProfile();
    static uint8_t GetItem( int itemSlot );
    static void SetItem( int itemSlot, int value );
    static void PostRupeeWin( uint8_t value );
    static void PostRupeeLoss( uint8_t value );
    static void AddItem( int itemId );
    static void DecrementItem( int itemSlot );
    static void FillHearts( int heartValue );
    static bool HasCurrentMap();
    static bool HasCurrentCompass();
    static bool HasCurrentLevelItem( int itemSlot1To8, int itemSlot9 );
    static bool IsLiftingItem();
    static bool IsUWCellar();
    static void EndLevel();
    static void WinGame();
    static void AddUWRoomItem();
    static Direction GetDoorwayDir();
    static void SetDoorwayDir( Direction dir );
    static int  GetFromUnderground();
    static void SetFromUnderground( int value );
    static int  GetActiveShots();
    static void SetActiveShots( int count );
    static Direction GetShuttersPassedDirs();
    static void SetShuttersPassedDirs( Direction dir );
    static int  GetTriggeredDoorCmd();
    static void SetTriggeredDoorCmd( int value );
    static Direction GetTriggeredDoorDir();
    static void SetTriggeredDoorDir( Direction dir );
    static int  GetRoomId();
    static bool IsOverworld();
    static bool DoesRoomSupportLadder();
    static int  GetTileAction( int tileRef );
    static DoorType GetDoorType( Direction dir );
    static DoorType GetDoorType( int roomId, Direction dir );
    static bool GetDoorState( int door );
    static UWRoomFlags& GetUWRoomFlags( int curRoomId );

    static const LevelInfoBlock* GetLevelInfo();
    static bool IsUWMain( int roomId );
    static bool IsPlayingCave();

    static bool GotItem();
    static bool GotItem( int roomId );
    static void TakeSecret();
    static void MarkItem();
    static void LiftItem( int itemId, uint16_t timer = 0x80 );
    static void OpenShutters();
    static void ResetKilledObjectCount();
    static void IncrementKilledObjectCount( bool allowBombDrop );
    static void IncrementRoomKillCount();
    static void SetBombItemDrop();
    static int  GetRoomObjCount();
    static void SetRoomObjCount( int value );
    static int  GetRoomObjId();
    static void SetObservedPlayerPos( int x, int y );

    static ObjectAttr GetObjectAttrs( int type );
    static int GetObjectMaxHP( int type );
    static int GetPlayerDamage( int type );

    static void SetPersonWallY( int y );
    static int  GetFadeStep();
    static void BeginFadeIn();
    static void FadeIn();
    static bool HasLivingObjects();
    static void EnablePersonFireballs();
    static Util::Array<uint8_t> GetShortcutRooms();
};
