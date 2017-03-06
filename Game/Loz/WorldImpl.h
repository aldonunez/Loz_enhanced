/*
   Copyright 2017 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "World.h"
#include "Profile.h"


class WorldImpl : private World
{
public:
    enum TileInteraction
    {
        TInteract_Load,
        TInteract_Push,
        TInteract_Touch,
        TInteract_Cover,
    };

private:
    static const int Rooms = 128;
    static const int UniqueRooms = 124;
    static const int ColumnTables = 16;
    static const int ScrollSpeed = 4;
    static const int TileTypes = 56;
    static const int TileActions = 16;
    static const int LoadingTileActions = 4;
    static const int SparseAttrs = 11;
    static const int RoomHistoryLength = 6;
    static const int Modes = Mode_Max;

    struct LevelDirectory
    {
        typedef char FixedString[32];

        FixedString LevelInfoBlock;
        FixedString RoomCols;
        FixedString ColTables;
        FixedString TileAttrs;
        FixedString TilesImage;
        FixedString PlayerImage;
        FixedString PlayerSheet;
        FixedString NpcImage;
        FixedString NpcSheet;
        FixedString BossImage;
        FixedString BossSheet;
        FixedString RoomAttrs;
        FixedString LevelInfoEx;
        FixedString ObjLists;
        FixedString Extra1;
        FixedString Extra2;
        FixedString Extra3;
        FixedString Extra4;
    };

    struct RoomCols
    {
        uint8_t     ColumnDesc[Columns];
    };

    struct TileMap
    {
        uint8_t     tileRefs[Rows][Columns];
    };

    struct SparsePos
    {
        uint8_t     roomId;
        uint8_t     pos;
    };

    struct SparsePos2
    {
        uint8_t     roomId;
        uint8_t     x;
        uint8_t     y;
    };

    typedef Util::Table<uint8_t> ColumnResTable;
    typedef Util::Table<uint8_t> SparseAttrTable;
    typedef Util::Table<uint8_t> OWExtraTable;
    typedef Util::Table<uint8_t> ObjListTable;
    typedef Util::Table<uint8_t> TextTable;
    typedef void (WorldImpl::*UpdateFunc)();
    typedef void (WorldImpl::*DrawFunc)();
    typedef void (WorldImpl::*TileActionFunc)( int row, int col, TileInteraction interaction );

public:
    LevelDirectory  directory;
    LevelInfoBlock  infoBlock;
    RoomCols        roomCols[UniqueRooms];
    ColumnResTable  colTables;
    TileMap         tileMaps[2];
    RoomAttrs       roomAttrs[Rooms];
    int             curRoomId;
    int             curTileMapIndex;
    uint8_t         tileAttrs[TileTypes];
    SparseAttrTable sparseRoomAttrs;
    OWExtraTable    extraData;
    ObjListTable    objLists;
    TextTable       textTable;

    int             rowCount;
    int             colCount;
    int             startRow;
    int             startCol;
    int             tileTypeCount;
    int             marginRight;
    int             marginLeft;
    int             marginBottom;
    int             marginTop;
    ALLEGRO_BITMAP* wallsBmp;
    ALLEGRO_BITMAP* doorsBmp;

    GameMode        lastMode;
    GameMode        curMode;
    StatusBar       statusBar;
    Submenu         menu;
    Credits*        credits;
    TextBox*        textBox1;
    TextBox*        textBox2;
    Menu*           gameMenu;
    Menu*           nextGameMenu;

private:
    struct PlayState
    {
        enum Substate
        {
            Active,
        };

        enum RoomType
        {
            Regular,
            Cellar,
            Cave,
        };

        Substate    substate;
        int         timer;

        bool        animatingRoomColors;
        bool        allowWalkOnWater;
        bool        uncoveredRecorderSecret;
        RoomType    roomType;
        uint16_t    liftItemTimer;
        int         liftItemId;
        int         personWallY;
    };

    struct PlayCellarState
    {
        enum Substate
        {
            Start,
            FadeOut,
            LoadRoom,
            FadeIn,
            Walk,

            MaxSubstate
        };

        Substate        substate;
        SpritePriority  playerPriority;
        int             targetY;
        int             fadeTimer;
        int             fadeStep;
    };

    struct LeaveCellarState
    {
        enum Substate
        {
            Start,
            FadeOut,
            LoadRoom,
            FadeIn,
            Walk,

            Wait,
            LoadOverworldRoom,

            MaxSubstate
        };

        Substate        substate;
        int             fadeTimer;
        int             fadeStep;
        int             timer;
    };

    struct PlayCaveState
    {
        enum Substate
        {
            Start,
            Wait,
            LoadRoom,
            Walk,

            MaxSubstate
        };

        Substate        substate;
        int             timer;
        int             targetY;
        SpritePriority  playerPriority;
    };

    struct ScrollState
    {
        enum Substate
        {
            Start,
            AnimatingColors,
            FadeOut,
            LoadRoom,
            Scroll,

            MaxSubstate
        };

        static const int StateTime = 32;

        Substate    substate;
        int         timer;
        Direction   scrollDir;
        int         nextRoomId;
        int         curRoomId;

        int         offsetX;
        int         offsetY;
        int         speedX;
        int         speedY;
        int         oldTileMapIndex;
        int         oldRoomId;
        int         oldMapToNewMapDistX;
        int         oldMapToNewMapDistY;
    };

    struct LeaveState
    {
        static const int StateTime = 2;

        int         timer;
        Direction   scrollDir;
        int         curRoomId;
    };

    struct EnterState
    {
        enum Substate
        {
            Start,
            Wait,
            FadeIn,
            Walk,
            WalkCave,

            MaxSubstate
        };

        static const int StateTime = 2;

        Substate    substate;
        int         timer;
        Direction   scrollDir;
        int         targetX;
        int         targetY;
        int         playerSpeed;
        int         playerFraction;
        SpritePriority  playerPriority;
        bool        gotoPlay;
    };

    struct LoadLevelState
    {
        enum Substate
        {
            Load,
            Wait,
        };

        static const int StateTime = 18;

        Substate    substate;
        int         timer;
        int         level;
        bool        restartOW;
    };

    struct UnfurlState
    {
        enum Substate
        {
            Start,
            Unfurl,
        };

        static const int StateTime = 11;

        Substate    substate;
        int         timer;
        int         stepTimer;
        int         left;
        int         right;
        bool        restartOW;
    };

    struct EndLevelState
    {
        enum Substate
        {
            Start,
            Wait1,
            Flash,
            FillHearts,
            Wait2,
            Furl,
            Wait3,

            MaxSubstate
        };

        static const int Wait1Time = 0x30;
        static const int FlashTime = 0x30;
        static const int Wait2Time = 0x80;
        static const int Wait3Time = 0x80;

        Substate    substate;
        int         timer;
        int         stepTimer;
        int         left;
        int         right;
    };

    struct WinGameState
    {
        enum Substate
        {
            Start,
            Text1,
            Stand,
            Hold1,
            Colors,
            Hold2,
            Text2,
            Hold3,
            NoObjects,
            Credits,

            MaxSubstate
        };

        enum NpcVisual
        {
            Npc_None,
            Npc_Stand,
            Npc_Lift,
        };

        enum
        {
            TextBox2Top = 0xA8,
        };

        Substate    substate;
        int         timer;
        int         stepTimer;
        int         left;
        int         right;
        NpcVisual   npcVisual;
    };

    struct StairsState
    {
        enum Substate
        {
            Start,
            Walk,
            WalkCave,

            MaxSubstate
        };

        Substate        substate;
        Direction       scrollDir;
        int             targetX;
        int             targetY;
        int             playerSpeed;
        int             playerFraction;
        int             tileRef;
        SpritePriority  playerPriority;
    };

    struct DeathState
    {
        enum Substate
        {
            Start,
            Flash,
            Wait1,
            Turn,
            Fade,
            GrayLink,
            Spark,
            Wait2,
            GameOver,

            MaxSubstate
        };

        Substate    substate;
        int         timer;
        int         step;
    };

    struct ContinueState
    {
        enum Substate
        {
            Start,
            Idle,
            Chosen,

            MaxSubstate
        };

        Substate    substate;
        int         timer;
        int         selectedIndex;
    };

    union State
    {
        PlayState           play;
        PlayCellarState     playCellar;
        LeaveCellarState    leaveCellar;
        PlayCaveState       playCave;
        ScrollState         scroll;
        LeaveState          leave;
        EnterState          enter;
        LoadLevelState      loadLevel;
        UnfurlState         unfurl;
        StairsState         stairs;
        EndLevelState       endLevel;
        WinGameState        winGame;
        DeathState          death;
        ContinueState       continueQuestion;
    };

    static TileActionFunc sActionFuncs[TileActions];
    static UpdateFunc sModeFuncs[Modes];
    static DrawFunc sDrawFuncs[Modes];

    static UpdateFunc sPlayCellarFuncs[PlayCellarState::MaxSubstate];
    static UpdateFunc sLeaveCellarFuncs[LeaveCellarState::MaxSubstate];
    static UpdateFunc sPlayCaveFuncs[PlayCaveState::MaxSubstate];
    static UpdateFunc sScrollFuncs[ScrollState::MaxSubstate];
    static UpdateFunc sEnterFuncs[EnterState::MaxSubstate];
    static UpdateFunc sEndLevelFuncs[EndLevelState::MaxSubstate];
    static UpdateFunc sWinGameFuncs[WinGameState::MaxSubstate];
    static UpdateFunc sDeathFuncs[DeathState::MaxSubstate];

public:
    State           state;
    int             curColorSeqNum;
    int             darkRoomFadeStep;
    int             curMazeStep;
    int             spotIndex;
    int             tempShutterRoomId;
    int             tempShutterDoorDir;
    int             tempShuttersRoomId;
    bool            tempShutters;
    bool            prevRoomWasCellar;
    int             savedOWRoomId;
    int             edgeX;
    int             edgeY;
    int             nextRoomHistorySlot;    // 620
    int             roomObjCount;           // 34E
    int             roomObjId;              // 35F
    uint8_t         worldKillCycle;         // 52A
    uint8_t         worldKillCount;         // 627
    uint8_t         helpDropCounter;        // 50
    uint8_t         helpDropValue;          // 51
    int             roomKillCount;          // 34F
    bool            roomAllDead;            // 34D
    bool            madeRoomItem;
    bool            enablePersonFireballs;
    bool            swordBlocked;           // 52E
    uint8_t         whirlwindTeleporting;   // 522
    uint8_t         teleportingRoomIndex;   // 523
    uint8_t         pause;                  // E0
    uint8_t         submenu;                // E1
    int             submenuOffsetY;         // EC
    bool            statusBarVisible;
    uint8_t         levelKillCounts[LevelBlockRooms];
    uint8_t         roomHistory[RoomHistoryLength];

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

public:
    WorldImpl();
    ~WorldImpl();

    void Init();
    void Start( int slot, const Profile& profile );

    void Update();
    void Draw();

public:
    bool IsUWCellar( int roomId );
    void TakeShortcut();
    bool GetDoorState( int door );

    bool FindSparseFlag( int attrId, int roomId );
    const ObjectAttr* GetObjectAttrs();

    void AddUWRoomItem( int roomId );
    void PostRupeeChange( uint8_t value, int itemSlot );

    void GotoLeave( Direction dir, int currentRoomId );
    void GotoUnfurl( bool restartOW = false );
    void GotoEndLevel();
    void GotoWinGame();
    void GotoLeaveCellar();
    void GotoDie();
    void GotoFileMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries );
    void GotoRegisterMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries );
    void GotoEliminateMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries );

public:
    const uint8_t* GetString( int stringId );
    TileCollision CollidesWithTile( int x, int y, Direction dir, int offset );
    void OnPushedBlock();
    void OnActivatedArmos( int x, int y );

public:
    void UseRecorder();
    void MakeFluteSecret();
    void SetTile( int x, int y, int tileType );
    int GetInnerPalette();
    Point GetRandomWaterTile();
    void FadeIn();
    void InteractTile( int row, int col, TileInteraction interaction );

public:
    void AddItem( int itemId );
    void DecrementItem( int itemSlot );
    bool HasCurrentLevelItem( int itemSlot1To8, int itemSlot9 );
    void FillHearts( int heartValue );

    void SetOnlyObject( int slot, Object* obj );
    Ladder* GetLadderObj();
    void SetLadderObj( Ladder* ladder );
    int FindEmptyMonsterSlot();

    void OnTouchedPowerTriforce();
    void LiftItem( int itemId, uint16_t timer );

private:
    bool GotShortcut( int roomId );
    bool GotSecret();
    bool UseKey();
    bool GetDoorState( int roomId, int door );
    void SetDoorState( int roomId, int door );
    bool IsRoomInHistory();
    void AddRoomToHistory();
    void SaveObjectCount();
    void CalcObjCountToMake( int& objId, int& count );
    void OnLeavePlay();
    void ClearLevelData();

    const SparsePos* FindSparsePos( int attrId, int roomId );
    const SparsePos2* FindSparsePos2( int attrId, int roomId );
    const SparseRoomItem* FindSparseItem( int attrId, int roomId );

    void LoadLevel( int level );
    void LoadRoom( int roomId, int tileMapIndex );
    void LoadMap( int roomId, int tileMapIndex );
    void LoadLayout( int uniqueRoomId, int tileMapIndex, bool owTileFormat );
    void LoadCaveRoom( int uniqueRoomId );

    void LoadOverworldContext();
    void LoadUnderworldContext();
    void LoadCellarContext();

    void LoadOpenRoomContext();
    void LoadClosedRoomContext();
    void LoadMapResourcesFromDirectory( int uniqueRoomCount );

    void DrawRoom();
    void DrawMap( int roomId, int mapIndex, int offsetX, int offsetY );
    void DrawDoors( int roomId, bool above, int offsetX, int offsetY );

    int  GetNextTeleportingRoomIndex();
    void UpdateRoomColors();
    void CheckBombables();
    void CheckSecrets();
    void CheckShutters();
    void UpdateDoors2();
    bool CalcHasLivingObjects();
    void AddUWRoomStairs();
    void KillAllObjects();
    void MoveRoomItem();
    void UpdateStatues();
    void UpdateObservedPlayerPos();
    void UpdateRupees();
    void UpdateLiftItem();
    void DrawObjects( Object** objOverPlayer = nullptr );
    void DrawLinkLiftingItem( int itemId );
    void DrawSubmenu();

    void MakeObjects( Direction entryDir );
    void MakeCellarObjects();
    void MakeCaveObjects();
    void MakeUnderworldPerson( int objId );
    void MakePersonRoomObjects( int type, const CaveSpec* spec );
    void MakeWhirlwind();
    bool FindSpawnPos( int type, const RSpot* spots, int len, int& x, int& y );
    bool CalcMazeStayPut( Direction dir );
    void PutEdgeObject();
    void HandleNormalObjectDeath();
    void TryDroppingItem( int origType, int x, int y );
    void FillHeartsStep();
    void UpdateSubmenu();

    void GotoPlay( PlayState::RoomType roomType = PlayState::Regular );
    void UpdatePlay();
    void DrawPlay();

    void GotoScroll( Direction dir );
    void GotoScroll( Direction dir, int currentRoomId );
    void UpdateScroll();
    void UpdateScroll_Start();
    void UpdateScroll_AnimatingColors();
    void UpdateScroll_FadeOut();
    void UpdateScroll_LoadRoom();
    void UpdateScroll_Scroll();
    void DrawScroll();

    void GotoLeave( Direction dir );
    void UpdateLeave();
    void DrawLeave();

    void GotoEnter( Direction dir );
    void UpdateEnter();
    void UpdateEnter_Start();
    void UpdateEnter_Wait();
    void UpdateEnter_FadeIn();
    void UpdateEnter_Walk();
    void UpdateEnter_WalkCave();
    void DrawEnter();

    void GotoLoadLevel( int level, bool restartOW = false );
    void UpdateLoadLevel();
    void DrawLoadLevel();

    void UpdateUnfurl();
    void DrawUnfurl();

    void UpdateEndLevel();
    void UpdateEndLevel_Start();
    void UpdateEndLevel_Wait();
    void UpdateEndLevel_Flash();
    void UpdateEndLevel_FillHearts();
    void UpdateEndLevel_Furl();
    void DrawEndLevel();

    void UpdateWinGame();
    void UpdateWinGame_Start();
    void UpdateWinGame_Text1();
    void UpdateWinGame_Stand();
    void UpdateWinGame_Hold1();
    void UpdateWinGame_Colors();
    void UpdateWinGame_Hold2();
    void UpdateWinGame_Text2();
    void UpdateWinGame_Hold3();
    void UpdateWinGame_NoObjects();
    void UpdateWinGame_Credits();
    void DrawWinGame();

    void GotoStairs( int tileRef );
    void UpdateStairsState();
    void DrawStairsState();

    void GotoPlayCellar();
    void UpdatePlayCellar();
    void UpdatePlayCellar_Start();
    void UpdatePlayCellar_FadeOut();
    void UpdatePlayCellar_LoadRoom();
    void UpdatePlayCellar_FadeIn();
    void UpdatePlayCellar_Walk();
    void DrawPlayCellar();

    void UpdateLeaveCellar();
    void UpdateLeaveCellar_Start();
    void UpdateLeaveCellar_FadeOut();
    void UpdateLeaveCellar_LoadRoom();
    void UpdateLeaveCellar_FadeIn();
    void UpdateLeaveCellar_Walk();
    void UpdateLeaveCellar_Wait();
    void UpdateLeaveCellar_LoadOverworldRoom();
    void DrawLeaveCellar();

    void GotoPlayCave();
    void UpdatePlayCave();
    void UpdatePlayCave_Start();
    void UpdatePlayCave_Wait();
    void UpdatePlayCave_LoadRoom();
    void UpdatePlayCave_Walk();
    void DrawPlayCave();

    void UpdateDie();
    void UpdateDie_Start();
    void UpdateDie_Flash();
    void UpdateDie_Wait1();
    void UpdateDie_Turn();
    void UpdateDie_Fade();
    void UpdateDie_GrayLink();
    void UpdateDie_Spark();
    void UpdateDie_Wait2();
    void UpdateDie_GameOver();
    void DrawDie();

    void GotoContinueQuestion();
    void UpdateContinueQuestion();
    void DrawContinueQuestion();

    void GotoFileMenu();
    void UpdateGameMenu();
    void UpdateRegisterMenu();
    void UpdateEliminateMenu();
    void DrawGameMenu();

    int FindCellarRoomId( int mainRoomId, bool& isLeft );
    void SetPlayerExitPosOW( int roomId );

    void DrawRoomNoObjects( SpritePriority playerPriority = SpritePri_AboveBg );

private:
    void SummonWhirlwind();
    int GetMapTile( int row, int col );
    void MakeActivatedObject( int type, int row, int col );

    bool CollidesWithUWBorder( int fineRow, int fineCol1, int fineCol2 );
    bool CollidesTile( int row, int col );
    void CheckPowerTriforceFanfare();
    void AdjustInventory();
    void WarnLowHPIfNeeded();
    void PlayAmbientSounds();
    void ShowShortcutStairs( int roomId, int tileMapIndex );

private:
    void NoneTileAction( int row, int col, TileInteraction interaction );
    void PushTileAction( int row, int col, TileInteraction interaction );
    void BombTileAction( int row, int col, TileInteraction interaction );
    void BurnTileAction( int row, int col, TileInteraction interaction );
    void HeadstoneTileAction( int row, int col, TileInteraction interaction );
    void LadderTileAction( int row, int col, TileInteraction interaction );
    void RaftTileAction( int row, int col, TileInteraction interaction );
    void CaveTileAction( int row, int col, TileInteraction interaction );
    void StairsTileAction( int row, int col, TileInteraction interaction );
    void GhostTileAction( int row, int col, TileInteraction interaction );
    void ArmosTileAction( int row, int col, TileInteraction interaction );
    void BlockTileAction( int row, int col, TileInteraction interaction );

private:
    void ClearRoomItemData();
    void SetPlayerColor();

    void ClearDeadObjectQueue();
    void SetBlockObj( Object* block );
    void DeleteObjects();
    void CleanUpRoomItems();
    void DeleteDeadObjects();
    void InitObjectTimers();
    void DecrementObjectTimers();
    void InitStunTimers();
    void DecrementStunTimers();
    void InitPlaceholderTypes();

    void MovePlayer( Direction dir, int speed, int& fraction );
};
