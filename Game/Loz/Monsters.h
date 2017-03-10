/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Object.h"
#include "SpriteAnimator.h"
#include "World.h"


struct WalkerSpec;
struct FlyerSpec;
struct JumperSpec;

class Bomb;


class Walker : public Object
{
protected:
    SpriteAnimator      animator;

    int                 curSpeed;
    int                 shootTimer;
    bool                wantToShoot;

    const WalkerSpec*   spec;

public:
    Walker( ObjType type, const WalkerSpec* spec, int x, int y );

    virtual void Draw();

protected:
    void SetSpec( const WalkerSpec* spec );
    void SetFacingAnimation();

    void TryShooting();
    int Shoot( ObjType shotType );

    bool TryBigShove();
};


class ChaseWalker : public Walker
{
public:
    ChaseWalker( ObjType type, const WalkerSpec* spec, int x, int y );

    virtual void Update();

protected:
    void TargetPlayer();
    void UpdateNoAnimation();
};


class StdChaseWalker : public ChaseWalker
{
public:
    StdChaseWalker( ObjType type, const WalkerSpec* spec, int x, int y );
};


class Goriya : public ChaseWalker, public IThrower
{
    ObjRef  shotRef;

public:
    Goriya( ObjType type, const WalkerSpec* spec, int x, int y );

    virtual void Update();
    virtual void* GetInterface( ObjInterfaces iface );

    virtual void Catch();

private:
    void TryThrowingBoomerang();
};


class Armos : public ChaseWalker
{
    int     state;

public:
    Armos( int x, int y );

    virtual void Update();
    virtual void Draw();
};


class Wanderer : public Walker
{
    int                 turnTimer;
    int                 turnRate;

public:
    Wanderer( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y );

    virtual void Update();

protected:
    void Move();
    void MoveIfNeeded();

private:
    void TargetPlayer();
    void TurnIfTime();
    void TurnX();
    void TurnY();
};


class StdWanderer : public Wanderer
{
public:
    StdWanderer( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y );
};


class DelayedWanderer : public Wanderer
{
public:
    DelayedWanderer( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y );
};


class Ghini : public Wanderer
{
public:
    Ghini( int x, int y );

    virtual void Update();
};


class Gibdo : public StdWanderer
{
public:
    Gibdo( int x, int y );

    virtual void Update();
};


class Darknut : public StdWanderer
{
public:
    Darknut( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y );

    virtual void Update();

    static Darknut* MakeBlueDarknut( int x, int y );
    static Darknut* MakeRedDarknut( int x, int y );
};


class Stalfos : public StdWanderer
{
public:
    Stalfos( int x, int y );

    virtual void Update();
};


class Gel : public Wanderer
{
    int                 state;

public:
    Gel( ObjType type, int x, int y, Direction dir, int fraction );

    virtual void Update();

private:
    void UpdateShove();
    void UpdateWander();
};


class Zol : public Wanderer
{
    int                 state;

public:
    Zol( int x, int y );

    virtual void Update();

private:
    void UpdateWander();
    void UpdateShove();
    void UpdateSplit();
};


class Bubble : public Wanderer
{
public:
    Bubble( ObjType type, int x, int y );

    virtual void Update();
    virtual void Draw();
};


class Vire : public Wanderer
{
    int     state;

public:
    Vire( int x, int y );

    virtual void Update();

private:
    void UpdateWander();
    void UpdateShove();
    void UpdateSplit();
};


class LikeLike : public Wanderer
{
    int     framesHeld;

public:
    LikeLike( int x, int y );

    virtual void Update();
};


class DigWanderer : public Wanderer
{
    const WalkerSpec**  stateSpecs;
    const int*          stateTimes;

protected:
    int     state;

public:
    DigWanderer( ObjType type, const WalkerSpec** stateSpecs, const int* stateTimes, int x, int y );

    virtual void Update();
    virtual void Draw();

protected:
    void UpdateDig();
};


class BlueLeever : public DigWanderer
{
public:
    BlueLeever( int x, int y );
};


class Zora : public DigWanderer
{
public:
    Zora();

    virtual void Update();
};


class RedLeever : public Object
{
    SpriteAnimator      animator;

    int                 state;
    const WalkerSpec*   spec;

    static int          count;

public:
    RedLeever( int x, int y );

    virtual void Update();
    virtual void Draw();

    static void ClearRoomData();

private:
    void SetSpec( const WalkerSpec* spec );
    void SetFacingAnimation();
    bool TargetPlayer();
};


class Flyer : public Object
{
    typedef void (Flyer::*StateFunc)();

    SpriteAnimator      animator;

    int                 state;
    int                 sprintsLeft;
    const FlyerSpec*    spec;

    static StateFunc    sStateFuncs[6];

protected:
    int                 curSpeed;
    int                 accelStep;

    Direction           deferredDir;
    int                 moveCounter;

public:
    Flyer( ObjType type, const FlyerSpec* spec, int x, int y );

    virtual void Draw();

protected:
    void UpdateStateAndMove();
    int GetState();
    void GoToState( int state, int sprints );
    void SetFacingAnimation();

    virtual void UpdateFullSpeedImpl();
    virtual void UpdateChaseImpl();
    virtual void UpdateTurnImpl();
    virtual int GetFrame();

private:
    void Move();
    void UpdateHastening();
    void UpdateFullSpeed();
    void UpdateChase();
    void UpdateTurn();
    void UpdateSlowing();
    void UpdateStill();
};


class StdFlyer : public Flyer
{
public:
    StdFlyer( ObjType type, const FlyerSpec* spec, int x, int y, Direction facing );
};


class Peahat : public StdFlyer
{
public:
    Peahat( int x, int y );

    virtual void Update();
};


class FlyingGhini : public Flyer
{
    int     state;

public:
    FlyingGhini( int x, int y );

    virtual void Update();
    virtual void Draw();

protected:
    virtual void UpdateFullSpeedImpl();
    virtual int GetFrame();
};


class Keese : public Flyer
{
public:
    virtual void Update();

    // This should be exposed higher up.
    void SetFacing( Direction dir );

    static Keese* MakeBlueKeese( int x, int y );
    static Keese* MakeRedKeese( int x, int y );
    static Keese* MakeBlackKeese( int x, int y );

protected:
    virtual void UpdateFullSpeedImpl();
    virtual int GetFrame();

private:
    Keese( ObjType type, const FlyerSpec* spec, int x, int y, int startSpeed );
};


class Moldorm : public Flyer
{
public:
    enum
    {
        HeadSlot1   = MonsterSlot1 + 4,
        HeadSlot2   = HeadSlot1 + 5,
        TailSlot1   = MonsterSlot1,
        TailSlot2   = TailSlot1 + 5
    };

private:
    Direction   oldFacing;

public:
    Moldorm( int x, int y );

    virtual void Update();

    static Object* MakeSet();

protected:
    virtual void UpdateTurnImpl();
    virtual void UpdateChaseImpl();
    virtual int GetFrame();

private:
    void UpdateSubstates();
    void ShiftFacings();
    void CheckMoldormCollisions();
};


class Patra : public Flyer
{
    int     xMove;
    int     yMove;
    int     maneuverState;
    int     childStateTimer;

public:
    int GetManeuverState();
    int GetXMove();
    int GetYMove();

    virtual void Update();
    virtual void UpdateFullSpeedImpl();

    static Patra* MakePatra( ObjType type );

private:
    Patra( ObjType type );
};


class PatraChild : public Object
{
    int             x;
    int             y;
    SpriteAnimator  animator;

    int             angleAccum;

public:
    PatraChild( ObjType type );

    virtual void Update();
    virtual void Draw();

private:
    void UpdateStart();
    void UpdateTurn();
};


class Jumper : public Object
{
    int                 curSpeed;
    int                 accelStep;
    SpriteAnimator      animator;

    int                 state;
    int                 targetY;
    int                 reversesPending;
    const JumperSpec*   spec;

public:
    Jumper( ObjType type, const JumperSpec* spec, int x, int y );
    ~Jumper();

    virtual void Update();
    virtual void Draw();

private:
    void UpdateStill();
    void UpdateJump();

    void UpdateY( int maxSpeed, int acceleration );
    void SetupJump();
    int GetRandomStillTime();
    void ConstrainFacing();
};


class Boulders : public Object
{
    static const int MaxBoulders = 3;

    static int      sCount;

public:
    Boulders();

    virtual void Update();
    virtual void Draw();

    static int& Count();
    static void ClearRoomData();
};


class Trap : public Object
{
    SpriteImage     image;

    int             trapIndex;
    int             state;
    int             speed;
    int             origCoord;

public:
    Trap( int trapIndex, int x, int y );

    virtual void Update();
    virtual void Draw();

    static Object* MakeSet( int count );

private:
    void UpdateIdle();
    void UpdateMoving();
};


class Rope : public Object
{
    SpriteAnimator  animator;

    int             speed;

public:
    Rope( int x, int y );

    virtual void Update();
    virtual void Draw();

private:
    void SetFacingAnimation();
    void TargetPlayer();
};


class PolsVoice : public Object
{
    int                 curSpeed;
    int                 accelStep;
    int                 state;
    int                 stateTimer;
    int                 targetY;
    SpriteAnimator      animator;

public:
    PolsVoice( int x, int y );

    virtual void Update();
    virtual void Draw();

private:
    void Move();
    void UpdateX();
    bool UpdateY();
    bool UpdateJumpY();
    bool UpdateWalkY();
    void SetupJump();
};


class RedWizzrobe : public Object
{
    typedef void (RedWizzrobe::*StateFunc)();

    SpriteAnimator      animator;

    uint8_t             stateTimer;
    uint8_t             flashTimer;

    static StateFunc    sStateFuncs[4];

public:
    RedWizzrobe();

    virtual void Update();
    virtual void Draw();

private:
    int GetState();
    void SetFacingAnimation();

    void UpdateHidden();
    void UpdateGoing();
    void UpdateVisible();
    void UpdateComing();

    void CheckRedWizzrobeCollisions();
};


class BlueWizzrobeBase : public Object
{
protected:
    uint8_t             flashTimer;
    uint8_t             turnTimer;

public:
    BlueWizzrobeBase( ObjType type, int x, int y );

protected:
    void TruncatePosition();

    void MoveOrTeleport();
    void TryShooting();

    void MoveAndCollide();
    void Move();
    void Turn();
    void TurnIfNeeded();
    void TryTeleporting();
};


class BlueWizzrobe : public BlueWizzrobeBase
{
    SpriteAnimator      animator;

public:
    BlueWizzrobe( int x, int y );

    virtual void Update();
    virtual void Draw();

private:
    void SetFacingAnimation();

    void AnimateAndCheckCollisions();
};


class Lamnola : public Object
{
    enum
    {
        HeadSlot1   = MonsterSlot1 + 4,
        HeadSlot2   = HeadSlot1 + 5,
        TailSlot1   = MonsterSlot1,
        TailSlot2   = TailSlot1 + 5
    };

    SpriteImage     image;

public:
    Lamnola( ObjType type, bool isHead, int x, int y );

    virtual void Update();
    virtual void Draw();

    static Object* MakeSet( ObjType type );

private:
    void UpdateHead();
    void Turn();
    void CheckLamnolaCollisions();
};


class Wallmaster : public Object
{
    SpriteAnimator  animator;

    int             state;
    int             dirIndex;
    int             tilesCrossed;
    bool            holdingPlayer;

public:
    Wallmaster();

    virtual void Update();
    virtual void Draw();

private:
    void CalcStartPosition( 
        int playerOrthoCoord, int playerCoord, int dir, 
        int baseDirIndex, int leastCoord, int& orthoCoord, int& coordIndex );

    void UpdateIdle();
    void UpdateMoving();
};


class Aquamentus : public Object
{
    SpriteAnimator  animator;
    SpriteImage     mouthImage;

    int             distance;
    int8_t          fireballOffsets[MaxMonsters];

public:
    Aquamentus();

    virtual void Update();
    virtual void Draw();

private:
    void Move();
    void TryShooting();
    void Animate();
};


class Dodongo : public Wanderer
{
    typedef void (Dodongo::*StateFunc)();

    int                 state;
    int                 bloatedSubstate;
    int                 bloatedTimer;
    int                 bombHits;

    static StateFunc    sStateFuncs[3];
    static StateFunc    sBloatedSubstateFuncs[5];

public:
    Dodongo( int x, int y );

    virtual void Update();
    virtual void Draw();

private:
    void UpdateState();
    void CheckPlayerHit();
    void CheckBombHit();
    void Animate();

    void UpdateMoveState();
    void UpdateBloatedState();
    void UpdateStunnedState();

    void SetWalkAnimation();
    void SetBloatAnimation();

    bool Overlaps( int xDist, int yDist, int boundsIndex );
    void CheckTickingBombHit( Bomb* bomb, int xDist, int yDist );
    void CheckPlayerHitStdSize();

    void UpdateBloatedWait();
    void UpdateBloatedDie();
    void UpdateBloatedEnd();
};


class Manhandla : public Object
{
    SpriteAnimator  animator;

    uint16_t        curSpeedFix;
    uint16_t        speedAccum;
    uint16_t        frameAccum;
    int             frame;
    int             oldFrame;

    static int          sPartsDied;
    static Direction    sFacingAtFrameBegin;
    static Direction    sBounceDir;

public:
    static Object* Make( int x, int y );

    virtual void Update();
    virtual void Draw();

    static void ClearRoomData();

private:
    Manhandla( int index, int x, int y, Direction facing );

    void SetPartFacings( Direction dir );
    void UpdateBody();
    void Move();
    void TryShooting();
    void CheckManhandlaCollisions();
};


class DigdoggerBase : public Object
{
protected:
    uint16_t        curSpeedFix;
    uint16_t        speedAccum;
    uint16_t        targetSpeedFix;
    uint16_t        accelDir;
    bool            isChild;

protected:
    DigdoggerBase( ObjType type, int x, int y );

    void UpdateMove();

private:
    void Move();
    void Accelerate();

    void IncreaseSpeed();
    void DecreaseSpeed();
};


class Digdogger : public DigdoggerBase
{
    SpriteAnimator  animator;
    SpriteAnimator  littleAnimator;

    int             childCount;
    bool            updateBig;

public:
    static Object* Make( int x, int y, int childCount );

    virtual void Update();
    virtual void Draw();

private:
    Digdogger( int x, int y, int childCount );

    void UpdateSplit();
};


class DigdoggerChild : public DigdoggerBase
{
    SpriteAnimator  animator;

public:
    static Object* Make( int x, int y );

    virtual void Update();
    virtual void Draw();

private:
    DigdoggerChild( int x, int y );
};


class Gohma : public Object
{
    SpriteAnimator  animator;
    SpriteAnimator  leftAnimator;
    SpriteAnimator  rightAnimator;

    bool            changeFacing;
    uint16_t        speedAccum;
    int             distance;
    int             sprints;
    int             startOpenEyeTimer;
    int             eyeOpenTimer;
    int             eyeClosedTimer;
    int             shootTimer;
    int             frame;
    int             curCheckPart;

public:
    Gohma( ObjType type );

    virtual void Update();
    virtual void Draw();

    int GetCurrentCheckPart();
    int GetEyeFrame();

private:
    void ChangeFacing();
    void Move();
    void AnimateEye();
    void TryShooting();
    void CheckGohmaCollisions();
};


class GleeokHead : public Flyer
{
public:
    GleeokHead( int x, int y );

    virtual void Update();

protected:
    virtual void UpdateFullSpeedImpl();
};


class GleeokNeck
{
public:
    static const int MaxParts = 5;
    static const int HeadIndex = MaxParts - 1;
    static const int ShooterIndex = HeadIndex;

private:
    typedef void (GleeokNeck::*Func)( int index );

    struct Limits
    {
        int     values[3];
    };

    struct Part
    {
        int     x;
        int     y;
    };

    Part        parts[MaxParts];
    SpriteImage neckImage;
    SpriteImage headImage;

    int         startHeadTimer;
    int         xSpeed;
    int         ySpeed;
    int         changeXDirTimer;
    int         changeYDirTimer;
    int         changeDirsTimer;
    bool        isAlive;
    int         hp;

public:
    bool IsAlive();
    void SetDead();
    int  GetHP();
    void SetHP( int value );
    Point GetPartLocation( int partIndex );

    void Init( int index );
    void Update();
    void Draw();

private:
    void TryShooting();
    void MoveHead();
    void MoveNeck();

    void GetLimits( int distance, Limits& limits );
    void Stretch( int index, const Limits& xLimits, const Limits& yLimits );

    void CrossedNoLimits( int index );
    void CrossedLowLimit( int index );
    void CrossedMidYLimit( int index );
    void CrossedMidXLimit( int index );
    void CrossedBothMidLimits( int index );
};


class Gleeok : public Object
{
    enum
    {
        MaxNecks = 4,
        NormalAnimFrames = 17 * 4,
        WrithingAnimFrames = 7 * 4,
        TotalWrithingFrames = 7 * 7,
    };

    SpriteAnimator  animator;
    int             writhingTimer;
    int             neckCount;
    GleeokNeck      necks[MaxNecks];

public:
    Gleeok( ObjType type, int x, int y );

    virtual void Update();
    virtual void Draw();

private:
    void Animate();
    void CheckNeckCollisions( int index );
};


class Ganon : public BlueWizzrobeBase
{
    enum Visual
    {
        Visual_None,
        Visual_Ganon   = 1,
        Visual_Pile    = 2,
        Visual_Pieces  = 4,
    };

    int             visual;
    int             state;
    uint8_t         lastHitTimer;
    int             dyingTimer;
    int             frame;

    int             cloudDist;
    int             sparksX[8];
    int             sparksY[8];
    Direction       piecesDir[8];

    SpriteAnimator  animator;
    SpriteAnimator  cloudAnimator;
    SpriteImage     pileImage;

public:
    Ganon( int x, int y );

    virtual void Update();
    virtual void Draw();

private:
    void UpdateHoldDark();
    void UpdateHoldLight();
    void UpdateActive();

    void UpdateDying();
    void UpdateLastHit();
    void UpdateMoveAndShoot();
    void CheckCollision();
    void MoveAround();
    void MakePieces();
    void MovePieces();

    void SetBossPalette( const uint8_t* palette );
    void ResetPosition();
};


class Zelda : public Object
{
    int         state;
    SpriteImage image;

public:
    Zelda();

    virtual void Update();
    virtual void Draw();

    static Object* Make();
};


class StandingFire : public Object
{
    SpriteAnimator  animator;

public:
    StandingFire( int x, int y );

    virtual void Update();
    virtual void Draw();
};


class GuardFire : public Object
{
    SpriteAnimator  animator;

public:
    GuardFire( int x, int y );

    virtual void Update();
    virtual void Draw();
};


class RupeeStash : public Object
{
public:
    RupeeStash( int x, int y );

    virtual void Update();
    virtual void Draw();

    static Object* Make();
};


class Fairy : public Flyer
{
    int             timer;

public:
    Fairy( int x, int y );

    virtual void Update();

protected:
    virtual void UpdateFullSpeedImpl();
    virtual int GetFrame();

private:
    bool TouchesObject( Object* obj );
};


class PondFairy : public Object
{
    enum State
    {
        Idle,
        Healing,
        Healed,
    };

    State           state;
    SpriteAnimator  animator;
    uint8_t         heartState[8];
    uint8_t         heartAngle[8];

public:
    PondFairy();

    virtual void Update();
    virtual void Draw();

private:
    void UpdateIdle();
    void UpdateHealing();
};


class DeadDummy : public Object
{
public:
    DeadDummy( int x, int y );

    virtual void Update();
    virtual void Draw();
};


class Statues
{
    enum
    {
        Patterns = 3,
        MaxStatues = 4,
    };

    static uint8_t timers[MaxStatues];

public:
    static void Init();
    static void Update( int pattern );
};


Object* MakeMonster( ObjType type, int x, int y );
void ClearRoomMonsterData();
