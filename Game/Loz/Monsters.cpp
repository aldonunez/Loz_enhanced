/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Monsters.h"
#include "World.h"
#include "Graphics.h"
#include "ObjType.h"
#include "OWNpcsAnim.h"
#include "UWNpcsAnim.h"
#include "UWBossAnim.h"
#include "PlayerItemsAnim.h"
#include "ItemObj.h"
#include "Player.h"
#include "Profile.h"
#include "Sound.h"
#include "SoundId.h"


struct WalkerSpec
{
    const uint8_t*  animMap;
    int             animTime;
    int             palette;
    int             speed;
    ObjType         shotType;
};


const int PlayerPal             = 4;
const int BluePal               = 5;
const int RedPal                = 6;
const int SeaPal                = 7;
const int StdSpeed              = 0x20;
const int FastSpeed             = 0x40;
const ObjType ShotFromLynel     = Obj_PlayerSwordShot;
const ObjType ShotFromMoblin    = Obj_Arrow;
const ObjType ShotFromOctorock  = Obj_FlyingRock;

const int DirCount = 4;

static const uint8_t    armosAnimMap[DirCount] = 
{
    Anim_OW_Armos_Right,
    Anim_OW_Armos_Left,
    Anim_OW_Armos_Down,
    Anim_OW_Armos_Up
};

static const uint8_t    lynelAnimMap[DirCount] = 
{
    Anim_OW_Lynel_Right,
    Anim_OW_Lynel_Left,
    Anim_OW_Lynel_Down,
    Anim_OW_Lynel_Up,
};

static const uint8_t    moblinAnimMap[DirCount] = 
{
    Anim_OW_Moblin_Right,
    Anim_OW_Moblin_Left,
    Anim_OW_Moblin_Down,
    Anim_OW_Moblin_Up,
};

static const uint8_t    octorockAnimMap[DirCount] = 
{
    Anim_OW_Octorock_Right,
    Anim_OW_Octorock_Left,
    Anim_OW_Octorock_Down,
    Anim_OW_Octorock_Up,
};

static const uint8_t    ghiniAnimMap[DirCount] = 
{
    Anim_OW_Ghini_Right,
    Anim_OW_Ghini_Left,
    Anim_OW_Ghini_Left,
    Anim_OW_Ghini_UpRight,
};

static const uint8_t    stalfosAnimMap[DirCount] = 
{
    Anim_UW_Stalfos,
    Anim_UW_Stalfos,
    Anim_UW_Stalfos,
    Anim_UW_Stalfos,
};

static const uint8_t    goriyaAnimMap[DirCount] = 
{
    Anim_UW_Goriya_Right,
    Anim_UW_Goriya_Left,
    Anim_UW_Goriya_Down,
    Anim_UW_Goriya_Up
};

static const uint8_t    darknutAnimMap[DirCount] = 
{
    Anim_UW_Darknut_Right,
    Anim_UW_Darknut_Left,
    Anim_UW_Darknut_Down,
    Anim_UW_Darknut_Up
};

static const uint8_t    gibdoAnimMap[DirCount] = 
{
    Anim_UW_Gibdo,
    Anim_UW_Gibdo,
    Anim_UW_Gibdo,
    Anim_UW_Gibdo
};


static const WalkerSpec armosSpec = 
{
    armosAnimMap,
    12,
    RedPal,
    StdSpeed,
    Obj_None
};

static const WalkerSpec blueLynelSpec = 
{
    lynelAnimMap,
    12,
    BluePal,
    StdSpeed,
    ShotFromLynel
};

static const WalkerSpec redLynelSpec = 
{
    lynelAnimMap,
    12,
    RedPal,
    StdSpeed,
    ShotFromLynel
};

static const WalkerSpec blueMoblinSpec = 
{
    moblinAnimMap,
    12,
    7,
    StdSpeed,
    ShotFromMoblin
};

static const WalkerSpec redMoblinSpec = 
{
    moblinAnimMap,
    12,
    RedPal,
    StdSpeed,
    ShotFromMoblin
};

static const WalkerSpec blueSlowOctorockSpec = 
{
    octorockAnimMap,
    12,
    BluePal,
    StdSpeed,
    ShotFromOctorock
};

static const WalkerSpec blueFastOctorockSpec = 
{
    octorockAnimMap,
    12,
    BluePal,
    FastSpeed,
    ShotFromOctorock
};

static const WalkerSpec redSlowOctorockSpec = 
{
    octorockAnimMap,
    12,
    RedPal,
    StdSpeed,
    ShotFromOctorock
};

static const WalkerSpec redFastOctorockSpec = 
{
    octorockAnimMap,
    12,
    RedPal,
    FastSpeed,
    ShotFromOctorock
};

static const WalkerSpec stalfosSpec = 
{
    stalfosAnimMap,
    16,
    RedPal,
    StdSpeed,
    Obj_PlayerSwordShot,
};

static const WalkerSpec blueGoriyaSpec = 
{
    goriyaAnimMap,
    12,
    BluePal,
    StdSpeed,
    Obj_None
};

static const WalkerSpec redGoriyaSpec = 
{
    goriyaAnimMap,
    12,
    RedPal,
    StdSpeed,
    Obj_None
};

static const WalkerSpec redDarknutSpec = 
{
    darknutAnimMap,
    16,
    RedPal,
    StdSpeed,
    Obj_None
};

static const WalkerSpec blueDarknutSpec = 
{
    darknutAnimMap,
    16,
    BluePal,
    0x28,
    Obj_None
};

static const WalkerSpec gibdoSpec = 
{
    gibdoAnimMap,
    16,
    BluePal,
    StdSpeed,
    Obj_None
};


static Direction GetXDirToPlayer( int x )
{
    Point   playerPos = World::GetObservedPlayerPos();

    if ( playerPos.X < x )
        return Dir_Left;
    else
        return Dir_Right;
}

static Direction GetYDirToPlayer( int y )
{
    Point   playerPos = World::GetObservedPlayerPos();

    if ( playerPos.Y < y )
        return Dir_Up;
    else
        return Dir_Down;
}

static Direction GetXDirToTruePlayer( int x )
{
    Player* player = World::GetPlayer();

    if ( (int) player->GetX() < x )
        return Dir_Left;
    else
        return Dir_Right;
}

static Direction GetYDirToTruePlayer( int y )
{
    Player* player = World::GetPlayer();

    if ( (int) player->GetY() < y )
        return Dir_Up;
    else
        return Dir_Down;
}

static Direction GetDir8ToPlayer( int x, int y )
{
    Point   playerPos = World::GetObservedPlayerPos();
    int     dir = 0;

    if ( playerPos.Y < y )
        dir |= Dir_Up;
    else if ( playerPos.Y > y )
        dir |= Dir_Down;

    if ( playerPos.X < x )
        dir |= Dir_Left;
    else if ( playerPos.X > x )
        dir |= Dir_Right;

    return (Direction) dir;
}

static Direction TurnTowardsPlayer8( int x, int y, Direction facing )
{
    Direction   dirToPlayer = GetDir8ToPlayer( x, y );
    uint32_t    dirIndex = Util::GetDirection8Ord( facing );

    dirIndex = (dirIndex + 1) % 8;

    for ( int i = 0; i < 3; i++ )
    {
        if ( Util::GetDirection8( dirIndex ) == dirToPlayer )
            return facing;
        dirIndex = (dirIndex - 1) % 8;
    }

    dirIndex = (dirIndex + 1) % 8;

    for ( int i = 0; i < 3; i++ )
    {
        Direction dir = Util::GetDirection8( dirIndex );
        if ( (dir & dirToPlayer) != 0 )
        {
            if ( (dir | dirToPlayer) < 7 )
                return dir;
        }
        dirIndex = (dirIndex + 1) % 8;
    }

    dirIndex = (dirIndex - 1) % 8;
    return Util::GetDirection8( dirIndex );
}

static Direction TurnRandomly8( Direction facing )
{
    int r = Util::GetRandom( 256 );

    if ( r >= 0xA0 )
        ;   // keep going in the same direction
    else if ( r >= 0x50 )
        facing = Util::GetNextDirection8( facing );
    else
        facing = Util::GetPrevDirection8( facing );

    return facing;
}

static int Shoot( ObjType shotType, int objX, int objY, Direction facing )
{
    int slot = World::FindEmptyMonsterSlot();
    if ( slot < 0 )
        return -1;

    Object* shot = nullptr;
    int thisSlot = World::GetCurrentObjectSlot();
    int oldActiveShots = World::GetActiveShots();

    if ( shotType == Obj_Boomerang )
    {
        shot = MakeBoomerang( objX, objY, facing, 0x51, 2.5, thisSlot, slot );
    }
    else
    {
        shot = MakeProjectile( shotType, objX, objY, facing, slot );
    }

    if ( shot == nullptr )
        return -1;

    int newActiveShots = World::GetActiveShots();
    if ( oldActiveShots != newActiveShots && newActiveShots > 4 )
    {
        delete shot;
        return -1;
    }

    World::SetObject( slot, shot );
    // In the original, they start in state $10. But, that was only a way to say that the object exists.
    shot->SetObjectTimer( 0 );
    return slot;
}

static void InitCommonFacing( int x, int y, Direction& facing )
{
    if ( facing != Dir_None )
        return;

    Point playerPos = World::GetObservedPlayerPos();
    // Why did the original game test these distances as unsigned?
    uint32_t xDist = playerPos.X - x;
    uint32_t yDist = playerPos.Y - y;

    if ( xDist <= yDist )
    {
        // Why is this away from the player, while for Y it's toward the player?
        if ( playerPos.X > x )
            facing = Dir_Left;
        else
            facing = Dir_Right;
    }
    else
    {
        if ( playerPos.Y > y )
            facing = Dir_Down;
        else
            facing = Dir_Up;
    }
}

static void InitCommonStateTimer( uint8_t& stateTimer )
{
    int t = World::GetCurrentObjectSlot();
    t = (t + 2) * 16;
    stateTimer = t;
}

static void ShootFireball( ObjType type, int x, int y )
{
    int newSlot = World::FindEmptyMonsterSlot();
    if ( newSlot >= 0 )
    {
        Fireball* fireball = new Fireball( type, x + 4, y, 1.75 );
        World::SetObject( newSlot, fireball );
    }
}


//----------------------------------------------------------------------------
//  Walker
//----------------------------------------------------------------------------

Walker::Walker( ObjType type, const WalkerSpec* spec, int x, int y )
    :   Object( type ),
        curSpeed( spec->speed ),
        shootTimer( 0 ),
        wantToShoot( false ),
        spec( spec )
{
    animator.time = 0;
    animator.durationFrames = spec->animTime;

    objX = x;
    objY = y;
}

void Walker::Draw()
{
    SetFacingAnimation();
    int offsetX = (16 - animator.anim->width) / 2;
    int pal = CalcPalette( spec->palette );
    animator.Draw( Sheet_Npcs, objX + offsetX, objY, pal );
}

void Walker::SetSpec( const WalkerSpec* spec )
{
    this->spec = spec;
    curSpeed = spec->speed;
    animator.SetDuration( spec->animTime );
    SetFacingAnimation();
}

void Walker::SetFacingAnimation()
{
    int dirOrd = Util::GetDirectionOrd( facing );
    if ( spec->animMap != nullptr )
        animator.anim = Graphics::GetAnimation( Sheet_Npcs, spec->animMap[dirOrd] );
    else
        animator.anim = nullptr;
}

void Walker::TryShooting()
{
    if ( spec->shotType == Obj_None )
        return;

    bool isBlue = GetType() == Obj_BlueFastOctorock || GetType() == Obj_BlueSlowOctorock 
        || GetType() == Obj_BlueMoblin || GetType() == Obj_BlueLynel;

    if ( isBlue || shootTimer != 0 || (Util::GetRandom( 256 ) >= 0xF8) )
    {
        if ( invincibilityTimer > 0 )
        {
            shootTimer = 0;
        }
        else
        {
            if ( shootTimer > 0 )
                shootTimer--;
            else if ( wantToShoot )
                shootTimer = 0x30;
        }

        if ( shootTimer == 0 )
        {
            curSpeed = spec->speed;
            return;
        }

        if ( shootTimer != 0x10 || IsStunned() )
        {
            curSpeed = 0;
            return;
        }

        int slot = Shoot( spec->shotType );
        if ( slot >= 0 )
        {
            // The original sets state timer to $80 and decrements $437. See $04:8837.
            // But, I haven't found any code that depends on them.
            objTimer = 0x80;
            curSpeed = 0;
            wantToShoot = false;
        }
        else
        {
            curSpeed = spec->speed;
        }
    }
}

int Walker::Shoot( ObjType shotType )
{
    if ( !wantToShoot )
        return -1;

    return ::Shoot( shotType, objX, objY, facing );
}

bool Walker::TryBigShove()
{
    if ( tileOffset == 0 )
    {
        if ( World::CollidesWithTileMoving( objX, objY, facing, false ) )
            return false;
    }

    if ( CheckWorldMargin( facing ) == Dir_None )
        return false;

    ObjMoveDir( 0xFF, facing );

    if ( (tileOffset & 0xF) == 0 )
        tileOffset &= 0xF;

    return true;
}


//----------------------------------------------------------------------------
//  ChaseWalker
//----------------------------------------------------------------------------

ChaseWalker::ChaseWalker( ObjType type, const WalkerSpec* spec, int x, int y )
    :   Walker( type, spec, x, y )
{
}

void ChaseWalker::Update()
{
    animator.Advance();
    UpdateNoAnimation();
}

void ChaseWalker::UpdateNoAnimation()
{
    ObjMove( curSpeed );
    TargetPlayer();
    TryShooting();
}

void ChaseWalker::TargetPlayer()
{
    if ( shoveDir != 0 )
        return;

    if ( curSpeed == 0 || (tileOffset & 0xF) != 0 )
    {
        moving = facing;
        return;
    }

    tileOffset &= 0xF;

    // ORIGINAL: If player.state = $FF, then skip all this, go to the end (moving := facing).
    //           But, I don't see how the player can get in that state.

    Point observedPos = World::GetObservedPlayerPos();
    int xDiff = abs( objX - observedPos.X );
    int yDiff = abs( objY - observedPos.Y );
    int maxDiff;
    Direction dir;

    if ( yDiff >= xDiff )
    {
        maxDiff = yDiff;
        dir = (objY > observedPos.Y) ? Dir_Up : Dir_Down;
    }
    else
    {
        maxDiff = xDiff;
        dir = (objX > observedPos.X) ? Dir_Left : Dir_Right;
    }

    if ( maxDiff < 0x51 )
    {
        wantToShoot = true;
        facing = dir;
    }
    else
    {
        wantToShoot = false;
    }

    moving = facing;
}


//----------------------------------------------------------------------------
//  StdChaseWalker
//----------------------------------------------------------------------------

StdChaseWalker::StdChaseWalker( ObjType type, const WalkerSpec* spec, int x, int y )
    :   ChaseWalker( type, spec, x, y )
{
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}


//----------------------------------------------------------------------------
//  Goriya
//----------------------------------------------------------------------------

Goriya::Goriya( ObjType type, const WalkerSpec* spec, int x, int y )
    :   ChaseWalker( type, spec, x, y ),
        shotSlot( -1 )
{
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}

void* Goriya::GetInterface( ObjInterfaces iface )
{
    if ( iface == ObjItf_IThrower )
        return (IThrower*) this;
    return nullptr;
}

void Goriya::Catch()
{
    shotSlot = -1;

    int r = Util::GetRandom( 256 );
    int t;

    if ( r < 0x30 )
        t = 0x30;
    else if ( r < 0x70 )
        t = 0x50;
    else
        t = 0x70;

    objTimer = t;
}

void Goriya::Update()
{
    animator.Advance();

    if ( shotSlot >= 0 )
        return;

    ObjMove( curSpeed );
    if ( shoveDir != 0 )
        return;
    TargetPlayer();
    TryThrowingBoomerang();
}

void Goriya::TryThrowingBoomerang()
{
    if ( objTimer != 0 )
        return;

    if ( GetType() == Obj_RedGoriya )
    {
        int r = Util::GetRandom( 256 );
        if ( r != 0x23 && r != 0x77 )
            return;
    }

    if ( World::GetItem( ItemSlot_Clock ) != 0 )
        return;

    int slot = Shoot( Obj_Boomerang );
    if ( slot >= 0 )
    {
        wantToShoot = false;
        shotSlot = slot;
        objTimer = Util::GetRandom( 0x40 );
    }
}


//----------------------------------------------------------------------------
//  Armos
//----------------------------------------------------------------------------

Armos::Armos( int x, int y )
    :   ChaseWalker( Obj_Armos, &armosSpec, x, y ),
        state( 0 )
{
    decoration = 0;
    facing = Dir_Down;
    SetFacingAnimation();

    // Set this to make up for the fact that this armos begins completely aligned with tile.
    tileOffset = 3;

    int r = Util::GetRandom( 2 );
    if ( r == 0 )
        curSpeed = 0x20;
    else
        curSpeed = 0x60;
}

void Armos::Update()
{
    int slot = World::GetCurrentObjectSlot();

    if ( state == 0 )
    {
        // ORIGINAL: Can hit the player, but not get hit.
        if ( objTimer == 0 )
        {
            state++;
            World::OnActivatedArmos( objX, objY );
        }
    }
    else
    {
        ChaseWalker::UpdateNoAnimation();

        if ( shoveDir == 0 )
        {
            animator.Advance();
            CheckCollisions();
            if ( decoration != 0 )
            {
                DeadDummy* dummy = new DeadDummy( objX, objY );
                World::SetObject( slot, dummy );
                dummy->SetDecoration( decoration );
            }
        }
    }
}

void Armos::Draw()
{
    if ( state == 0 )
    {
        if ( (objTimer & 1) == 1 )
            ChaseWalker::Draw();
    }
    else
    {
        ChaseWalker::Draw();
    }
}


//----------------------------------------------------------------------------
//  Wanderer
//----------------------------------------------------------------------------

Wanderer::Wanderer( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y )
    :   Walker( type, spec, x, y ),
        turnTimer( 0 ),
        turnRate( turnRate )
{
}

void Wanderer::Update()
{
    animator.Advance();
    Move();
    TryShooting();
    CheckCollisions();
}

void Wanderer::Move()
{
    ObjMove( curSpeed );
    TargetPlayer();
}

void Wanderer::MoveIfNeeded()
{
    if ( shoveDir != 0 )
    {
        ObjShove();
        return;
    }

    if ( IsStunned() )
        return;

    ObjMove( curSpeed );
    TargetPlayer();
}

void Wanderer::TargetPlayer()
{
    if ( turnTimer > 0 )
        turnTimer--;

    if ( shoveDir != 0 )
        return;

    if ( curSpeed == 0 || (tileOffset & 0xF) != 0 )
    {
        moving = facing;
        return;
    }

    tileOffset &= 0xF;

    int r = Util::GetRandom( 256 );

    // ORIGINAL: If (r > turnRate) or (player.state = $FF), then ...
    //           But, I don't see how the player can get in that state.

    if ( r > turnRate )
    {
        TurnIfTime();
    }
    else
    {
        Point   playerPos = World::GetObservedPlayerPos();

        if ( abs( objX - playerPos.X ) < 9 )
            TurnY();
        else if ( abs( objY - playerPos.Y ) < 9 )
            TurnX();
        else
            TurnIfTime();
    }

    moving = facing;
}

void Wanderer::TurnIfTime()
{
    wantToShoot = false;

    if ( turnTimer != 0 )
        return;

    if ( Util::IsVertical( facing ) )
        TurnX();
    else
        TurnY();
}

void Wanderer::TurnX()
{
    facing = GetXDirToPlayer( objX );
    turnTimer = Util::GetRandom( 256 );
    wantToShoot = true;
}

void Wanderer::TurnY()
{
    facing = GetYDirToPlayer( objY );
    turnTimer = Util::GetRandom( 256 );
    wantToShoot = true;
}


//----------------------------------------------------------------------------
//  StdWanderer
//----------------------------------------------------------------------------

StdWanderer::StdWanderer( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y )
    :   Wanderer( type, spec, turnRate, x, y )
{
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}


//----------------------------------------------------------------------------
//  DelayedWanderer
//----------------------------------------------------------------------------

DelayedWanderer::DelayedWanderer( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y )
    :   Wanderer( type, spec, turnRate, x, y )
{
    InitCommonFacing( objX, objY, facing );
    InitCommonStateTimer( objTimer );
    SetFacingAnimation();

    if ( type == Obj_BlueFastOctorock || type == Obj_RedFastOctorock )
        curSpeed = 0x30;
}


//----------------------------------------------------------------------------
//  Ghini
//----------------------------------------------------------------------------

static const WalkerSpec ghiniSpec = 
{
    ghiniAnimMap,
    12,
    BluePal,
    StdSpeed,
    Obj_None
};


Ghini::Ghini( int x, int y )
    :   Wanderer( Obj_Ghini, &ghiniSpec, 0xFF, x, y )
{
    InitCommonFacing( objX, objY, facing );
    InitCommonStateTimer( objTimer );
    SetFacingAnimation();
}

void Ghini::Update()
{
    animator.Advance();
    MoveIfNeeded();
    CheckCollisions();

    if ( decoration != 0 )
    {
        for ( int i = MonsterSlot1; i < MonsterSlotEnd; i++ )
        {
            Object* obj = World::GetObject( i );
            if ( obj != nullptr && obj->GetType() == Obj_FlyingGhini )
                obj->SetDecoration( 0x11 );
        }
    }
}


//----------------------------------------------------------------------------
//  Gibdo
//----------------------------------------------------------------------------

Gibdo::Gibdo( int x, int y )
    :   StdWanderer( Obj_Gibdo, &gibdoSpec, 0x80, x, y )
{
}

void Gibdo::Update()
{
    MoveIfNeeded();
    CheckCollisions();
    animator.Advance();
}


//----------------------------------------------------------------------------
//  Darknut
//----------------------------------------------------------------------------

Darknut* Darknut::MakeRedDarknut( int x, int y )
{
    return new Darknut( Obj_RedDarknut, &redDarknutSpec, 0x80, x, y );
}

Darknut* Darknut::MakeBlueDarknut( int x, int y )
{
    return new Darknut( Obj_BlueDarknut, &blueDarknutSpec, 0x80, x, y );
}

Darknut::Darknut( ObjType type, const WalkerSpec* spec, int turnRate, int x, int y )
    :   StdWanderer( type, spec, turnRate, x, y )
{
    invincibilityMask = 0xF6;
}

void Darknut::Update()
{
    MoveIfNeeded();
    CheckCollisions();
    stunTimer = 0;
    animator.Advance();
}


//----------------------------------------------------------------------------
//  Stalfos
//----------------------------------------------------------------------------

Stalfos::Stalfos( int x, int y )
    :   StdWanderer( Obj_Stalfos, &stalfosSpec, 0x80, x, y )
{
}

void Stalfos::Update()
{
    MoveIfNeeded();
    CheckCollisions();
    animator.Advance();

    if ( World::GetProfile().Quest == 1 )
    {
        TryShooting();
    }
}


//----------------------------------------------------------------------------
//  Gel
//----------------------------------------------------------------------------

static const int gelWaitTimes[] = { 0x08, 0x18, 0x28, 0x38 };

static const uint8_t    gelAnimMap[DirCount] = 
{
    Anim_UW_Gel,
    Anim_UW_Gel,
    Anim_UW_Gel,
    Anim_UW_Gel,
};

static const WalkerSpec gelSpec = 
{
    gelAnimMap,
    4,
    SeaPal,
    0x40,
    Obj_None
};

Gel::Gel( ObjType type, int x, int y, Direction dir, int fraction )
    :   Wanderer( type, &gelSpec, 0x20, x, y ),
        state( 0 )
{
    facing = dir;

    if ( type == Obj_Gel )
        state = 2;
    else
        this->fraction = fraction;

    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}

void Gel::Update()
{
    switch ( state )
    {
    case 0:
        objTimer = 5;
        state = 1;
        break;

    case 1:
        UpdateShove();
        break;

    case 2:
        UpdateWander();
        break;
    }

    CheckCollisions();
    animator.Advance();
}

void Gel::UpdateShove()
{
    if ( objTimer != 0 )
    {
        if ( TryBigShove() )
            return;
    }
    objX = (objX + 8) & 0xF0;
    objY = (objY + 8) & 0xF0;
    objY |= 0xD;
    tileOffset = 0;
    state = 2;
}

void Gel::UpdateWander()
{
    if ( objTimer < 5 )
    {
        Move();

        if ( objTimer == 0 && tileOffset == 0 )
        {
            int index = Util::GetRandom( 4 );
            objTimer = gelWaitTimes[index];
        }
    }
}


//----------------------------------------------------------------------------
//  Zol
//----------------------------------------------------------------------------

static const int zolWaitTimes[] = { 0x18, 0x28, 0x38, 0x48 };

static const uint8_t    zolAnimMap[DirCount] = 
{
    Anim_UW_Zol,
    Anim_UW_Zol,
    Anim_UW_Zol,
    Anim_UW_Zol,
};

static const WalkerSpec zolSpec = 
{
    zolAnimMap,
    16,
    SeaPal,
    0x18,
    Obj_None
};

Zol::Zol( int x, int y )
    :   Wanderer( Obj_Zol, &zolSpec, 0x20, x, y ),
        state( 0 )
{
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}

void Zol::Update()
{
    switch ( state )
    {
    case 0: UpdateWander(); break;
    case 1: UpdateShove(); break;
    case 2: UpdateSplit(); break;
    }

    animator.Advance();
}

void Zol::UpdateWander()
{
    if ( objTimer < 5 )
    {
        Move();

        if ( objTimer == 0 && tileOffset == 0 )
        {
            int index = Util::GetRandom( 4 );
            objTimer = zolWaitTimes[index];
        }
    }

    // Above is almost the same as Gel::UpdateWander.

    CheckCollisions();

    if ( decoration == 0 && invincibilityTimer != 0 )
    {
        // On collision , go to state 2 or 1, depending on alignment.

        const uint AlignedY = 0xD;

        Player* player = World::GetPlayer();
        uint    dirMask = 0;

        if ( (objY & 0xF) == AlignedY )
            dirMask |= 3;

        if ( (objX & 0xF) == 0 )
            dirMask |= 0xC;

        if ( (dirMask & player->GetFacing()) == 0 )
            state = 2;
        else
            state = 1;
    }
}

void Zol::UpdateShove()
{
    if ( !TryBigShove() )
        state = 2;
}

void Zol::UpdateSplit()
{
    isDeleted = true;
    World::Get()->SetRoomObjCount( World::Get()->GetRoomObjCount() + 1 );

    const static Direction sHDirs[] = { Dir_Right, Dir_Left };
    const static Direction sVDirs[] = { Dir_Down, Dir_Up };
    const Direction*  orthoDirs;

    if ( Util::IsHorizontal( facing ) )
        orthoDirs = sVDirs;
    else
        orthoDirs = sHDirs;

    for ( int i = 0; i < 2; i++ )
    {
        int slot = World::FindEmptyMonsterSlot();
        if ( slot < 0 )
            break;

        Gel* gel = new Gel( Obj_ChildGel, objX, objY, orthoDirs[i], fraction );
        World::SetObject( slot, gel );
        gel->SetObjectTimer( 0 );
    }
}


//----------------------------------------------------------------------------
//  Bubble
//----------------------------------------------------------------------------

static const uint8_t    bubbleAnimMap[DirCount] = 
{
    Anim_UW_Bubble,
    Anim_UW_Bubble,
    Anim_UW_Bubble,
    Anim_UW_Bubble
};

static const WalkerSpec bubbleSpec = 
{
    bubbleAnimMap,
    2,
    BluePal,
    FastSpeed,
    Obj_None
};

Bubble::Bubble( ObjType type, int x, int y )
    :   Wanderer( type, &bubbleSpec, 0x40, x, y )
{
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}


void Bubble::Update()
{
    MoveIfNeeded();

    if ( CheckPlayerCollision() )
    {
        if ( GetType() == Obj_Bubble1 )
            World::SetStunTimer( NoSwordTimerSlot, 0x10 );
        else
            World::SetSwordBlocked( GetType() == Obj_Bubble3 );

        // The sword blocked state is cleared by touching blue bubbles (Bubble2) 
        // and by refilling all hearts with the potion or pond fairy.
    }
}

void Bubble::Draw()
{
    uint pal = 4;

    if ( GetType() == Obj_Bubble1 )
        pal += GetFrameCounter() % 4;
    else
        pal += (GetType() - Obj_Bubble1);

    animator.Draw( Sheet_Npcs, objX, objY, pal );
}


//----------------------------------------------------------------------------
//  Vire
//----------------------------------------------------------------------------

static const int vireOffsetY[] = { 0, -3, -2, -1, -1, 0, -1, 0, 0, 1, 0, 1, 1, 2, 3, 0 };

static const uint8_t    vireAnimMap[DirCount] = 
{
    Anim_UW_Vire_Down,
    Anim_UW_Vire_Down,
    Anim_UW_Vire_Down,
    Anim_UW_Vire_Up,
};

static const WalkerSpec vireSpec = 
{
    vireAnimMap,
    20,
    BluePal,
    StdSpeed,
    Obj_None
};

Vire::Vire( int x, int y )
    :   Wanderer( Obj_Vire, &vireSpec, 0x80, x, y ),
        state( 0 )
{
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}

void Vire::Update()
{
    switch ( state )
    {
    case 0: UpdateWander(); break;
    case 1: UpdateShove(); break;
    default: UpdateSplit(); break;
    }

    if ( state < 2 )
    {
        animator.Advance();
    }
}

void Vire::UpdateWander()
{
    MoveIfNeeded();

    if ( !IsStunned() && Util::IsHorizontal( facing ) )
    {
        int offsetX = abs( tileOffset );
        objY += vireOffsetY[offsetX];
    }

    CheckCollisions();

    if ( decoration == 0 && invincibilityTimer != 0 )
        state = 1;
}

void Vire::UpdateShove()
{
    if ( !TryBigShove() )
        state = 2;
}

void Vire::UpdateSplit()
{
    isDeleted = true;
    World::Get()->SetRoomObjCount( World::Get()->GetRoomObjCount() + 1 );

    for ( int i = 0; i < 2; i++ )
    {
        int slot = World::FindEmptyMonsterSlot();
        if ( slot < 0 )
            break;

        Keese* keese = Keese::MakeRedKeese( objX, objY );
        World::SetObject( slot, keese );
        keese->SetFacing( GetFacing() );
        keese->SetObjectTimer( 0 );
    }
}


//----------------------------------------------------------------------------
//  LikeLike
//----------------------------------------------------------------------------

static const uint8_t    likeLikeAnimMap[DirCount] = 
{
    Anim_UW_LikeLike,
    Anim_UW_LikeLike,
    Anim_UW_LikeLike,
    Anim_UW_LikeLike,
};

static const WalkerSpec likeLikeSpec = 
{
    likeLikeAnimMap,
    24,
    RedPal,
    StdSpeed,
    Obj_None
};

LikeLike::LikeLike( int x, int y )
    :   Wanderer( Obj_LikeLike, &likeLikeSpec, 0x80, x, y ),
        framesHeld( 0 )
{
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}

void LikeLike::Update()
{
    Player* player = World::GetPlayer();

    if ( framesHeld == 0 )
    {
        MoveIfNeeded();
        animator.Advance();

        if ( CheckCollisions() )
        {
            framesHeld++;

            objX = player->GetX();
            objY = player->GetY();
            player->SetObjectTimer( 0 );
            // ORIGINAL: player.[$405] := 0  (But, what's the point?)
            player->ResetShove();
            player->SetParalyzed( true );
            animator.durationFrames = animator.anim->length * 4;
            animator.time = 0;
            objFlags.SetDrawAbovePlayer();
        }
    }
    else
    {
        int frame = animator.time / 4;
        if ( frame < 3 )
            animator.Advance();

        framesHeld++;
        if ( framesHeld >= 0x60 )
        {
            World::SetItem( ItemSlot_MagicShield, 0 );
            framesHeld = 0xC0;
        }

        CheckCollisions();

        if ( decoration != 0 )
            player->SetParalyzed( false );
    }
}


//----------------------------------------------------------------------------
//  DigWanderer
//----------------------------------------------------------------------------

static const uint8_t    leeverAnimMap[DirCount] = 
{
    Anim_OW_Leever,
    Anim_OW_Leever,
    Anim_OW_Leever,
    Anim_OW_Leever,
};

static const uint8_t    leeverHalfAnimMap[DirCount] = 
{
    Anim_OW_LeeverHalf,
    Anim_OW_LeeverHalf,
    Anim_OW_LeeverHalf,
    Anim_OW_LeeverHalf,
};

static const uint8_t    moundAnimMap[DirCount] = 
{
    Anim_OW_Mound,
    Anim_OW_Mound,
    Anim_OW_Mound,
    Anim_OW_Mound,
};

static const WalkerSpec blueLeeverHiddenSpec = 
{
    0,
    32,
    BluePal,
    0x8,
    Obj_None
};

static const WalkerSpec blueLeeverMoundSpec = 
{
    moundAnimMap,
    22,
    BluePal,
    0xA,
    Obj_None
};

static const WalkerSpec blueLeeverHalfSpec = 
{
    leeverHalfAnimMap,
    2,
    BluePal,
    0x10,
    Obj_None
};

static const WalkerSpec blueLeeverFullSpec = 
{
    leeverAnimMap,
    10,
    BluePal,
    StdSpeed,
    Obj_None
};

static const WalkerSpec*    blueLeeverSpecs[] = 
{
    &blueLeeverHiddenSpec,
    &blueLeeverMoundSpec,
    &blueLeeverHalfSpec,
    &blueLeeverFullSpec,
    &blueLeeverHalfSpec,
    &blueLeeverMoundSpec,
};

static const int    blueLeeverStateTimes[] = 
{
    0x80,
    0x20,
    0x0F,
    0xFF,
    0x10,
    0x60
};


DigWanderer::DigWanderer( ObjType type, const WalkerSpec** stateSpecs, const int* stateTimes, int x, int y )
    :   Wanderer( type, stateSpecs[0], 0xA0, x, y ),
        stateSpecs( stateSpecs ),
        stateTimes( stateTimes ),
        state( 0 )
{
    objTimer = 0;
}

void DigWanderer::Update()
{
    Move();
    UpdateDig();
}

void DigWanderer::UpdateDig()
{
    if ( objTimer == 0 )
    {
        state = (state + 1) % 6;
        objTimer = stateTimes[state];
        SetSpec( stateSpecs[state] );
    }

    animator.Advance();

    if ( state == 3 || (GetType() == Obj_Zora && (state == 2 || state == 4)) )
    {
        CheckCollisions();
    }
}

void DigWanderer::Draw()
{
    if ( state != 0 )
        Wanderer::Draw();
}


//----------------------------------------------------------------------------
//  BlueLeever
//----------------------------------------------------------------------------

BlueLeever::BlueLeever( int x, int y )
    :   DigWanderer( Obj_BlueLeever, blueLeeverSpecs, blueLeeverStateTimes, x, y )
{
    decoration = 0;
    InitCommonStateTimer( objTimer );
    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();
}


//----------------------------------------------------------------------------
//  Zora
//----------------------------------------------------------------------------

static uint8_t  zoraAnimMap[DirCount] = 
{
    Anim_OW_Zora_Down,
    Anim_OW_Zora_Down,
    Anim_OW_Zora_Down,
    Anim_OW_Zora_Up,
};

static const WalkerSpec zoraHiddenSpec = 
{
    0,
    32,
    SeaPal,
    0,
    Obj_None
};

static const WalkerSpec zoraMoundSpec = 
{
    moundAnimMap,
    22,
    SeaPal,
    0,
    Obj_None
};

static const WalkerSpec zoraHalfSpec = 
{
    zoraAnimMap,
    2,
    SeaPal,
    0,
    Obj_None
};

static const WalkerSpec zoraFullSpec = 
{
    zoraAnimMap,
    10,
    SeaPal,
    0,
    Obj_None
};

static const WalkerSpec* zoraSpecs[] = 
{
    &zoraHiddenSpec,
    &zoraMoundSpec,
    &zoraHalfSpec,
    &zoraFullSpec,
    &zoraHalfSpec,
    &zoraMoundSpec,
};

static const int zoraStateTimes[] = 
{
    2,
    0x20,
    0x0F,
    0x22,
    0x10,
    0x60
};


Zora::Zora()
    :   DigWanderer( Obj_Zora, zoraSpecs, zoraStateTimes, 0, 0 )
{
    objTimer = zoraStateTimes[0];
    decoration = 0;
}

void Zora::Update()
{
    if ( World::GetItem( ItemSlot_Clock ) != 0 )
        return;

    UpdateDig();

    if ( state == 0 )
    {
        if ( objTimer == 1 )
        {
            Player* player = World::GetPlayer();
            Point p = World::GetRandomWaterTile();

            objX = p.X * World::TileWidth;
            objY = p.Y * World::TileHeight - 3;

            if ( player->GetY() >= objY )
                facing = Dir_Down;
           else
                facing = Dir_Up;
        } 
    }
    else if ( state == 3 )
    {
        if ( objTimer == 0x20 )
            ShootFireball( Obj_Fireball, objX, objY );
    }
}


//----------------------------------------------------------------------------
//  RedLeever
//----------------------------------------------------------------------------

int RedLeever::count;

static const WalkerSpec redLeeverHiddenSpec = 
{
    0,
    32,
    RedPal,
    0,
    Obj_None
};

static const WalkerSpec redLeeverMoundSpec = 
{
    moundAnimMap,
    16,
    RedPal,
    0,
    Obj_None
};

static const WalkerSpec redLeeverHalfSpec = 
{
    leeverHalfAnimMap,
    16,
    RedPal,
    0,
    Obj_None
};

static const WalkerSpec redLeeverFullSpec = 
{
    leeverAnimMap,
    10,
    RedPal,
    StdSpeed,
    Obj_None
};

static const WalkerSpec*    redLeeverSpecs[] = 
{
    &redLeeverHiddenSpec,
    &redLeeverMoundSpec,
    &redLeeverHalfSpec,
    &redLeeverFullSpec,
    &redLeeverHalfSpec,
    &redLeeverMoundSpec,
};

static const int redLeeverStateTimes[] = 
{
    0x00,
    0x10,
    0x08,
    0xFF,
    0x08,
    0x10
};


RedLeever::RedLeever( int x, int y )
    :   Object( Obj_RedLeever ),
        state( 0 ),
        spec( redLeeverSpecs[0] )
{
    objX = x;
    objY = y;
    decoration = 0;
    facing = Dir_Right;

    animator.time = 0;
    animator.durationFrames = spec->animTime;

    InitCommonStateTimer( objTimer );
    // No need to InitCommonFacing, because the facing is changed with every update.
    SetFacingAnimation();

    World::SetStunTimer( RedLeeverClassTimerSlot, 5 );
}

void RedLeever::Update()
{
    bool advanceState = false;

    if ( state == 0 )
    {
        if ( RedLeever::count >= 2 
            || World::GetStunTimer( RedLeeverClassTimerSlot ) != 0 )
            return;
        if ( !TargetPlayer() )
            return;
        World::SetStunTimer( RedLeeverClassTimerSlot, 2 );
        advanceState = true;
    }
    else if ( state == 3 )
    {
        if ( shoveDir != 0 )
        {
            ObjShove();
        }
        else if ( !IsStunned() )
        {
            if ( World::CollidesWithTileMoving( objX, objY, facing, false )
                || CheckWorldMargin( facing ) == Dir_None )
            {
                advanceState = true;
            }
            else
            {
                ObjMoveDir( spec->speed, facing );
                if ( (tileOffset & 0xF) == 0 )
                    tileOffset &= 0xF;
                objTimer = 0xFF;
            }
        }
    }

    if ( advanceState || (state != 3 && objTimer == 0) )
    {
        state = (state + 1) % _countof( redLeeverStateTimes );
        objTimer = redLeeverStateTimes[state];
        SetSpec( redLeeverSpecs[state] );

        if ( state == 1 )
            RedLeever::count++;
        else if ( state == 0 )
            RedLeever::count--;
        assert( RedLeever::count >= 0 && RedLeever::count <= 2 );
    }

    animator.Advance();

    if ( state == 3 )
    {
        CheckCollisions();
        if ( decoration != 0 && GetType() == Obj_RedLeever )
            RedLeever::count--;
    }
}

void RedLeever::Draw()
{
    if ( state != 0 )
    {
        int pal = CalcPalette( RedPal );
        animator.Draw( Sheet_Npcs, objX, objY, pal );
    }
}

void RedLeever::SetSpec( const WalkerSpec* spec )
{
    this->spec = spec;
    animator.SetDuration( spec->animTime );
    SetFacingAnimation();
}

void RedLeever::SetFacingAnimation()
{
    int dirOrd = Util::GetDirectionOrd( facing );
    if ( spec->animMap != nullptr )
        animator.anim = Graphics::GetAnimation( Sheet_Npcs, spec->animMap[dirOrd] );
    else
        animator.anim = nullptr;
}

bool RedLeever::TargetPlayer()
{
    Player* player = World::GetPlayer();
    int     x = player->GetX();
    int     y = player->GetY();

    facing = player->GetFacing();

    int r = Util::GetRandom( 256 );
    if ( r >= 0xC0 )
        facing = Util::GetOppositeDir( facing );

    if ( Util::IsVertical( facing ) )
    {
        if ( facing == Dir_Down )
            y += 0x28;
        else
            y -= 0x28;
        y = (y & 0xF0) + 0xD;
        // y's going to be assigned to a byte, so truncate it now before we test it.
        y &= 0xFF;
    }
    else
    {
        if ( facing == Dir_Right )
            x += 0x28;
        else
            x -= 0x28;
        x &= 0xF8;

        if ( abs( player->GetX() - x ) >= 0x30 )
            return false;
    }

    if ( y < 0x5D )
        return false;

    if ( World::CollidesWithTileStill( x, y ) )
        return false;

    facing = Util::GetOppositeDir( facing );
    objX = x;
    objY = y;
    return true;
}

void RedLeever::ClearRoomData()
{
    RedLeever::count = 0;
}


//----------------------------------------------------------------------------
//  Flyer
//----------------------------------------------------------------------------

struct FlyerSpec
{
    const uint8_t*  animMap;
    int             sheet;
    int             palette;
    int             speed;
};

static uint8_t peahatAnimMap[DirCount] = 
{
    Anim_OW_Peahat,
    Anim_OW_Peahat,
    Anim_OW_Peahat,
    Anim_OW_Peahat
};

FlyerSpec peahatSpec = 
{
    peahatAnimMap,
    Sheet_Npcs,
    RedPal,
    0xA0
};

Flyer::StateFunc Flyer::sStateFuncs[] = 
{
    &Flyer::UpdateHastening,
    &Flyer::UpdateFullSpeed,
    &Flyer::UpdateChase,
    &Flyer::UpdateTurn,
    &Flyer::UpdateSlowing,
    &Flyer::UpdateStill,
};


Flyer::Flyer( ObjType type, const FlyerSpec* spec, int x, int y )
    :   Object( type ),
        curSpeed( 0 ),
        accelStep( 0 ),
        state( 0 ),
        sprintsLeft( 0 ),
        spec( spec ),
        deferredDir( Dir_None ),
        moveCounter( 0 )
{
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( spec->sheet, spec->animMap[0] );
    animator.durationFrames = animator.anim->length;

    objX = x;
    objY = y;
}

void Flyer::UpdateStateAndMove()
{
    Direction origFacing = facing;

    StateFunc func = sStateFuncs[state];
    (this->*func)();

    Move();

    if ( facing != origFacing )
        SetFacingAnimation();
}

void Flyer::Draw()
{
    int pal = CalcPalette( spec->palette );
    int frame = GetFrame();
    animator.DrawFrame( spec->sheet, objX, objY, pal, frame );
}

int Flyer::GetFrame()
{
    return moveCounter & 1;
}

void Flyer::Move()
{
    accelStep += (curSpeed & 0xE0);

    if ( accelStep < 0x100 )
        return;

    accelStep &= 0xFF;
    moveCounter++;

    if ( (facing & Dir_Right) != 0 )
        objX++;

    if ( (facing & Dir_Left) != 0 )
        objX--;

    if ( (facing & Dir_Down) != 0 )
        objY++;

    if ( (facing & Dir_Up) != 0 )
        objY--;

    if ( Dir_None != CheckWorldMargin( facing ) )
        return;

    if ( GetType() == Obj_Moldorm )
    {
        int slot = World::GetCurrentObjectSlot();
        if ( slot == Moldorm::HeadSlot1 || slot == Moldorm::HeadSlot2 )
            deferredDir = Util::GetOppositeDir8( facing );
    }
    else
    {
        facing = Util::GetOppositeDir8( facing );
    }
}

int Flyer::GetState()
{
    return state;
}

void Flyer::GoToState( int state, int sprints )
{
    this->state = state;
    this->sprintsLeft = sprints;
}

void Flyer::SetFacingAnimation()
{
    int dirOrd = facing - 1;

    if ( (facing & Dir_Down) != 0 )
    {
        if ( (facing & Dir_Right) != 0 )
            dirOrd = 0;
        else
            dirOrd = 1;
    }
    else if ( (facing & Dir_Up) != 0 )
    {
        if ( (facing & Dir_Right) != 0 )
            dirOrd = 2;
        else
            dirOrd = 3;
    }

    animator.anim = Graphics::GetAnimation( spec->sheet, spec->animMap[dirOrd] );
}

void Flyer::UpdateStill()
{
    if ( objTimer == 0 )
        state = 0;
}

void Flyer::UpdateHastening()
{
    curSpeed++;
    if ( (curSpeed & 0xE0) >= spec->speed )
    {
        curSpeed = spec->speed;
        state = 1;
    }
}

void Flyer::UpdateSlowing()
{
    curSpeed--;
    if ( (curSpeed & 0xE0) <= 0 )
    {
        curSpeed = 0;
        state = 5;
        objTimer = Util::GetRandom( 64 ) + 64;
    }
}

void Flyer::UpdateFullSpeed()
{
    UpdateFullSpeedImpl();
}

void Flyer::UpdateFullSpeedImpl()
{
    int r = Util::GetRandom( 256 );

    if ( r >= 0xB0 )
    {
        state = 2;
    }
    else if ( r >= 0x20 )
    {
        state = 3;
    }
    else
    {
        state = 4;
    }
    sprintsLeft = 6;
}

void Flyer::UpdateTurn()
{
    UpdateTurnImpl();
}

void Flyer::UpdateTurnImpl()
{
    if ( objTimer != 0 )
        return;

    sprintsLeft--;
    if ( sprintsLeft == 0 )
    {
        state = 1;
        return;
    }

    objTimer = 0x10;

    facing = TurnRandomly8( facing );
}

void Flyer::UpdateChase()
{
    UpdateChaseImpl();
}

void Flyer::UpdateChaseImpl()
{
    if ( objTimer != 0 )
        return;

    sprintsLeft--;
    if ( sprintsLeft == 0 )
    {
        state = 1;
        return;
    }

    objTimer = 0x10;

    facing = TurnTowardsPlayer8( objX, objY, facing );
}


//----------------------------------------------------------------------------
//  StdFlyer
//----------------------------------------------------------------------------

StdFlyer::StdFlyer( ObjType type, const FlyerSpec* spec, int x, int y, Direction facing )
    :   Flyer( type, spec, x, y )
{
    this->facing = facing;
}


//----------------------------------------------------------------------------
//  Peahat
//----------------------------------------------------------------------------

Peahat::Peahat( int x, int y )
    :   StdFlyer( Obj_Peahat, &peahatSpec, x, y, Dir_Up )
{
    decoration = 0;
    curSpeed = 0x1F;
    objTimer = 0;
}

void Peahat::Update()
{
    if ( shoveDir != 0 )
    {
        ObjShove();
    }
    else if ( !IsStunned() )
    {
        UpdateStateAndMove();
    }

    if ( GetState() == 5 )
        CheckCollisions();
    else
        CheckPlayerCollision();
}


//----------------------------------------------------------------------------
//  FlyingGhini
//----------------------------------------------------------------------------

static uint8_t  flyingGhiniAnimMap[DirCount] = 
{
    Anim_OW_Ghini_Right,
    Anim_OW_Ghini_Left,
    Anim_OW_Ghini_UpRight,
    Anim_OW_Ghini_UpLeft,
};

FlyerSpec flyingGhiniSpec = 
{
    flyingGhiniAnimMap,
    Sheet_Npcs,
    BluePal,
    0xA0
};


FlyingGhini::FlyingGhini( int x, int y )
    :   Flyer( Obj_FlyingGhini, &flyingGhiniSpec, x, y ),
        state( 0 )
{
    decoration = 0;
    facing = Dir_Up;
    curSpeed = 0x1F;
}

void FlyingGhini::Update()
{
    if ( state == 0 )
    {
        if ( objTimer == 0 )
        {
            state++;
        }
    }
    else
    {
        if ( World::GetItem( ItemSlot_Clock ) == 0 )
            UpdateStateAndMove();

        CheckPlayerCollision();
    }
}

void FlyingGhini::Draw()
{
    if ( state == 0 )
    {
        if ( (objTimer & 1) == 1 )
            Flyer::Draw();
    }
    else
    {
        Flyer::Draw();
    }
}

void FlyingGhini::UpdateFullSpeedImpl()
{
    int r = Util::GetRandom( 256 );

    if ( r >= 0xA0 )
        GoToState( 2, 6 );
    else if ( r >= 8 )
        GoToState( 3, 6 );
    else
        GoToState( 4, 6 );
}

int FlyingGhini::GetFrame()
{
    return 0;
}


//----------------------------------------------------------------------------
//  Keese
//----------------------------------------------------------------------------

static uint8_t  keeseAnimMap[DirCount] = 
{
    Anim_UW_Keese,
    Anim_UW_Keese,
    Anim_UW_Keese,
    Anim_UW_Keese,
};

FlyerSpec blueKeeseSpec = 
{
    keeseAnimMap,
    Sheet_Npcs,
    BluePal,
    0xC0
};

FlyerSpec redKeeseSpec = 
{
    keeseAnimMap,
    Sheet_Npcs,
    RedPal,
    0xC0
};

FlyerSpec blackKeeseSpec = 
{
    keeseAnimMap,
    Sheet_Npcs,
    LevelFgPalette,
    0xC0
};


Keese* Keese::MakeBlueKeese( int x, int y )
{
    return new Keese( Obj_BlueKeese, &blueKeeseSpec, x, y, 0x1F );
}

Keese* Keese::MakeRedKeese( int x, int y )
{
    return new Keese( Obj_RedKeese, &redKeeseSpec, x, y, 0x7F );
}

Keese* Keese::MakeBlackKeese( int x, int y )
{
    return new Keese( Obj_BlackKeese, &blackKeeseSpec, x, y, 0x7F );
}

Keese::Keese( ObjType type, const FlyerSpec* spec, int x, int y, int startSpeed )
    : Flyer( type, spec, x, y )
{
    int r = Util::GetRandom( 8 );
    facing = Util::GetDirection8( r );

    curSpeed = startSpeed;
}

void Keese::SetFacing( Direction dir )
{
    facing = dir;
}

void Keese::Update()
{
    if (    World::GetItem( ItemSlot_Clock ) == 0
        && !World::IsLiftingItem() )
    {
        UpdateStateAndMove();
    }

    CheckCollisions();

    shoveDir = 0;
    shoveDistance = 0;
}

void Keese::UpdateFullSpeedImpl()
{
    int r = Util::GetRandom( 256 );

    if ( r >= 0xA0 )
        GoToState( 2, 6 );
    else if ( r >= 0x20 )
        GoToState( 3, 6 );
    else
        GoToState( 4, 6 );
}

int Keese::GetFrame()
{
    return (moveCounter & 2) >> 1;
}


//----------------------------------------------------------------------------
//  Moldorm
//----------------------------------------------------------------------------

static const uint8_t  moldormAnimMap[DirCount] = 
{
    Anim_UW_Moldorm,
    Anim_UW_Moldorm,
    Anim_UW_Moldorm,
    Anim_UW_Moldorm,
};

static const FlyerSpec moldormSpec = 
{
    moldormAnimMap,
    Sheet_Npcs,
    RedPal,
    0x80
};


Object* Moldorm::MakeSet()
{
    for ( int i = 0; i < 5 * 2; i++ )
    {
        Moldorm* moldorm = new Moldorm( 0x80, 0x70 );
        World::SetObject( i, moldorm );
    }

    Moldorm* head1 = (Moldorm*) World::GetObject( 4 );
    Moldorm* head2 = (Moldorm*) World::GetObject( 9 );
    int r;

    r = Util::GetRandom( 8 );
    head1->facing = Util::GetDirection8( r );
    head1->oldFacing = head1->facing;

    r = Util::GetRandom( 8 );
    head2->facing = Util::GetDirection8( r );
    head2->oldFacing = head2->facing;

    World::Get()->SetRoomObjCount( 8 );

    return World::GetObject( 0 );
}

Moldorm::Moldorm( int x, int y )
    : Flyer( Obj_Moldorm, &moldormSpec, x, y )
{
    decoration = 0;
    facing = Dir_None;
    oldFacing = facing;

    curSpeed = 0x80;

    GoToState( 2, 1 );
}

void Moldorm::Update()
{
    if ( facing == Dir_None )
        return;

    if ( World::GetItem( ItemSlot_Clock ) == 0 )
        UpdateStateAndMove();

    CheckMoldormCollisions();
}

void Moldorm::CheckMoldormCollisions()
{
    // ORIGINAL: This is just like CheckLamnolaCollisions; but it saves stateTimer, and plays sounds.

    Direction origFacing = facing;
    int origStateTimer = objTimer;

    CheckCollisions();

    objTimer = origStateTimer;
    facing = origFacing;

    if ( decoration == 0 )
        return;

    Sound::PlayEffect( SEffect_boss_hit );
    Sound::StopEffect( Sound::AmbientInstance );

    int slot = World::GetCurrentObjectSlot();
    Object* obj = nullptr;

    if ( slot >= TailSlot2 )
        slot = TailSlot2;
    else
        slot = TailSlot1;

    for ( ; ; slot++ )
    {
        obj = World::GetObject( slot );
        if ( obj != nullptr && obj->GetType() == GetType() )
            break;
    }

    if ( slot == HeadSlot1 || slot == HeadSlot2 )
        return;

    hp = 0x20;
    shoveDir = 0;
    shoveDistance = 0;
    decoration = 0;

    DeadDummy* dummy = new DeadDummy( objX, objY );
    World::SetObject( slot, dummy );
}

void Moldorm::UpdateTurnImpl()
{
    int slot = World::GetCurrentObjectSlot();
    if ( slot != HeadSlot1 && slot != HeadSlot2 )
        return;

    Flyer::UpdateTurnImpl();
    UpdateSubstates();
}

void Moldorm::UpdateChaseImpl()
{
    int slot = World::GetCurrentObjectSlot();
    if ( slot != HeadSlot1 && slot != HeadSlot2 )
        return;

    Flyer::UpdateChaseImpl();
    UpdateSubstates();
}

void Moldorm::UpdateSubstates()
{
    if ( objTimer == 0 )
    {
        int r = Util::GetRandom( 256 );
        if ( r < 0x40 )
            GoToState( 3, 8 );
        else
            GoToState( 2, 8 );

        objTimer = 0x10;

        // This is the head, so all other parts are at lower indexes.
        int slot = World::GetCurrentObjectSlot();
        int prevSlot = slot - 1;

        Object* obj = World::GetObject( prevSlot );
        if ( obj != nullptr && obj->GetType() == Obj_Moldorm && obj->GetFacing() != Dir_None )
            ShiftFacings();
    }
    else
    {
        ShiftFacings();
    }
}

void Moldorm::ShiftFacings()
{
    if ( objTimer != 0x10 )
        return;

    if ( deferredDir != Dir_None )
    {
        facing = deferredDir;
        deferredDir = Dir_None;
    }

    int     slot = World::GetCurrentObjectSlot() - 4;

    for ( int i = 0; i < 4; i++, slot++ )
    {
        Object* curObj  = World::GetObject( slot );
        Object* nextObj = World::GetObject( slot + 1 );

        if (   curObj  == nullptr || curObj->GetType()  != Obj_Moldorm 
            || nextObj == nullptr || nextObj->GetType() != Obj_Moldorm )
            continue;

        Moldorm* curMoldorm  = (Moldorm*) curObj;
        Moldorm* nextMoldorm = (Moldorm*) nextObj;

        Direction nextOldFacing = nextMoldorm->oldFacing;
        curMoldorm->oldFacing = nextOldFacing;
        curMoldorm->facing = nextOldFacing;
    }

    oldFacing = facing;
}

int Moldorm::GetFrame()
{
    return 0;
}


//----------------------------------------------------------------------------
//  Patra
//----------------------------------------------------------------------------

static uint8_t patraAnimMap[DirCount] = 
{
    Anim_B3_Patra,
    Anim_B3_Patra,
    Anim_B3_Patra,
    Anim_B3_Patra
};

FlyerSpec patraSpec = 
{
    patraAnimMap,
    Sheet_Boss,
    BluePal,
    0x40
};


Patra* Patra::MakePatra( ObjType type )
{
    Patra*  patra = new Patra( type );
    ObjType childType = (type == Obj_Patra1) ? Obj_PatraChild1 : Obj_PatraChild2;

    World::SetObject( 0, patra );

    for ( int i = 1; i < 9; i++ )
    {
        PatraChild* child = new PatraChild( childType );
        World::SetObject( i, child );
    }

    return patra;
}

static int patraAngle[9];
static int patraState[9];

Patra::Patra( ObjType type )
    :   Flyer( type, &patraSpec, 0x80, 0x70 ),
        xMove( 0 ),
        yMove( 0 ),
        maneuverState( 0 ),
        childStateTimer( 0xFF )
{
    invincibilityMask = 0xFE;
    facing = Dir_Up;
    curSpeed = 0x1F;

    Sound::PlayEffect( SEffect_boss_roar3, true, Sound::AmbientInstance );

    memset( patraAngle, 0, sizeof patraAngle );
    memset( patraState, 0, sizeof patraState );
}

int Patra::GetXMove()
{
    return xMove;
}

int Patra::GetYMove()
{
    return yMove;
}

int Patra::GetManeuverState()
{
    return maneuverState;
}

void Patra::Update()
{
    if ( childStateTimer > 0 )
        childStateTimer--;

    int origX = GetX();
    int origY = GetY();

    UpdateStateAndMove();

    xMove = GetX() - origX;
    yMove = GetY() - origY;

    bool foundChild = false;

    for ( int slot = MonsterSlot1 + 1; slot < MonsterSlot1 + 9; slot++ )
    {
        Object* obj = World::GetObject( slot );
        if ( obj != nullptr && (obj->GetType() == Obj_PatraChild1 || obj->GetType() == Obj_PatraChild2) )
        {
            foundChild = true;
            break;
        }
    }

    if ( foundChild )
    {
        CheckPlayerCollision();
    }
    else
    {
        CheckCollisions();
        PlayBossHitSoundIfHit();
        PlayBossHitSoundIfDied();
    }

    if ( childStateTimer == 0 && patraAngle[2] == 0 )
    {
        maneuverState ^= 1;
        // ORIGINAL: I don't see how this is ever $50. See Patra's Update routine.
        childStateTimer = 0xFF;
    }
}

void Patra::UpdateFullSpeedImpl()
{
    int r = Util::GetRandom( 256 );

    if ( r >= 0x40 )
        GoToState( 2, 8 );
    else
        GoToState( 3, 8 );
}


//----------------------------------------------------------------------------
//  PatraChild
//----------------------------------------------------------------------------

PatraChild::PatraChild( ObjType type )
    :   Object( type ),
        x( 0 ),
        y( 0 ),
        angleAccum( 0 )
{
    invincibilityMask = 0xFE;
    decoration = 0;

    objTimer = 0;

    animator.durationFrames = 4;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B3_PatraChild );
}

static uint16_t ShiftMult( int mask, int addend, int shiftCount )
{
    uint16_t n = 0;

    do
    {
        n <<= 1;
        mask <<= 1;
        if ( (mask & 0x100) != 0 )
        {
            n += addend;
        }
        shiftCount--;
    } while ( shiftCount != 0 );

    return n;
}

void PatraChild::Update()
{
    int slot = World::GetCurrentObjectSlot();

    if ( patraState[slot] == 0 )
    {
        UpdateStart();
    }
    else
    {
        UpdateTurn();
        animator.Advance();

        if ( patraState[0] != 0 )
        {
            CheckCollisions();
            if ( decoration != 0 )
            {
                DeadDummy* dummy = new DeadDummy( objX, objY );
                World::SetObject( slot, dummy );
            }
        }
    }
}

void PatraChild::Draw()
{
    int slot = World::GetCurrentObjectSlot();

    if ( patraState[slot] != 0 )
    {
        int pal = CalcPalette( RedPal );
        animator.Draw( Sheet_Boss, objX, objY, pal );
    }
}

void PatraChild::UpdateStart()
{
    static const uint8_t patraEntryAngles[] = 
    { 0x14, 0x10, 0xC, 0x8, 0x4, 0, 0x1C };

    int slot = World::GetCurrentObjectSlot();

    if ( slot != 1 )
    {
        if ( patraState[1] == 0 )
            return;

        int index = slot - 2;
        if ( patraAngle[1] != patraEntryAngles[index] )
            return;
    }

    Patra*  patra = (Patra*) World::GetObject( 0 );
    int     distance = (GetType() == Obj_PatraChild1) ? 0x2C : 0x18;

    if ( slot == 8 )
        patraState[0] = 1;
    patraState[slot] = 1;
    patraAngle[slot] = 0x18;

    x = patra->GetX() << 8;
    y = (patra->GetY() - distance) << 8;

    objX = x >> 8;
    objY = y >> 8;
}

void PatraChild::UpdateTurn()
{
    int     slot = World::GetCurrentObjectSlot();
    Patra*  patra = (Patra*) World::GetObject( 0 );

    x += patra->GetXMove() << 8;
    y += patra->GetYMove() << 8;

    int         step = (GetType() == Obj_PatraChild1) ? 0x70 : 0x60;
    uint16_t    angleFix = (patraAngle[slot] << 8) | angleAccum;
    angleFix -= step;
    angleAccum = angleFix & 0xFF;
    patraAngle[slot] = (angleFix >> 8) & 0x1F;

    static const int shiftCounts[] = { 6, 5, 6, 6 };
    int yShiftCount;
    int xShiftCount;
    int index = patra->GetManeuverState();

    if ( GetType() == Obj_PatraChild1 )
    {
        yShiftCount = shiftCounts[index];
        xShiftCount = shiftCounts[index + 2];
    }
    else
    {
        yShiftCount = shiftCounts[index + 1];
        xShiftCount = yShiftCount;
    }

    const int TurnSpeed = 0x20;
    static const uint8_t sinCos[] = 
    { 0x00, 0x18, 0x30, 0x47, 0x5A, 0x6A, 0x76, 0x7D, 0x80, 0x7D, 0x76, 0x6A, 0x5A, 0x47, 0x30, 0x18 };

    index = patraAngle[slot] & 0xF;
    uint8_t cos = sinCos[index];
    uint16_t n = ShiftMult( cos, TurnSpeed, xShiftCount );

    if ( (patraAngle[slot] & 0x18) < 0x10 )
        x = x + n;
    else
        x = x - n;

    index = (patraAngle[slot] + 8) & 0xF;
    uint8_t sin = sinCos[index];
    n = ShiftMult( sin, TurnSpeed, yShiftCount );

    if ( ((patraAngle[slot] - 8) & 0x18) < 0x10 )
        y = y + n;
    else
        y = y - n;

    objX = x >> 8;
    objY = y >> 8;
}


//----------------------------------------------------------------------------
//  Jumper
//----------------------------------------------------------------------------

struct JumperSpec
{
    const uint8_t*  animMap;
    int             animTimer;
    int             jumpFrame;
    int             palette;
    int             speed;
    const uint8_t*  accelMap;
};

static uint8_t tektiteAnimMap[DirCount] = 
{
    Anim_OW_Tektite,
    Anim_OW_Tektite,
    Anim_OW_Tektite,
    Anim_OW_Tektite
};

static uint8_t boulderAnimMap[DirCount] = 
{
    Anim_OW_Boulder,
    Anim_OW_Boulder,
    Anim_OW_Boulder,
    Anim_OW_Boulder
};

static const uint8_t blueTektiteSpeeds[] = 
{
    0, 
    0x40, 
    0x40, 
    0, 
    0, 
    0x40, 
    0x40, 
    0, 
    0, 
    0x30, 
    0x30
};

static const uint8_t redTektiteSpeeds[] = 
{
    0, 
    0x80, 
    0x80, 
    0, 
    0, 
    0x80, 
    0x80, 
    0, 
    0, 
    0x50, 
    0x50
};

static const uint8_t boulderSpeeds[] = 
{
    0x60, 0x60, 
    0x60, 0x60, 0x60, 0x60, 0x60
};

JumperSpec blueTektiteSpec = 
{
    tektiteAnimMap,
    32,
    1,
    BluePal,
    -3,
    blueTektiteSpeeds
};

JumperSpec redTektiteSpec = 
{
    tektiteAnimMap,
    32,
    1,
    RedPal,
    -4,
    redTektiteSpeeds
};

JumperSpec boulderSpec = 
{
    boulderAnimMap,
    12,
    -1,
    RedPal,
    -2,
    boulderSpeeds
};

int targetYOffset[] = 
{
    0,
    0,
    0,
    0,
    0,
    0x20,
    0x20,
    0,
    0,
    -0x20,
    -0x20,
};

static const int jumperStartDirs[] = 
{
    1,
    2,
    5,
    0xA
};


Jumper::Jumper( ObjType type, const JumperSpec* spec, int x, int y )
    :   Object( type ),
        curSpeed( 0 ),
        accelStep( 0 ),
        state( 0 ),
        targetY( 0 ),
        reversesPending( 0 ),
        spec( spec )
{
    objX = x;
    objY = y;

    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Npcs, spec->animMap[0] );
    animator.durationFrames = spec->animTimer;

    int r = Util::GetRandom( 4 );
    facing = (Direction) jumperStartDirs[r];
    objTimer = facing * 4;

    if ( GetType() == Obj_Boulder )
    {
        Boulders::Count()++;
        decoration = 0;
    }
}

Jumper::~Jumper()
{
    if ( GetType() == Obj_Boulder )
        Boulders::Count()--;
}

void Jumper::Update()
{
    if ( shoveDir == 0 && !IsStunned() )
    {
        if ( state == 0 )
            UpdateStill();
        else
            UpdateJump();
    }

    if ( GetType() == Obj_Boulder )
    {
        animator.Advance();
        CheckPlayerCollision();
        if ( objY >= World::WorldLimitBottom  )
            isDeleted = true;
    }
    else
    {
        if ( state == 0 && objTimer >= 0x21 )
            animator.Advance();
        CheckCollisions();
    }
}

void Jumper::Draw()
{
    int pal = CalcPalette( spec->palette );

    if ( state == 1 && spec->jumpFrame >= 0 )
    {
        animator.DrawFrame( Sheet_Npcs, objX, objY, pal, spec->jumpFrame );
    }
    else
    {
        animator.Draw( Sheet_Npcs, objX, objY, pal );
    }
}

void Jumper::UpdateStill()
{
    if ( objTimer != 0 )
        return;

    state = 1;
    facing = TurnTowardsPlayer8( objX, objY, facing );

    if ( (facing & (Dir_Right | Dir_Left)) == 0 )
    {
        facing = facing | GetXDirToPlayer( objX );
    }

    SetupJump();
}

void Jumper::UpdateJump()
{
    Direction dir = CheckWorldMarginH( objX, facing, false );
    if ( GetType() != Obj_Boulder )
        dir = CheckWorldMarginV( objY, dir, false );

    if ( dir == Dir_None )
    {
        facing = Util::GetOppositeDir8( facing );
        reversesPending++;
        SetupJump();
        return;
    }

    ConstrainFacing();
    reversesPending = 0;
    int acceleration = spec->accelMap[facing];

    UpdateY( 2, acceleration );
    if ( (facing & Dir_Left) != 0 )
        objX--;
    else if ( (facing & Dir_Right) != 0 )
        objX++;

    if ( curSpeed >= 0 && abs( objY - targetY ) < 3 )
    {
        state = 0;
        if ( GetType() == Obj_Boulder )
            objTimer = 0;
        else
            objTimer = GetRandomStillTime();
    }
}

void Jumper::UpdateY( int maxSpeed, int acceleration )
{
    objY += curSpeed;
    accelStep += acceleration;

    int carry = accelStep >> 8;
    accelStep &= 0xFF;

    curSpeed += carry;

    if ( curSpeed >= maxSpeed && accelStep >= 0x80 )
    {
        curSpeed = maxSpeed;
        accelStep = 0;
    }
}

void Jumper::SetupJump()
{
    if ( reversesPending >= 2 )
    {
        facing = facing ^ (Dir_Right | Dir_Left);
        reversesPending = 0;
    }

    ConstrainFacing();
    targetY = objY + targetYOffset[facing];
    curSpeed = spec->speed;
    accelStep = 0;
}

int Jumper::GetRandomStillTime()
{
    uint8_t r = Util::GetRandom( 256 );
    uint8_t t = r + 0x10;

    if ( t < 0x20 )
        t -= 0x40;
    if ( GetType() != Obj_BlueTektite )
    {
        t &= 0x7F;
        if ( r >= 0xA0 )
            t &= 0x0F;
    }
    return t;
}

void Jumper::ConstrainFacing()
{
    if ( GetType() == Obj_Boulder )
    {
        facing = facing & (Dir_Right | Dir_Left);
        facing = facing | Dir_Down;
    }
}


//----------------------------------------------------------------------------
//  Boulders
//----------------------------------------------------------------------------

int Boulders::sCount;

Boulders::Boulders()
    :   Object( Obj_Boulders )
{
    int r = Util::GetRandom( 4 );
    int facing = jumperStartDirs[r];
    objTimer = facing * 4;
    decoration = 0;
}

void Boulders::Update()
{
    if ( objTimer == 0 )
    {
        if ( sCount < MaxBoulders )
        {
            Point playerPos = World::GetObservedPlayerPos();
            int y = World::WorldLimitTop;
            int x = Util::GetRandom( 256 );

            // Make sure the new boulder is in the same half of the screen.
            if ( playerPos.X < World::WorldMidX )
                x = x % 0x80;
            else
                x = x | 0x80;

            int slot = World::FindEmptyMonsterSlot();
            if ( slot >= 0 )
            {
                Object* obj = MakeMonster( Obj_Boulder, x, y );
                World::SetObject( slot, obj );

                objTimer = Util::GetRandom( 32 );
            }
        }
        else
        {
            int r = Util::GetRandom( 256 );
            objTimer = (objTimer + r) % 256;
        }
    }
}

void Boulders::Draw()
{
}

int& Boulders::Count()
{
    return sCount;
}

void Boulders::ClearRoomData()
{
    sCount = 0;
}


//----------------------------------------------------------------------------
//  Trap
//----------------------------------------------------------------------------

static const Point trapPos[] = 
{
    { 0x20, 0x60 },
    { 0x20, 0xC0 },
    { 0xD0, 0x60 },
    { 0xD0, 0xC0 },
    { 0x40, 0x90 },
    { 0xB0, 0x90 },
};

static const int trapAllowedDirs[] = 
{
    5, 9, 6, 0xA, 1, 2
};

Object* Trap::MakeSet( int count )
{
    assert( count >= 1 && count <= 6 );
    if ( count < 1 )
        count = 1;
    if ( count > 6 )
        count = 6;

    int slot = MonsterSlot1;

    for ( int i = 0; i < count; i++, slot++ )
    {
        Object* obj = new Trap( i, trapPos[i].X, trapPos[i].Y );
        World::SetObject( slot, obj );
    }

    return World::GetObject( MonsterSlot1 );
}

Trap::Trap( int trapIndex, int x, int y )
    :   Object( Obj_Trap ),
        trapIndex( trapIndex ),
        state( 0 ),
        speed( 0 ),
        origCoord( 0 )
{
    objX = x;
    objY = y;

    image.anim = Graphics::GetAnimation( Sheet_Npcs, Anim_UW_Trap );
}

void Trap::Update()
{
    if ( state == 0 )
        UpdateIdle();
    else
        UpdateMoving();

    CheckCollisions();
}

void Trap::UpdateIdle()
{
    Player*     player = World::GetPlayer();
    int         playerX = player->GetX();
    int         playerY = player->GetY();
    Direction   dir = Dir_None;
    int         distX = abs( playerX - objX );
    int         distY = abs( playerY - objY );

    if ( distY >= 0xE )
    {
        if ( distX < 0xE )
        {
            if ( playerY < objY )
                dir = Dir_Up;
            else
                dir = Dir_Down;
            origCoord = objY;
        }
    }
    else
    {
        if ( distX >= 0xE )
        {
            if ( playerX < objX )
                dir = Dir_Left;
            else
                dir = Dir_Right;
            origCoord = objX;
        }
    }

    if ( dir != Dir_None )
    {
        if ( (dir & trapAllowedDirs[trapIndex]) != 0 )
        {
            facing = dir;
            state++;
            speed = 0x70;
        }
    }
}

void Trap::UpdateMoving()
{
    ObjMoveDir( speed, facing );

    if ( (tileOffset & 0xF) == 0 )
        tileOffset &= 0xF;

    CheckPlayerCollision();

    int coord;
    int limit;

    if ( Util::IsVertical( facing ) )
    {
        coord = objY;
        limit = 0x90;
    }
    else
    {
        coord = objX;
        limit = 0x78;
    }

    if ( state == 1 )
    {
        if ( abs( coord - limit ) < 5 )
        {
            facing = Util::GetOppositeDir( facing );
            speed = 0x20;
            state++;
        }
    }
    else
    {
        if ( coord == origCoord )
            state = 0;
    }
}

void Trap::Draw()
{
    image.Draw( Sheet_Npcs, objX, objY, BluePal );
}


//----------------------------------------------------------------------------
//  Rope
//----------------------------------------------------------------------------

const static uint8_t    ropeAnimMap[DirCount] = 
{
    Anim_UW_Rope_Right,
    Anim_UW_Rope_Left,
    Anim_UW_Rope_Right,
    Anim_UW_Rope_Right
};

const int RopeNormalSpeed = 0x20;
const int RopeFastSpeed = 0x60;

Rope::Rope( int x, int y )
    :   Object( Obj_Rope ),
        speed( RopeNormalSpeed )
{
    objX = x;
    objY = y;

    animator.time = 0;
    animator.durationFrames = 20;

    InitCommonFacing( objX, objY, facing );
    SetFacingAnimation();

    Profile& profile = World::GetProfile();

    if ( profile.Quest == 0 )
        hp = 0x10;
    else
        hp = 0x40;
}

void Rope::Update()
{
    Direction origFacing = facing;

    moving = facing;

    if ( !IsStunned() )
    {
        ObjMove( speed );

        if ( (tileOffset & 0xF) == 0 )
            tileOffset &= 0xF;

        if ( speed != RopeFastSpeed && objTimer == 0 )
        {
            objTimer = Util::GetRandom( 0x40 );
            TurnToUnblockedDir();
        }
    }

    if ( facing != origFacing )
        speed = RopeNormalSpeed;

    TargetPlayer();

    animator.Advance();

    CheckCollisions();
    SetFacingAnimation();
}

void Rope::TargetPlayer()
{
    if ( speed != RopeNormalSpeed || tileOffset != 0 )
        return;

    Player* player = World::GetPlayer();

    int xDist = abs( player->GetX() - objX );
    if ( xDist < 8 )
    {
        if ( player->GetY() < objY )
            facing = Dir_Up;
        else
            facing = Dir_Down;
        speed = RopeFastSpeed;
    }
    else
    {
        int yDist = abs( player->GetY() - objY );
        if ( yDist < 8 )
        {
            if ( player->GetX() < objX )
                facing = Dir_Left;
            else
                facing = Dir_Right;
            speed = RopeFastSpeed;
        }
    }
}

void Rope::Draw()
{
    Profile& profile = World::GetProfile();
    int pal;

    if ( profile.Quest == 0 )
        pal = CalcPalette( RedPal );
    else
        pal = PlayerPalette + (GetFrameCounter() & 3);

    animator.Draw( Sheet_Npcs, objX, objY, pal );
}

void Rope::SetFacingAnimation()
{
    int dirOrd = Util::GetDirectionOrd( facing );
    animator.anim = Graphics::GetAnimation( Sheet_Npcs, ropeAnimMap[dirOrd] );
}


//----------------------------------------------------------------------------
//  PolsVoice
//----------------------------------------------------------------------------

static const int polsVoiceXSpeeds[]    = { 1, -1, 0, 0 };
static const int polsVoiceYSpeeds[]    = { 0, 0, 1, -1 };
static const int polsVoiceJumpSpeeds[] = { -3, -3, -1, -4 };
static const int polsVoiceJumpLimits[] = { 0, 0, 0x20, -0x20 };

PolsVoice::PolsVoice( int x, int y )
    :   Object( Obj_PolsVoice ),
        curSpeed( 0 ),
        accelStep( 0 ),
        state( 0 ),
        stateTimer( 0 ),
        targetY( 0 )
{
    objX = x;
    objY = y;

    InitCommonFacing( x, y, facing );

    animator.anim = Graphics::GetAnimation( Sheet_Npcs, Anim_UW_PolsVoice );
    animator.durationFrames = 16;
    animator.time = 0;
}

void PolsVoice::Update()
{
    if ( !IsStunned() && (GetFrameCounter() & 1) == 0 )
        Move();

    animator.Advance();
    invincibilityMask = 0xFE;
    CheckCollisions();
}

void PolsVoice::Draw()
{
    int pal = CalcPalette( PlayerPal );
    animator.Draw( Sheet_Npcs, objX, objY, pal );
}

void PolsVoice::Move()
{
    UpdateX();
    if ( !UpdateY() )
        return;

    TileCollision collision;
    int x = objX;
    int y = objY;

    collision = World::CollidesWithTileStill( x, y );
    if ( !collision.Collides )
    {
        x += 0xE;
        y += 6;
        collision = World::CollidesWithTileStill( x, y );
        if ( !collision.Collides )
            return;
    }

    if ( collision.TileRef == Tile_Wall )
    {
        facing = Util::GetOppositeDir( facing );

        if ( Util::IsHorizontal( facing ) )
        {
            UpdateX();
            UpdateX();
        }
    }
    else
    {
        SetupJump();
    }
}

void PolsVoice::UpdateX()
{
    int ord = Util::GetDirectionOrd( facing );
    objX += polsVoiceXSpeeds[ord];
}

bool PolsVoice::UpdateY()
{
    if ( state == 1 )
        return UpdateJumpY();
    else
        return UpdateWalkY();
}

bool PolsVoice::UpdateJumpY()
{
    const int Acceleration = 0x38;

    accelStep += Acceleration;

    int carry = accelStep >> 8;
    accelStep &= 0xFF;

    curSpeed += carry;
    objY += curSpeed;

    if ( curSpeed >= 0 && objY >= targetY )
    {
        state = 0;
        curSpeed = 0;
        accelStep = 0;
        int r = Util::GetRandom( 256 );
        facing = Util::GetOrdDirection( r & 3 );
        stateTimer = (r & 0x40) + 0x30;
        objX = (objX + 8) & 0xF0;
        objY = (objY + 8) & 0xF0;
        objY -= 3;
    }
    return true;
}

bool PolsVoice::UpdateWalkY()
{
    if ( stateTimer == 0 )
    {
        SetupJump();
        return false;
    }
    else
    {
        stateTimer--;
        int ord = Util::GetDirectionOrd( facing );
        objY += polsVoiceYSpeeds[ord];
    }
    return true;
}

void PolsVoice::SetupJump()
{
    if ( state != 0 )
        return;

    int dirOrd = Util::GetDirectionOrd( facing );

    if ( objY < 0x78 )
        dirOrd = 2;
    else if ( objY >= 0xA8 )
        dirOrd = 3;

    curSpeed = polsVoiceJumpSpeeds[dirOrd];
    targetY = objY + polsVoiceJumpLimits[dirOrd];

    facing = Util::GetOrdDirection( dirOrd );
    state = 1;
}


//----------------------------------------------------------------------------
//  RedWizzrobe
//----------------------------------------------------------------------------

static const uint8_t wizzrobeAnimMap[DirCount] = 
{
    Anim_UW_Wizzrobe_Right,
    Anim_UW_Wizzrobe_Left,
    Anim_UW_Wizzrobe_Right,
    Anim_UW_Wizzrobe_Up
};

RedWizzrobe::StateFunc RedWizzrobe::sStateFuncs[] = 
{
    &RedWizzrobe::UpdateHidden,
    &RedWizzrobe::UpdateGoing,
    &RedWizzrobe::UpdateVisible,
    &RedWizzrobe::UpdateComing,
};


RedWizzrobe::RedWizzrobe()
    :   Object( Obj_RedWizzrobe ),
        stateTimer( 0 ),
        flashTimer( 0 )
{
    decoration = 0;
    animator.durationFrames = 8;
    animator.time = 0;
}

void RedWizzrobe::Update()
{
    if ( World::GetItem( ItemSlot_Clock ) != 0 )
    {
        animator.Advance();
        CheckRedWizzrobeCollisions();
        return;
    }

    stateTimer--;

    int state = GetState();

    StateFunc proc = sStateFuncs[state];
    (this->*proc)();

    animator.Advance();
}

void RedWizzrobe::Draw()
{
    int state = GetState();

    if ( state == 2 || (state > 0 && (flashTimer & 1) == 0) )
    {
        int pal = CalcPalette( RedPal );
        animator.Draw( Sheet_Npcs, objX, objY, pal );
    }
}

int RedWizzrobe::GetState()
{
    return stateTimer >> 6;
}

void RedWizzrobe::SetFacingAnimation()
{
    int dirOrd = Util::GetDirectionOrd( facing );
    animator.anim = Graphics::GetAnimation( Sheet_Npcs, wizzrobeAnimMap[dirOrd] );
}

void RedWizzrobe::UpdateHidden()
{
    // Nothing to do
}

void RedWizzrobe::UpdateGoing()
{
    if ( stateTimer == 0x7F )
        stateTimer = 0x4F;

    flashTimer++;

    if ( (flashTimer & 1) == 0 )
        CheckRedWizzrobeCollisions();
}

void RedWizzrobe::UpdateVisible()
{
    if ( stateTimer == 0xB0 )
    {
        if ( World::GetItem( ItemSlot_Clock ) == 0 )
        {
            Sound::PlayEffect( SEffect_magic_wave );
            ::Shoot( Obj_MagicWave2, objX, objY, facing );
        }
    }

    CheckRedWizzrobeCollisions();
}

static const Direction wizzrobeDirs[] = 
{
    Dir_Down,
    Dir_Up,
    Dir_Right,
    Dir_Left
};

static const int wizzrobeXOffsets[] = 
{ 
    0x00, 0x00, -0x20, 0x20, 0x00, 0x00, -0x40, 0x40, 
    0x00, 0x00, -0x30, 0x30, 0x00, 0x00, -0x50, 0x50
};

static const int wizzrobeYOffsets[] = 
{
    -0x20, 0x20, 0x00, 0x00, -0x40, 0x40, 0x00, 0x00, 
    -0x30, 0x30, 0x00, 0x00, -0x50, 0x50, 0x00, 0x00
};

static const int allWizzrobeCollisionXOffsets[] = 
{ 0xF, 0, 0, 4, 8, 0, 0, 4, 8, 0 };

static const int allWizzrobeCollisionYOffsets[] = 
{ 4, 4, 0, 8, 8, 8, 0, -8, 0, 0 };

static int CheckWizzrobeTileCollision( int x, int y, Direction dir )
{
    int ord = dir - 1;
    x += allWizzrobeCollisionXOffsets[ord];
    y += allWizzrobeCollisionYOffsets[ord];

    TileCollision collision;

    collision = World::CollidesWithTileStill( x, y );
    if ( !collision.Collides )
        return 0;

    // This isn't quite the same as the original game, because the original contrasted 
    // blocks and water together with everything else.

    if ( collision.TileRef == Tile_Wall )
        return 1;

    return 2;
}

void RedWizzrobe::UpdateComing()
{
    if ( stateTimer == 0xFF )
    {
        Player* player = World::GetPlayer();

        uint32_t r = Util::GetRandom( 16 );
        int dirOrd = r % 4;
        facing = wizzrobeDirs[dirOrd];

        objX = (player->GetX() + wizzrobeXOffsets[r]) & 0xF0;
        objY = (player->GetY() + wizzrobeYOffsets[r] + 3) & 0xF0 - 3;

        if ( objY < 0x5D || objY >= 0xC4 )
        {
            stateTimer++;    // Try again
        }
        else
        {
            int collisionResult = CheckWizzrobeTileCollision( objX, objY, facing );

            if ( collisionResult != 0 )
                stateTimer++;    // Try again
        }

        if ( stateTimer != 0 )
            SetFacingAnimation();
    }
    else
    {
        if ( stateTimer == 0x7F )
            stateTimer = 0x4F;

        flashTimer++;
        if ( (flashTimer & 1) == 0 )
            CheckRedWizzrobeCollisions();
    }
}

void RedWizzrobe::CheckRedWizzrobeCollisions()
{
    // If I really wanted, I could make a friend function or class to do this, which is the same 
    // as in BlueWizzrobe.

    invincibilityMask = 0xF6;
    if ( invincibilityTimer == 0 )
    {
        CheckWave( PlayerSwordShotSlot );
        CheckBombAndFire( BombSlot );
        CheckBombAndFire( BombSlot2 );
        CheckBombAndFire( FireSlot );
        CheckBombAndFire( FireSlot2 );
        CheckSword( PlayerSwordSlot );
    }
    CheckPlayerCollision();
}


//----------------------------------------------------------------------------
//  BlueWizzrobeBase
//----------------------------------------------------------------------------

BlueWizzrobeBase::BlueWizzrobeBase( ObjType type, int x, int y )
    :   Object( type ),
        flashTimer( 0 ),
        turnTimer( 0 )
{
    objX = x;
    objY = y;
    decoration = 0;
}

void BlueWizzrobeBase::TruncatePosition()
{
    objX = (objX + 8) & 0xF0;
    objY = (objY + 8) & 0xF0;
    objY -= 3;
}

void BlueWizzrobeBase::MoveOrTeleport()
{
    if ( objTimer != 0 )
    {
        if ( objTimer >= 0x10 )
        {
            if ( (GetFrameCounter() & 1) == 1 )
            {
                TurnIfNeeded();
            }
            else
            {
                turnTimer++;
                TurnIfNeeded();
                MoveAndCollide();
            }
        }
        else if ( objTimer == 1 )
            TryTeleporting();
        return;
    }

    if ( flashTimer == 0 )
    {
        int r = Util::GetRandom( 256 );
        objTimer = r | 0x70;
        TruncatePosition();
        Turn();
        return;
    }

    flashTimer--;
    MoveAndCollide();
}

static const int blueWizzrobeXSpeeds[] = 
{ 0, 1, -1, 0, 0, 1, -1, 0, 0, 1, -1 };

static const int blueWizzrobeYSpeeds[] = 
{ 0, 0, 0, 0, 1, 1, 1, 0, -1, -1, -1 };

void BlueWizzrobeBase::MoveAndCollide()
{
    Move();

    int collisionResult = CheckWizzrobeTileCollision( objX, objY, facing );

    if ( collisionResult == 1 )
    {
        if ( (facing & 0xC) != 0 )
            facing = (Direction) (facing ^ 0xC);
        if ( (facing & 3) != 0 )
            facing = (Direction) (facing ^ 3);

        Move();
    }
    else if ( collisionResult == 2 )
    {
        if ( flashTimer == 0 )
        {
            flashTimer = 0x20;
            turnTimer ^= 0x40;
            objTimer = 0;
            TruncatePosition();
        }
    }
}

void BlueWizzrobeBase::Move()
{
    objX += blueWizzrobeXSpeeds[facing];
    objY += blueWizzrobeYSpeeds[facing];
}

void BlueWizzrobeBase::TryShooting()
{
    if ( World::GetItem( ItemSlot_Clock ) != 0 )
        return;
    if ( flashTimer != 0 )
        return;
    if ( (GetFrameCounter() % 0x20) != 0 )
        return;

    Player*     player = World::GetPlayer();
    Direction   dir;

    if ( (player->GetY() & 0xF0) != (objY & 0xF0) )
    {
        if ( player->GetX() != (objX & 0xF0) )
            return;

        dir = GetYDirToTruePlayer( objY );
    }
    else
    {
        dir = GetXDirToTruePlayer( objX );
    }

    if ( dir != facing )
        return;

    Sound::PlayEffect( SEffect_magic_wave );
    ::Shoot( Obj_MagicWave, objX, objY, facing );
}

void BlueWizzrobeBase::TurnIfNeeded()
{
    if ( (turnTimer & 0x3F) == 0 )
        Turn();
}

void BlueWizzrobeBase::Turn()
{
    Direction dir;

    if ( (turnTimer & 0x40) != 0 )
    {
        dir = GetYDirToTruePlayer( objY );
    }
    else
    {
        dir = GetXDirToTruePlayer( objX );
    }

    if ( dir == facing )
        return;

    facing = dir;
    TruncatePosition();
}

int blueWizzrobeTeleportXOffsets[] = 
{ -0x20, 0x20, -0x20, 0x20 };

int blueWizzrobeTeleportYOffsets[] = 
{ -0x20, -0x20, 0x20, 0x20 };

int blueWizzrobeTeleportDirs[] = 
{ 0xA, 9, 6, 5 };

void BlueWizzrobeBase::TryTeleporting()
{
    int index = Util::GetRandom( 4 );

    int teleportX = objX + blueWizzrobeTeleportXOffsets[index];
    int teleportY = objY + blueWizzrobeTeleportYOffsets[index];
    Direction dir = (Direction) blueWizzrobeTeleportDirs[index];

    int collisionResult = CheckWizzrobeTileCollision( teleportX, teleportY, dir );

    if ( collisionResult != 0 )
    {
        int r = Util::GetRandom( 256 );
        objTimer = r | 0x70;
    }
    else
    {
        facing = dir;

        flashTimer = 0x20;
        turnTimer ^= 0x40;
        objTimer = 0;
    }

    TruncatePosition();
}


//----------------------------------------------------------------------------
//  BlueWizzrobe
//----------------------------------------------------------------------------

BlueWizzrobe::BlueWizzrobe( int x, int y )
    :   BlueWizzrobeBase( Obj_BlueWizzrobe, x, y )
{
    animator.durationFrames = 16;
    animator.time = 0;
}

void BlueWizzrobe::Update()
{
    if ( World::GetItem( ItemSlot_Clock ) != 0 )
    {
        AnimateAndCheckCollisions();
        return;
    }

    Direction origFacing = facing;

    MoveOrTeleport();
    TryShooting();

    if ( (flashTimer & 1) == 0 )
        AnimateAndCheckCollisions();
    SetFacingAnimation();
}

void BlueWizzrobe::Draw()
{
    if ( (flashTimer & 1) == 0 && facing != Dir_None )
    {
        int pal = CalcPalette( BluePal );
        animator.Draw( Sheet_Npcs, objX, objY, pal );
    }
}

void BlueWizzrobe::SetFacingAnimation()
{
    int dirOrd = Util::GetDirectionOrd( facing );
    animator.anim = Graphics::GetAnimation( Sheet_Npcs, wizzrobeAnimMap[dirOrd] );
}

void BlueWizzrobe::AnimateAndCheckCollisions()
{
    animator.Advance();

    // If I really wanted, I could make a friend function or class to do this, which is the same 
    // as in RedWizzrobe.

    invincibilityMask = 0xF6;
    if ( invincibilityTimer == 0 )
    {
        CheckWave( PlayerSwordShotSlot );
        CheckBombAndFire( BombSlot );
        CheckBombAndFire( BombSlot2 );
        CheckBombAndFire( FireSlot );
        CheckBombAndFire( FireSlot2 );
        CheckSword( PlayerSwordSlot );
    }
    CheckPlayerCollision();
}


//----------------------------------------------------------------------------
//  Lamnola
//----------------------------------------------------------------------------

Object* Lamnola::MakeSet( ObjType type )
{
    const int Y = 0x8D;

    for ( int i = 0; i < 5 * 2; i++ )
    {
        bool        isHead = (i == 4) || (i == 9);
        Lamnola*    lamnola = new Lamnola( type, isHead, 0x40, Y );
        World::SetObject( i, lamnola );
    }

    Lamnola*    head1 = (Lamnola*) World::GetObject( 4 );
    Lamnola*    head2 = (Lamnola*) World::GetObject( 9 );

    head1->facing = Dir_Up;
    head2->facing = Dir_Up;

    World::Get()->SetRoomObjCount( 8 );

    return World::GetObject( 0 );
}

Lamnola::Lamnola( ObjType type, bool isHead, int x, int y )
    :   Object( type )
{
    decoration = 0;

    objX = x;
    objY = y;

    if ( isHead )
        image.anim = Graphics::GetAnimation( Sheet_Npcs, Anim_UW_LanmolaHead );
    else
        image.anim = Graphics::GetAnimation( Sheet_Npcs, Anim_UW_LanmolaBody );
}

void Lamnola::Update()
{
    if ( facing == Dir_None )
        return;

    if ( World::GetItem( ItemSlot_Clock ) == 0 )
    {
        int speed = GetType() - Obj_RedLamnola + 1;
        int slot = World::GetCurrentObjectSlot();

        Util::MoveSimple( objX, objY, facing, speed );

        if ( slot == HeadSlot1 || slot == HeadSlot2 )
            UpdateHead();
    }

    CheckLamnolaCollisions();
}

void Lamnola::Draw()
{
    int pal = GetType() == Obj_RedLamnola ? RedPal : BluePal;
    pal = CalcPalette( pal );
    int xOffset = (16 - image.anim->width) / 2;
    image.Draw( Sheet_Npcs, objX + xOffset, objY, pal );
}

void Lamnola::UpdateHead()
{
    const uint Adjustment = 3;

    if ( (objX & 7) != 0 || ((objY + Adjustment) & 7) != 0 )
        return;

    int slot = World::GetCurrentObjectSlot();

    for ( int i = slot - 4; i < slot; i++ )
    {
        Lamnola* lamnola1 = (Lamnola*) World::GetObject( i );
        Lamnola* lamnola2 = (Lamnola*) World::GetObject( i+1 );

        if ( lamnola1 != nullptr && lamnola2 != nullptr )
            lamnola1->facing = lamnola2->facing;
    }

    if ( (objX & 0xF) != 0 || ((objY + Adjustment) & 0xF) != 0 )
        return;

    Turn();
}

void Lamnola::Turn()
{
    Direction   oppositeDir = Util::GetOppositeDir( facing );
    uint32_t    dirMask = ~oppositeDir;
    int         r = Util::GetRandom( 256 );
    uint32_t    dir;

    if ( r < 128 )
    {
        Direction   xDir = GetXDirToTruePlayer( objX );
        Direction   yDir = GetYDirToTruePlayer( objY );

        if ( (xDir & dirMask) == 0 || (xDir & facing) == 0 )
            dir = yDir;
        else
            dir = xDir;
    }
    else
    {
        dir = facing;
        r = Util::GetRandom( 256 );

        if ( r < 128 )
        {
            while ( true )
            {
                dir = dir >> 1;
                if ( dir == 0 )
                    dir = Dir_Up;

                if ( (dir & dirMask) != 0 )
                {
                    if ( r >= 64 )
                        break;
                    r = 64;
                }
            }
        }
    }

    while ( true )
    {
        facing = (Direction) dir;

        if ( Dir_None != CheckWorldMargin( facing )
            && !World::CollidesWithTileMoving( objX, objY, facing, false ) )
            break;

        // If there were a room that had lamnolas, and they could get surrounded on 3 sides, 
        // then this would get stuck in an infinite loop. But, the only room with that configuration 
        // has those blocks blocked off with a push block, which can only be pushed after all foes 
        // are killed.

        do
        {
            dir = dir >> 1;
            if ( dir == 0 )
                dir = Dir_Up;
        } while ( (dir & dirMask) == 0 );
    }
}

void Lamnola::CheckLamnolaCollisions()
{
    Direction origFacing = facing;
    CheckCollisions();
    facing = origFacing;

    if ( decoration == 0 )
        return;

    int slot = World::GetCurrentObjectSlot();
    Object* obj = nullptr;

    if ( slot >= TailSlot2 )
        slot = TailSlot2;
    else
        slot = TailSlot1;

    for ( ; ; slot++ )
    {
        obj = World::GetObject( slot );
        if ( obj != nullptr && obj->GetType() == GetType() )
            break;
    }

    if ( slot == HeadSlot1 || slot == HeadSlot2 )
        return;

    hp = 0x20;
    shoveDir = 0;
    shoveDistance = 0;
    decoration = 0;

    DeadDummy* dummy = new DeadDummy( objX, objY );
    World::SetObject( slot, dummy );
}


//----------------------------------------------------------------------------
//  Wallmaster
//----------------------------------------------------------------------------

static const uint8_t wallmasterDirs[] = 
{ 
    0x01, 0x01, 0x08, 0x08, 0x08, 0x02, 0x02, 0x02, 
    0xC1, 0xC1, 0xC4, 0xC4, 0xC4, 0xC2, 0xC2, 0xC2, 
    0x42, 0x42, 0x48, 0x48, 0x48, 0x41, 0x41, 0x41,
    0x82, 0x82, 0x84, 0x84, 0x84, 0x81, 0x81, 0x81,
    0xC4, 0xC4, 0xC2, 0xC2, 0xC2, 0xC8, 0xC8, 0xC8, 
    0x84, 0x84, 0x81, 0x81, 0x81, 0x88, 0x88, 0x88,
    0x48, 0x48, 0x42, 0x42, 0x42, 0x44, 0x44, 0x44, 
    0x08, 0x08, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04
};


Wallmaster::Wallmaster()
    :   Object( Obj_Wallmaster ),
        state( 0 ),
        dirIndex( 0 ),
        tilesCrossed( 0 ),
        holdingPlayer( false )
{
    decoration = 0;
    objTimer = 0;

    animator.durationFrames = 16;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Npcs, Anim_UW_Wallmaster );
}

void Wallmaster::CalcStartPosition( 
    int playerOrthoCoord, int playerCoord, int dir, 
    int baseDirIndex, int leastCoord, int& orthoCoord, int& coordIndex )
{
    Player* player = World::GetPlayer();
    int     offset = 0x24;

    dirIndex = baseDirIndex;
    if ( player->GetMoving() != 0 )
        offset = 0x32;
    if ( player->GetFacing() == dir )
    {
        dirIndex += 8;
        offset = -offset;
    }
    orthoCoord = playerOrthoCoord + offset;
    coordIndex = 0;
    if ( playerCoord != leastCoord )
    {
        dirIndex += 0x10;
        coordIndex++;
    }
}

void Wallmaster::Update()
{
    if ( state == 0 )
        UpdateIdle();
    else
        UpdateMoving();
}

void Wallmaster::Draw()
{
    if ( state != 0 )
    {
        int flags = wallmasterDirs[dirIndex] >> 6;
        int pal = CalcPalette( BluePal );

        if ( holdingPlayer )
            animator.DrawFrame( Sheet_Npcs, objX, objY, pal, 1, flags );
        else
            animator.Draw( Sheet_Npcs, objX, objY, pal, flags );
    }
}

void Wallmaster::UpdateIdle()
{
    if ( World::GetObjectTimer( MonsterSlot1 ) != 0 )
        return;

    Player* player = World::GetPlayer();

    if ( player->GetState() == Player::Paused )
        return;

    int     playerX = player->GetX();
    int     playerY = player->GetY();

    if ( playerX < 0x29 || playerX >= 0xC8 )
    {
        if ( playerY < 0x6D || playerY >= 0xB5 )
            return;
    }

    static const uint8_t startXs[] = { 0x00, 0xF0 };
    static const uint8_t startYs[] = { 0x3D, 0xDD };

    const int LeastY = 0x5D;
    const int MostY = 0xBD;

    if ( playerX == 0x20 || playerX == 0xD0 )
    {
        int y;
        int xIndex;
        CalcStartPosition( playerY, playerX, Dir_Up, 0, 0x20, y, xIndex );
        objX = startXs[xIndex];
        objY = y;
    }
    else if ( playerY == LeastY || playerY == MostY )
    {
        int x;
        int yIndex;
        CalcStartPosition( playerX, playerY, Dir_Left, 0x20, LeastY, x, yIndex );
        objY = startYs[yIndex];
        objX = x;
    }
    else
        return;

    state = 1;
    tilesCrossed = 0;
    World::SetObjectTimer( MonsterSlot1, 0x60 );
    facing = (Direction) (wallmasterDirs[dirIndex] & 0xF);
    tileOffset = 0;
}

void Wallmaster::UpdateMoving()
{
    Player* player = World::GetPlayer();

    if ( shoveDir != 0 )
    {
        ObjShove();
    }
    else if ( !IsStunned() )
    {
        ObjMoveDir( 0x18, facing );

        if ( tileOffset == 0x10 || tileOffset == -0x10 )
        {
            tileOffset = 0;
            dirIndex++;
            tilesCrossed++;
            facing = (Direction) (wallmasterDirs[dirIndex] & 0xF);

            if ( tilesCrossed >= 7 )
            {
                state = 0;
                if ( holdingPlayer )
                {
                    player->SetState( Player::Idle );
                    World::UnfurlLevel();
                }
                return;
            }
        }
    }

    if ( holdingPlayer )
    {
        player->SetX( objX );
        player->SetY( objY );
        player->GetAnimator()->Advance();
    }
    else
    {
        if ( CheckCollisions() )
        {
            holdingPlayer = true;
            player->SetState( Player::Paused );
            player->ResetShove();
            objFlags.SetDrawAbovePlayer();
        }
        animator.Advance();
    }
}


//----------------------------------------------------------------------------
//  Aquamentus
//----------------------------------------------------------------------------

Aquamentus::Aquamentus()
    :   Object( Obj_Aquamentus ),
        distance( 0 )
{
    invincibilityMask = 0xE2;
    objX = 0xB0;
    objY = 0x80;

    Sound::PlayEffect( SEffect_boss_roar1, true, Sound::AmbientInstance );

    animator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B1_Aquamentus );
    animator.durationFrames = 32;
    animator.time = 0;

    mouthImage.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B1_Aquamentus_Mouth_Closed );

    static const uint8_t palette[] = { 0, 0x0A, 0x29, 0x30 };
    Graphics::SetPaletteIndexed( LevelFgPalette, palette );
    Graphics::UpdatePalettes();
}

void Aquamentus::Update()
{
    if ( World::GetItem( ItemSlot_Clock ) == 0 )
    {
        Move();
        TryShooting();
    }
    Animate();
    CheckCollisions();
    PlayBossHitSoundIfHit();
    PlayBossHitSoundIfDied();
    shoveDir = 0;
    shoveDistance = 0;
}

void Aquamentus::Draw()
{
    int pal = CalcPalette( SeaPal );
    animator.Draw( Sheet_Boss, objX, objY, pal );
    mouthImage.Draw( Sheet_Boss, objX, objY, pal );
}

void Aquamentus::Move()
{
    if ( distance == 0 )
    {
        int r = Util::GetRandom( 16 );
        distance = r | 7;
        facing = (Direction) ((r & 1) + 1);
        return;
    }

    if ( (GetFrameCounter() & 7) != 0 )
        return;

    if ( objX < 0x88 )
    {
        objX = 0x88;
        facing = Dir_Right;
        distance = 7;
    }
    else if ( objX >= 0xC8 )
    {
        objX = 0xC7;
        facing = Dir_Left;
        distance = 7;
    }

    if ( facing == Dir_Right )
        objX++;
    else
        objX--;

    distance--;
}

void Aquamentus::TryShooting()
{
    if ( objTimer == 0 )
    {
        int r = Util::GetRandom( 256 );
        objTimer = r | 0x70;

        int8_t  yOffsets[] = { 1, 0, -1 };

        for ( int i = 0; i < 3; i++ )
        {
            int slot = World::FindEmptyMonsterSlot();
            if ( slot < 0 )
                break;

            ShootFireball( Obj_Fireball, objX, objY );
            fireballOffsets[slot] = yOffsets[i];
        }
    }
    else
    {
        for ( int i = 0; i < MaxMonsters; i++ )
        {
            Object* obj = World::GetObject( i );

            if ( obj == nullptr || obj->GetType() != Obj_Fireball )
                continue;
            if ( (GetFrameCounter() & 1) == 1 )
                continue;

            Fireball* fireball = (Fireball*) obj;
            fireball->SetY( fireball->GetY() + fireballOffsets[i] );
        }
    }
}

void Aquamentus::Animate()
{
    int mouthAnimIndex;

    if ( objTimer < 0x20 )
        mouthAnimIndex = Anim_B1_Aquamentus_Mouth_Open;
    else
        mouthAnimIndex = Anim_B1_Aquamentus_Mouth_Closed;

    mouthImage.anim = Graphics::GetAnimation( Sheet_Boss, mouthAnimIndex );
    animator.Advance();
}


//----------------------------------------------------------------------------
//  Dodongo
//----------------------------------------------------------------------------

static const uint8_t dodongoWalkAnimMap[DirCount] = 
{
    Anim_B1_Dodongo_R,
    Anim_B1_Dodongo_L,
    Anim_B1_Dodongo_D,
    Anim_B1_Dodongo_U
};

static const uint8_t dodongoBloatAnimMap[DirCount] = 
{
    Anim_B1_Dodongo_Bloated_R,
    Anim_B1_Dodongo_Bloated_L,
    Anim_B1_Dodongo_Bloated_D,
    Anim_B1_Dodongo_Bloated_U
};

static const WalkerSpec dodongoWalkSpec = 
{
    dodongoWalkAnimMap,
    20,
    RedPal,
    StdSpeed,
    Obj_None
};

Dodongo::StateFunc Dodongo::sStateFuncs[] = 
{
    &Dodongo::UpdateMoveState,
    &Dodongo::UpdateBloatedState,
    &Dodongo::UpdateStunnedState,
};

Dodongo::StateFunc Dodongo::sBloatedSubstateFuncs[] = 
{
    &Dodongo::UpdateBloatedWait,
    &Dodongo::UpdateBloatedWait,
    &Dodongo::UpdateBloatedWait,
    &Dodongo::UpdateBloatedDie,
    &Dodongo::UpdateBloatedEnd,
};


Dodongo::Dodongo( int x, int y )
    :   Wanderer( Obj_1Dodongo, &dodongoWalkSpec, 0x20, x, y ),
        state( 0 ),
        bloatedSubstate( 0 ),
        bloatedTimer( 0 ),
        bombHits( 0 )
{
    Sound::PlayEffect( SEffect_boss_roar2, true, Sound::AmbientInstance );
    int r = Util::GetRandom( 2 );
    if ( r == 1 )
        facing = Dir_Left;
    else
        facing = Dir_Right;

    animator.durationFrames = 16;
    animator.time = 0;
    SetWalkAnimation();

    static const uint8_t palette[] = { 0, 0x17, 0x27, 0x30 };
    Graphics::SetPaletteIndexed( LevelFgPalette, palette );
    Graphics::UpdatePalettes();
}

void Dodongo::Update()
{
    UpdateState();
    CheckPlayerHit();
    CheckBombHit();
    Animate();
}

void Dodongo::Draw()
{
    if ( state == 1 && (bloatedSubstate == 2 || bloatedSubstate == 3) )
    {
        if ( (GetFrameCounter() & 2) == 0 )
            return;
    }

    animator.Draw( Sheet_Boss, objX, objY, LevelFgPalette );
}

void Dodongo::SetWalkAnimation()
{
    int dirOrd = Util::GetDirectionOrd( facing );
    animator.anim = Graphics::GetAnimation( Sheet_Boss, dodongoWalkAnimMap[dirOrd] );
}

void Dodongo::SetBloatAnimation()
{
    int dirOrd = Util::GetDirectionOrd( facing );
    animator.anim = Graphics::GetAnimation( Sheet_Boss, dodongoBloatAnimMap[dirOrd] );
}

void Dodongo::UpdateState()
{
    StateFunc   func = sStateFuncs[state];
    (this->*func)();
}

void Dodongo::CheckPlayerHit()
{
    CheckPlayerHitStdSize();
    if ( invincibilityTimer == 0 )
    {
        if ( Util::IsVertical( facing ) )
            return;

        objX += 0x10;
        CheckPlayerHitStdSize();
        objX -= 0x10;

        if ( invincibilityTimer == 0 )
            return;
    }

    UpdateBloatedDie();
    World::Get()->SetBombItemDrop();
}

void Dodongo::CheckPlayerHitStdSize()
{
    invincibilityMask = 0xFF;
    CheckCollisions();

    if ( state == 2 )
    {
        invincibilityMask = 0xFE;
        CheckSword( PlayerSwordSlot );
    }
}

void Dodongo::CheckBombHit()
{
    if ( state != 0 )
        return;

    Bomb* bomb = (Bomb*) World::GetObject( FirstBombSlot );
    if ( bomb == nullptr || bomb->IsDeleted() )
        return;

    Bomb::State bombState = bomb->GetLifetimeState();
    int bombX = bomb->GetX() + 8;
    int bombY = bomb->GetY() + 8;
    int thisX = objX + 8;
    int thisY = objY + 8;

    if ( Util::IsHorizontal( facing ) )
        thisX += 8;

    int xDist = thisX - bombX;
    int yDist = thisY - bombY;

    if ( bombState == Bomb::Ticking )
    {
        CheckTickingBombHit( bomb, xDist, yDist );
    }
    else    // Blasting or Fading
    {
        if ( Overlaps( xDist, yDist, 0 ) )
            state = 2;
    }
}

void Dodongo::CheckTickingBombHit( Bomb* bomb, int xDist, int yDist )
{
    if ( !Overlaps( xDist, yDist, 1 ) )
        return;

    static const int negBounds[] = { -0x10, 0, -8, 0, -8, -4, -4, -0x10, 0, 0 };
    static const int posBounds[] = { 0, 0x10, 8, 0, 8, 4, 4, 0, 0, 0x10 };

    int index = facing >> 1;
    int dist = xDist;

    for ( int i = 0; i < 2; i++ )
    {
        if (   dist <  negBounds[index] 
            || dist >= posBounds[index] )
            return;

        index += 5;
        dist = yDist;
    }

    state++;
    bloatedSubstate = 0;
    bomb->SetDeleted();
}

bool Dodongo::Overlaps( int xDist, int yDist, int boundsIndex )
{
    static const int posBounds[] = {  0xC,  0x11 };
    static const int negBounds[] = { -0xC, -0x10 };

    int distances[2] = { xDist, yDist };

    for ( int i = 1; i >= 0; i-- )
    {
        if (   distances[i] >= posBounds[boundsIndex]
            || distances[i] <  negBounds[boundsIndex] )
            return false;
    }

    return true;
}

void Dodongo::Animate()
{
    if ( state == 0 )
        animator.SetDuration( 16 );
    else
        animator.SetDuration( 64 );

    if ( state == 0 || state == 2 || bloatedSubstate == 0 )
        SetWalkAnimation();
    else
        SetBloatAnimation();

    animator.Advance();
}

void Dodongo::UpdateMoveState()
{
    Direction   origFacing = facing;
    int         xOffset = 0;

    if ( facing != Dir_Left )
    {
        objX += 0x10;
        xOffset = 0x10;
    }

    Wanderer::Move();

    objX -= xOffset;
    if ( objX < 0x20 )
        facing = Dir_Right;

    if ( facing != origFacing )
        SetWalkAnimation();
}

void Dodongo::UpdateBloatedState()
{
    StateFunc func = sBloatedSubstateFuncs[bloatedSubstate];
    (this->*func)();
}

void Dodongo::UpdateStunnedState()
{
    if ( stunTimer == 0 )
    {
        stunTimer = 0x20;
    }
    else if ( stunTimer == 1 )
    {
        state = 0;
        bloatedSubstate = 0;
    }
}

void Dodongo::UpdateBloatedWait()
{
    static const int waitTimes[] = { 0x20, 0x40, 0x40 };

    if ( bloatedTimer == 0 )
    {
        bloatedTimer = waitTimes[bloatedSubstate];
        if ( bloatedSubstate == 0 )
        {
            Bomb* bomb = (Bomb*) World::GetObject( FirstBombSlot );
            if ( bomb != nullptr )
                bomb->SetDeleted();
            bombHits++;
        }
    }
    else if ( bloatedTimer == 1 )
    {
        bloatedSubstate++;
        if ( bloatedSubstate >= 2 && bombHits < 2 )
            bloatedSubstate = 4;
    }

    bloatedTimer--;
}

void Dodongo::UpdateBloatedDie()
{
    Sound::PlayEffect( SEffect_monster_die );
    Sound::PlayEffect( SEffect_boss_hit );
    Sound::StopEffect( Sound::AmbientInstance );
    decoration = 0x10;
    state = 0;
    bloatedSubstate = 0;
}

void Dodongo::UpdateBloatedEnd()
{
    state = 0;
    bloatedSubstate = 0;
}


//----------------------------------------------------------------------------
//  Manhandla
//----------------------------------------------------------------------------

static const uint8_t manhandlaAnimMap[5] = 
{
    Anim_B2_Manhandla_Hand_U,
    Anim_B2_Manhandla_Hand_D,
    Anim_B2_Manhandla_Hand_L,
    Anim_B2_Manhandla_Hand_R,
    Anim_B2_Manhandla_Body,
};

int         Manhandla::sPartsDied;
Direction   Manhandla::sFacingAtFrameBegin;
Direction   Manhandla::sBounceDir;


Object* Manhandla::Make( int x, int y )
{
    static const int xOffsets[] = { 0, 0, -0x10, 0x10, 0 };
    static const int yOffsets[] = { -0x10, 0x10, 0, 0, 0 };

    int         r = Util::GetRandom( 8 );
    Direction   dir = Util::GetDirection8( r );

    Sound::PlayEffect( SEffect_boss_roar3, true, Sound::AmbientInstance );

    for ( int i = 0; i < 5; i++ )
    {
        // ORIGINAL: Get the base X and Y from the fifth spawn spot.
        int xPos = x + xOffsets[i];
        int yPos = y + yOffsets[i];

        Manhandla* manhandla = new Manhandla( i, xPos, yPos, dir );

        World::SetObject( i, manhandla );
    }

    return World::GetObject( 0 );
}

Manhandla::Manhandla( int index, int x, int y, Direction facing )
    :   Object( Obj_Manhandla ),
        curSpeedFix( 0x80 ),
        speedAccum( 0 ),
        frameAccum( 0 ),
        frame( 0 ),
        oldFrame( 0 )
{
    objX = x;
    objY = y;
    invincibilityMask = 0xE2;
    decoration = 0;
    this->facing = facing;

    animator.durationFrames = 1;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Boss, manhandlaAnimMap[index] );
}

void Manhandla::SetPartFacings( Direction dir )
{
    for ( int i = 0; i < 5; i++ )
    {
        Manhandla* manhandla = (Manhandla*) World::GetObject( i );
        if ( manhandla != nullptr )
            manhandla->facing = dir;
    }
}

void Manhandla::Update()
{
    int slot = World::GetCurrentObjectSlot();

    if ( slot == 4 )
    {
        UpdateBody();
        sFacingAtFrameBegin = facing;
    }

    Move();
    CheckManhandlaCollisions();

    if ( facing != sFacingAtFrameBegin )
        sBounceDir = facing;

    frame = (frameAccum & 0x10) >> 4;

    if ( slot != 4 )
        TryShooting();
}

void Manhandla::Draw()
{
    int slot = World::GetCurrentObjectSlot();
    int pal = CalcPalette( BluePal );

    if ( slot == 4 )
    {
        animator.Draw( Sheet_Boss, objX, objY, pal );
    }
    else
    {
        animator.DrawFrame( Sheet_Boss, objX, objY, pal, frame );
    }
}

void Manhandla::UpdateBody()
{
    if ( sPartsDied != 0 )
    {
        for ( int i = 0; i < 5; i++ )
        {
            Manhandla* manhandla = (Manhandla*) World::GetObject( i );
            if ( manhandla != nullptr )
                manhandla->curSpeedFix += 0x80;
        }
        sPartsDied = 0;
    }

    if ( sBounceDir != Dir_None )
    {
        SetPartFacings( sBounceDir );
        sBounceDir = Dir_None;
    }

    assert( World::GetCurrentObjectSlot() == MonsterSlot1 + 4 );

    if ( objTimer == 0 )
    {
        objTimer = 0x10;

        int r = Util::GetRandom( 2 );
        if ( r == 0 )
            facing = TurnRandomly8( facing );
        else
            facing = TurnTowardsPlayer8( objX, objY, facing );

        // The original game set sBounceDir = facing here, instead of to Dir_None above.
        SetPartFacings( facing );
    }
}

void Manhandla::Move()
{
    speedAccum &= 0xFF;
    speedAccum += (curSpeedFix & 0xFFE0);
    int speed = speedAccum >> 8;

    Util::MoveSimple8( objX, objY, facing, speed );

    frameAccum += Util::GetRandom( 4 ) + speed;

    if ( Dir_None == CheckWorldMargin( facing ) )
    {
        facing = Util::GetOppositeDir8( facing );
    }
}

void Manhandla::TryShooting()
{
    if ( frame != oldFrame )
    {
        oldFrame = frame;

        if ( frame == 0 
            && Util::GetRandom( 256 ) >= 0xE0 
            && World::GetObject( 6 ) == nullptr )
        {
            ShootFireball( Obj_Fireball2, objX, objY );
        }
    }
}

void Manhandla::CheckManhandlaCollisions()
{
    int objSlot = World::GetCurrentObjectSlot();

    Direction origFacing = facing;
    int origStateTimer = objTimer;

    CheckCollisions();

    objTimer = origStateTimer;
    facing = origFacing;

    if ( objSlot == 4 )
        invincibilityTimer = 0;

    PlayBossHitSoundIfHit();

    if ( decoration == 0 )
        return;

    shoveDir = 0;
    shoveDistance = 0;

    if ( objSlot == 4 )
    {
        decoration = 0;
        return;
    }

    int handCount = 0;

    for ( int i = MonsterSlot1; i < MonsterSlot1 + 4; i++ )
    {
        Object* obj = World::GetObject( i );
        if ( obj != nullptr && obj->GetType() == Obj_Manhandla )
            handCount++;
    }

    DeadDummy* dummy = new DeadDummy( objX, objY );
    dummy->SetDecoration( decoration );

    if ( handCount > 1 )
    {
        World::SetObject( objSlot, dummy );
    }
    else
    {
        Sound::PlayEffect( SEffect_boss_hit );
        Sound::StopEffect( Sound::AmbientInstance );
        World::SetObject( 4, dummy );
    }

    sPartsDied++;
}

void Manhandla::ClearRoomData()
{
    sPartsDied = 0;
    sFacingAtFrameBegin = Dir_None;
    sBounceDir = Dir_None;
}


//----------------------------------------------------------------------------
//  DigdoggerBase
//----------------------------------------------------------------------------

DigdoggerBase::DigdoggerBase( ObjType type, int x, int y )
    :   Object( type ),
        curSpeedFix( 0x003F ),
        speedAccum( 0 ),
        targetSpeedFix( 0x0080 ),
        accelDir( 0 ),
        isChild( type == Obj_LittleDigdogger )
{
    objX = x;
    objY = y;

    int r = Util::GetRandom( 8 );
    facing = Util::GetDirection8( r );

    Sound::PlayEffect( SEffect_boss_roar3, true, Sound::AmbientInstance );
}

void DigdoggerBase::UpdateMove()
{
    if ( objTimer == 0 )
    {
        objTimer = 0x10;

        int r = Util::GetRandom( 2 );
        if ( r == 0 )
            facing = TurnRandomly8( facing );
        else
            facing = TurnTowardsPlayer8( objX, objY, facing );
    }

    Accelerate();
    Move();
}

void DigdoggerBase::Move()
{
    speedAccum &= 0xFF;
    speedAccum += (curSpeedFix & 0xFFE0);
    int speed = speedAccum >> 8;

    Util::MoveSimple8( objX, objY, facing, speed );
}

void DigdoggerBase::Accelerate()
{
    if ( accelDir == 0 )
        IncreaseSpeed();
    else
        DecreaseSpeed();
}

void DigdoggerBase::IncreaseSpeed()
{
    curSpeedFix++;

    if ( curSpeedFix != targetSpeedFix )
        return;

    accelDir++;
    targetSpeedFix = 0x0040;

    if ( isChild )
        targetSpeedFix += 0x0100;
}

void DigdoggerBase::DecreaseSpeed()
{
    curSpeedFix--;

    if ( curSpeedFix != targetSpeedFix )
        return;

    accelDir--;
    targetSpeedFix = 0x0080;

    if ( isChild )
        targetSpeedFix += 0x0100;
}


//----------------------------------------------------------------------------
//  Digdogger
//----------------------------------------------------------------------------

Object* Digdogger::Make( int x, int y, int childCount )
{
    Digdogger* digdogger = new Digdogger( x, y, childCount );
    return digdogger;
}

Digdogger::Digdogger( int x, int y, int childCount )
    :   DigdoggerBase( Obj_Digdogger1, x, y ),
        childCount( childCount ),
        updateBig( true )
{
    animator.durationFrames = 12;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B1_Digdogger_Big );

    littleAnimator.durationFrames = 12;
    littleAnimator.time = 0;
    littleAnimator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B1_Digdogger_Little );

    static const uint8_t palette[] = { 0, 0x17, 0x27, 0x30 };
    Graphics::SetPaletteIndexed( LevelFgPalette, palette );
    Graphics::UpdatePalettes();
}

void Digdogger::Update()
{
    if ( !IsStunned() )
    {
        if ( World::GetRecorderUsed() == 0 )
            DigdoggerBase::UpdateMove();
        else
            UpdateSplit();
    }

    static const int offsetsX[] = { 0, 0x10,     0, -0x10 };
    static const int offsetsY[] = { 0, 0x10, -0x10,  0x10 };

    if ( updateBig )
    {
        int x = objX;
        int y = objY;

        for ( int i = 0; i < 4; i++ )
        {
            objX += offsetsX[i];
            objY += offsetsY[i];

            if ( Dir_None == CheckWorldMargin( facing ) )
            {
                facing = Util::GetOppositeDir8( facing );
            }

            CheckCollisions();
        }

        objX = x;
        objY = y;

        animator.Advance();
    }

    littleAnimator.Advance();
}

void Digdogger::Draw()
{
    int pal = CalcPalette( LevelFgPalette );

    if ( updateBig )
    {
        animator.Draw( Sheet_Boss, objX, objY, pal );
    }
    littleAnimator.Draw( Sheet_Boss, objX + 8, objY + 8, pal );
}

void Digdogger::UpdateSplit()
{
    if ( World::GetRecorderUsed() == 1 )
    {
        objTimer = 0x40;
        World::SetRecorderUsed( 2 );
    }
    else
    {
        updateBig = false;

        if ( objTimer != 0 )
        {
            if ( (objTimer & 7) == 0 )
            {
                isChild = !isChild;
                if ( !isChild )
                    updateBig = true;
            }
        }
        else
        {
            World::SetRecorderUsed( 1 );
            World::Get()->SetRoomObjCount( childCount );
            for ( int i = 1; i <= childCount; i++ )
            {
                Object* child = DigdoggerChild::Make( objX, objY );
                World::SetObject( i, child );
            }
            Sound::PlayEffect( SEffect_boss_hit );
            Sound::StopEffect( Sound::AmbientInstance );
            isDeleted = true;
        }
    }
}


//----------------------------------------------------------------------------
//  DigdoggerChild
//----------------------------------------------------------------------------

Object* DigdoggerChild::Make( int x, int y )
{
    DigdoggerChild* digdogger = new DigdoggerChild( x, y );
    return digdogger;
}

DigdoggerChild::DigdoggerChild( int x, int y )
    :   DigdoggerBase( Obj_LittleDigdogger, x, y )
{
    targetSpeedFix = 0x0180;

    animator.durationFrames = 12;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B1_Digdogger_Little );
}

void DigdoggerChild::Update()
{
    if ( !IsStunned() )
    {
        DigdoggerBase::UpdateMove();
    }

    if ( Dir_None == CheckWorldMargin( facing ) )
    {
        facing = Util::GetOppositeDir8( facing );
    }

    CheckCollisions();
    animator.Advance();
}

void DigdoggerChild::Draw()
{
    int pal = CalcPalette( LevelFgPalette );
    animator.Draw( Sheet_Boss, objX, objY, pal );
}


//----------------------------------------------------------------------------
//  Gohma
//----------------------------------------------------------------------------

Gohma::Gohma( ObjType type )
    :   Object( type ),
        changeFacing( true ),
        speedAccum( 0 ),
        distance( 0 ),
        sprints( 0 ),
        startOpenEyeTimer( 0 ),
        eyeOpenTimer( 0 ),
        eyeClosedTimer( 0 ),
        shootTimer( 1 ),
        frame( 0 ),
        curCheckPart( 0 )
{
    Sound::PlayEffect( SEffect_boss_roar2, true, Sound::AmbientInstance );

    objX = 0x80;
    objY = 0x70;
    decoration = 0;
    invincibilityMask = 0xFB;

    animator.durationFrames = 1;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B2_Gohma_Eye_All );

    leftAnimator.durationFrames = 32;
    leftAnimator.time = 0;
    leftAnimator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B2_Gohma_Legs_L );

    rightAnimator.durationFrames = 32;
    rightAnimator.time = 0;
    rightAnimator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B2_Gohma_Legs_R );
}

int Gohma::GetCurrentCheckPart()
{
    return curCheckPart;
}

int Gohma::GetEyeFrame()
{
    return frame;
}

void Gohma::Update()
{
    if ( changeFacing )
        ChangeFacing();
    else
        Move();

    AnimateEye();
    TryShooting();
    CheckGohmaCollisions();

    leftAnimator.Advance();
    rightAnimator.Advance();
}

void Gohma::Draw()
{
    int pal = (GetType() == Obj_BlueGohma) ? BluePal : RedPal;
    pal = CalcPalette( pal );

    animator.DrawFrame( Sheet_Boss, objX, objY, pal, frame );
    leftAnimator.Draw( Sheet_Boss, objX - 0x10, objY, pal );
    rightAnimator.Draw( Sheet_Boss, objX + 0x10, objY, pal );
}

void Gohma::ChangeFacing()
{
    int dir = 1;
    int r = Util::GetRandom( 256 );

    if ( r < 0xB0 )
    {
        dir <<= 1;
        if ( r < 0x60 )
            dir <<= 1;
    }

    facing = (Direction) dir;
    changeFacing = false;
}

void Gohma::Move()
{
    speedAccum &= 0xFF;
    speedAccum += 0x80;

    if ( speedAccum >= 0x0100 )
    {
        distance++;
        Util::MoveSimple( objX, objY, facing, 1 );

        if ( distance == 0x20 )
        {
            distance = 0;
            facing = Util::GetOppositeDir( facing );

            sprints++;
            if ( (sprints & 1) == 0 )
                changeFacing = true;
        }
    }
}

void Gohma::AnimateEye()
{
    if ( startOpenEyeTimer == 0 )
    {
        eyeOpenTimer = 0x80;
        startOpenEyeTimer = 0xC0 | Util::GetRandom( 256 );
    }

    if ( (GetFrameCounter() & 1) == 1 )
    {
        startOpenEyeTimer--;
    }

    if ( eyeOpenTimer == 0 )
    {
        eyeClosedTimer++;
        if ( eyeClosedTimer == 8 )
        {
            eyeClosedTimer = 0;
            frame = (frame & 1) ^ 1;
        }
    }
    else
    {
        int t = eyeOpenTimer;
        eyeOpenTimer--;
        frame = 2;
        if ( t < 0x70 && t >= 0x10 )
            frame++;
    }
}

void Gohma::TryShooting()
{
    shootTimer--;
    if ( shootTimer == 0 )
    {
        shootTimer = 0x41;
        ShootFireball( Obj_Fireball2, objX, objY );
    }
}

void Gohma::CheckGohmaCollisions()
{
    int origX = objX;
    objX -= 0x10;

    for ( int i = 5; i > 0; i-- )
    {
        curCheckPart = i;
        // With other object types, we'd only call CheckCollisions. But, Gohma needs 
        // to pass down the index of the current part.
        CheckCollisions();
        objX += 8;
    }

    objX = origX;
}


//----------------------------------------------------------------------------
//  GleeokHead
//----------------------------------------------------------------------------

static const uint8_t gleeokHeadAnimMap[DirCount] = 
{
    Anim_B2_Gleeok_Head2,
    Anim_B2_Gleeok_Head2,
    Anim_B2_Gleeok_Head2,
    Anim_B2_Gleeok_Head2,
};

static const FlyerSpec gleeokHeadSpec = 
{
    gleeokHeadAnimMap,
    Sheet_Boss,
    RedPal,
    0xE0
};


GleeokHead::GleeokHead( int x, int y )
    :   Flyer( Obj_GleeokHead, &gleeokHeadSpec, x, y )
{
    int r = Util::GetRandom( 8 );
    facing = Util::GetDirection8( r );

    curSpeed = 0xBF;
    invincibilityMask = 0xFF;
}

void GleeokHead::Update()
{
    UpdateStateAndMove();

    int r = Util::GetRandom( 256 );

    if ( r < 0x20
        && (moveCounter & 1) == 0
        && World::GetObject( LastMonsterSlot ) == nullptr )
    {
        ShootFireball( Obj_Fireball2, GetX(), GetY() );
    }

    CheckCollisions();
    decoration = 0;
    shoveDir = 0;
    shoveDistance = 0;
    invincibilityTimer = 0;
}

void GleeokHead::UpdateFullSpeedImpl()
{
    int nextState = 2;
    int r = Util::GetRandom( 256 );

    if ( r >= 0xD0 )
        nextState++;

    GoToState( nextState, 6 );
}


//----------------------------------------------------------------------------
//  Gleeok
//----------------------------------------------------------------------------

Gleeok::Gleeok( ObjType type, int x, int y )
    :   Object( type ),
        writhingTimer( 0 ),
        neckCount( type - Obj_Gleeok1 + 1 )
{
    objX = 0x74;
    objY = 0x57;
    decoration = 0;
    invincibilityMask = 0xFE;

    animator.durationFrames = NormalAnimFrames;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B2_Gleeok_Body );

    for ( int i = 0; i < neckCount; i++ )
    {
        necks[i].Init( i );
    }

    static const uint8_t palette[] = { 0, 0x2A, 0x1A, 0x0C };
    Graphics::SetPaletteIndexed( LevelFgPalette, palette );
    Graphics::UpdatePalettes();

    Sound::PlayEffect( SEffect_boss_roar1, true, Sound::AmbientInstance );
}

void Gleeok::Update()
{
    Animate();

    for ( int i = 0; i < neckCount; i++ )
    {
        if ( !necks[i].IsAlive() )
            continue;

        if ( (GetFrameCounter() % MaxNecks) == i )
            necks[i].Update();

        CheckNeckCollisions( i );
    }
}

void Gleeok::Draw()
{
    int pal = CalcPalette( SeaPal );
    animator.Draw( Sheet_Boss, objX, objY, pal );

    for ( int i = 0; i < neckCount; i++ )
    {
        if ( necks[i].IsAlive() )
            necks[i].Draw();
    }
}

void Gleeok::Animate()
{
    animator.Advance();

    if ( writhingTimer != 0 )
    {
        writhingTimer--;
        if ( writhingTimer == 0 )
        {
            animator.durationFrames = NormalAnimFrames;
            animator.time = 0;
        }
    }
}

void Gleeok::CheckNeckCollisions( int index )
{
    GleeokNeck* neck = &necks[index];
    int   partIndexes[] = { 0, GleeokNeck::HeadIndex };
    int   origX = objX;
    int   origY = objY;
    int   bodyDecoration = 0;

    for ( int i = 0; i < 2; i++ )
    {
        int   partIndex = partIndexes[i];
        Point loc = neck->GetPartLocation( partIndex );

        objX = loc.X;
        objY = loc.Y;
        hp = neck->GetHP();

        CheckCollisions();

        neck->SetHP( hp );

        if ( shoveDir != 0 )
        {
            writhingTimer = TotalWrithingFrames;
            animator.durationFrames = WrithingAnimFrames;
            animator.time = 0;
        }

        shoveDir = 0;
        shoveDistance = 0;

        if ( partIndex != GleeokNeck::HeadIndex )
        {
            decoration = 0;
        }
        else
        {
            PlayBossHitSoundIfHit();

            if ( decoration != 0 )
            {
                neck->SetDead();

                int slot = MonsterSlot1 + index + 6;
                GleeokHead* head = new GleeokHead( objX, objY );
                World::SetObject( slot, head );

                int aliveCount = 0;
                for ( int i = 0; i < neckCount; i++ )
                {
                    if ( necks[i].IsAlive() )
                        aliveCount++;
                }

                if ( aliveCount == 0 )
                {
                    Sound::PlayEffect( SEffect_boss_hit );
                    Sound::StopEffect( Sound::AmbientInstance );

                    bodyDecoration = 0x11;
                    // Don't include the last slot, which is used for fireballs.
                    for ( int i = MonsterSlot1 + 1; i < LastMonsterSlot; i++ )
                    {
                        World::SetObject( i, nullptr );
                    }
                }
            }
        }
    }

    objY = origY;
    objX = origX;
    decoration = bodyDecoration;
}


//----------------------------------------------------------------------------
//  GleeokNeck
//----------------------------------------------------------------------------

bool GleeokNeck::IsAlive()
{
    return isAlive;
}

void GleeokNeck::SetDead()
{
    isAlive = false;
}

int GleeokNeck::GetHP()
{
    return hp;
}

void GleeokNeck::SetHP( int value )
{
    hp = value;
}

Point GleeokNeck::GetPartLocation( int partIndex )
{
    Point p = { parts[partIndex].x, parts[partIndex].y };
    return p;
}

void GleeokNeck::Init( int index )
{
    static const uint8_t startYs[] = 
    { 0x6F, 0x74, 0x79, 0x7E, 0x83 };

    for ( int i = 0; i < MaxParts; i++ )
    {
        parts[i].x = 0x7C;
        parts[i].y = startYs[i];
    }

    isAlive = true;
    hp = 0xA0;
    changeXDirTimer = 6;
    changeYDirTimer = 3;
    xSpeed = 1;
    ySpeed = 1;
    changeDirsTimer = 0;
    startHeadTimer = 0;

    if ( index == 0 || index == 2 )
        xSpeed = -1;
    else
        ySpeed = -1;

    switch ( index )
    {
    case 1: startHeadTimer = 12; break;
    case 2: startHeadTimer = 24; break;
    case 3: startHeadTimer = 36; break;
    }

    neckImage.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B2_Gleeok_Neck );
    headImage.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B2_Gleeok_Head );
}

void GleeokNeck::Update()
{
    MoveNeck();
    MoveHead();
    TryShooting();
}

void GleeokNeck::Draw()
{
    for ( int i = 0; i < HeadIndex; i++ )
    {
        neckImage.Draw( Sheet_Boss, parts[i].x, parts[i].y, SeaPal );
    }

    headImage.Draw( Sheet_Boss, parts[HeadIndex].x, parts[HeadIndex].y, SeaPal );
}

void GleeokNeck::MoveHead()
{
    if ( startHeadTimer != 0 )
    {
        startHeadTimer--;
        return;
    }

    parts[HeadIndex].x += xSpeed;
    parts[HeadIndex].y += ySpeed;

    changeDirsTimer++;
    if ( changeDirsTimer < 4 )
        return;
    changeDirsTimer = 0;

    changeXDirTimer++;
    if ( changeXDirTimer >= 0xC )
    {
        changeXDirTimer = 0;
        xSpeed = -xSpeed;
    }

    changeYDirTimer++;
    if ( changeYDirTimer >= 6 )
    {
        changeYDirTimer = 0;
        ySpeed = -ySpeed;
    }
}

void GleeokNeck::TryShooting()
{
    int r = Util::GetRandom( 256 );
    if (   r < 0x20 
        && World::GetObject( LastMonsterSlot ) == nullptr )
    {
        ShootFireball( Obj_Fireball2, parts[ShooterIndex].x, parts[ShooterIndex].y );
    }
}

void GleeokNeck::MoveNeck()
{
    Limits xLimits = { 0 };
    Limits yLimits = { 0 };

    int headToEndXDiv4 = (parts[4].x - parts[0].x) / 4;
    int headToEndXDiv4Abs = abs( headToEndXDiv4 );
    GetLimits( headToEndXDiv4Abs, xLimits );

    int headToEndYDiv4Abs = abs( parts[4].y - parts[0].y ) / 4;
    GetLimits( headToEndYDiv4Abs, yLimits );

    int distance;

    // If passed the capped high limit X or Y from previous part, then bring it back in. (1..4)
    for ( int i = 0; i < 4; i++ )
    {
        distance = abs( parts[i].x - parts[i+1].x );
        if ( distance >= xLimits.values[2] )
        {
            int oldX = parts[i+1].x;
            int x = oldX + 2;
            if ( oldX >= parts[i].x )
                x -= 4;
            parts[i+1].x = x;
        }
        distance = abs( parts[i].y - parts[i+1].y );
        if ( distance >= yLimits.values[2] )
        {
            int oldY = parts[i+1].y;
            int y = oldY + 2;
            if ( oldY >= parts[i].y )
                y -= 4;
            parts[i+1].y = y;
        }
    }

    // Stretch, depending on distance to the next part. (1..3)
    for ( int i = 0; i < 3; i++ )
    {
        Stretch( i, xLimits, yLimits );
    }

    // If passed the X limit, then bring it back in. (3..1)
    for ( int i = 2; i >= 0; i-- )
    {
        int xLimit = parts[0].x;
        for ( int j = i; j >= 0; j-- )
        {
            xLimit += headToEndXDiv4;
        }
        int x = parts[i+1].x + 1;
        if ( xLimit < parts[i+1].x )
            x -= 2;
        parts[i+1].x = x;
    }

    // If part's Y is not in between surrounding parts, then bring it back in. (3..2)
    for ( int i = 1; i >= 0; i-- )
    {
        int  y2 = parts[i+2].y;
        if ( y2 < parts[i+1].y )
        {
            if ( y2 < parts[i+3].y )
                parts[i+2].y++;
        }
        else
        {
            if ( y2 >= parts[i+3].y )
                parts[i+2].y--;
        }
    }
}

void GleeokNeck::GetLimits( int distance, Limits& limits )
{
    if ( distance > 4 )
        distance = 4;
    limits.values[0] = distance;

    distance += 4;
    if ( distance > 8 )
        distance = 8;
    limits.values[1] = distance;

    distance += 4;
    if ( distance > 11 )
        distance = 11;
    limits.values[2] = distance;
}

void GleeokNeck::Stretch( int index, const Limits& xLimits, const Limits& yLimits )
{
    int distance;
    int funcIndex = 0;

    // The original was [index+2] - [index+2]
    distance = abs( parts[index+2].x - parts[index+1].x );
    if ( distance >= xLimits.values[0] )
        funcIndex++;
    if ( distance >= xLimits.values[1] )
        funcIndex++;

    distance = abs( parts[index+2].y - parts[index+1].y );
    if ( distance >= yLimits.values[0] )
        funcIndex += 3;
    if ( distance >= yLimits.values[1] )
        funcIndex += 3;

    static const Func   funcs[] = 
    {
        &GleeokNeck::CrossedNoLimits,
        &GleeokNeck::CrossedLowLimit,
        &GleeokNeck::CrossedMidXLimit,
        &GleeokNeck::CrossedLowLimit,
        &GleeokNeck::CrossedLowLimit,
        &GleeokNeck::CrossedMidXLimit,
        &GleeokNeck::CrossedMidYLimit,
        &GleeokNeck::CrossedMidYLimit,
        &GleeokNeck::CrossedBothMidLimits,
    };

    assert( funcIndex >= 0 && funcIndex < _countof( funcs ) );

    Func f = funcs[funcIndex];
    (this->*f)( index );
}

void GleeokNeck::CrossedNoLimits( int index )
{
    int r = Util::GetRandom( 2 );
    if ( r == 0 )
    {
        int oldX = parts[index+1].x;
        int x = oldX + 2;
        if ( oldX < parts[index+2].x )
            x -= 4;
        parts[index+1].x = x;
    }
    else
    {
        int oldY = parts[index+1].y;
        int y = oldY + 2;
        if ( oldY <= parts[index+2].y )
            y -= 4;
        parts[index+1].y = y;
    }
}

void GleeokNeck::CrossedLowLimit( int index )
{
    // Nothing to do
}

void GleeokNeck::CrossedMidYLimit( int index )
{
    int oldY = parts[index+1].y;
    int y = oldY + 2;
    if ( oldY > parts[index+2].y )
        y -= 4;
    parts[index+1].y = y;
}

void GleeokNeck::CrossedMidXLimit( int index )
{
    int oldX = parts[index+1].x;
    int x = oldX + 2;
    if ( oldX >= parts[index+2].x )
        x -= 4;
    parts[index+1].x = x;
}

void GleeokNeck::CrossedBothMidLimits( int index )
{
    int r = Util::GetRandom( 2 );
    if ( r == 0 )
        CrossedMidXLimit( index );
    else
        CrossedMidYLimit( index );
}


//----------------------------------------------------------------------------
//  Ganon
//----------------------------------------------------------------------------

static const uint8_t    ganonNormalPalette[]    = { 0x16, 0x2C, 0x3C };
static const uint8_t    ganonRedPalette[]       = { 0x07, 0x17, 0x30 };


Ganon::Ganon( int x, int y )
    :   BlueWizzrobeBase( Obj_Ganon, x, y ),
        visual( Visual_None ),
        state( 0 ),
        lastHitTimer( 0 ),
        dyingTimer( 0 ),
        frame( 0 ),
        cloudDist( 0 )
{
    invincibilityMask = 0xFA;

    animator.durationFrames = 1;
    animator.time = 0;
    animator.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B3_Ganon );

    pileImage.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B3_Pile );

    cloudAnimator.durationFrames = 1;
    cloudAnimator.time = 0;
    cloudAnimator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Cloud );

    World::GetPlayer()->SetState( Player::Paused );
    World::GetPlayer()->SetObjectTimer( 0x40 );
    objTimer = 0;

    SetBossPalette( ganonNormalPalette );
    // The original game starts roaring here. But, I think it sounds better later.
}

void Ganon::Update()
{
    visual = Visual_None;

    switch ( state )
    {
    case 0: UpdateHoldDark(); break;
    case 1: UpdateHoldLight(); break;
    case 2: UpdateActive(); break;
    }
}

void Ganon::Draw()
{
    struct SlashSpec
    {
        uint8_t Sheet;
        uint8_t AnimIndex;
        uint8_t Flags;
    };

    static const SlashSpec slashSpecs[] = 
    {
        { Sheet_Boss,           Anim_B3_Slash_U,    0 },
        { Sheet_PlayerAndItems, Anim_PI_Slash,      1 },
        { Sheet_Boss,           Anim_B3_Slash_L,    1 },
        { Sheet_PlayerAndItems, Anim_PI_Slash,      3 },
        { Sheet_Boss,           Anim_B3_Slash_U,    2 },
        { Sheet_PlayerAndItems, Anim_PI_Slash,      2 },
        { Sheet_Boss,           Anim_B3_Slash_L,    0 },
        { Sheet_PlayerAndItems, Anim_PI_Slash,      0 },
    };

    if ( (visual & Visual_Ganon) != 0 )
    {
        int pal = CalcPalette( SeaPal );
        animator.DrawFrame( Sheet_Boss, objX, objY, pal, frame );
    }

    if ( (visual & Visual_Pile) != 0 )
    {
        pileImage.Draw( Sheet_Boss, objX, objY, SeaPal );
    }

    if ( (visual & Visual_Pieces) != 0 )
    {
        int cloudFrame = (cloudDist < 6) ? 2 : 1;

        for ( int i = 0; i < 8; i++ )
        {
            int cloudX = objX;
            int cloudY = objY;

            Util::MoveSimple8( cloudX, cloudY, piecesDir[i], cloudDist );

            cloudAnimator.DrawFrame( Sheet_PlayerAndItems, cloudX, cloudY, SeaPal, cloudFrame );
        }

        int slashPal = 4 + (GetFrameCounter() & 3);

        for ( int i = 0; i < 8; i++ )
        {
            const SlashSpec& slashSpec = slashSpecs[i];
            SpriteImage image;

            image.anim = Graphics::GetAnimation( slashSpec.Sheet, slashSpec.AnimIndex );
            image.Draw( slashSpec.Sheet, sparksX[i], sparksY[i], slashPal, slashSpec.Flags );
        }
    }
}

void Ganon::UpdateHoldDark()
{
    World::Get()->LiftItem( Item_TriforcePiece, 0 );

    if ( World::GetPlayer()->GetObjectTimer() != 0 )
    {
        if ( World::GetPlayer()->GetObjectTimer() == 1 )
        {
            Sound::PlayEffect( SEffect_boss_hit );
            Sound::PlaySong( Song_ganon, Sound::MainSongStream, false );
            //       The original game does it in the else part below, but only when [$51C] = $C0
            //       Which is in the first frame that the player's object timer is 0.
        }
    }
    else
    {
        World::FadeIn();

        if ( World::GetFadeStep() == 0 )
        {
            state = 1;
            World::GetPlayer()->SetObjectTimer( 0xC0 );
        }
        visual = Visual_Ganon;
    }
}

void Ganon::UpdateHoldLight()
{
    World::Get()->LiftItem( Item_TriforcePiece, 0 );

    if ( World::GetPlayer()->GetObjectTimer() == 0 )
    {
        World::GetPlayer()->SetState( Player::Idle );
        World::Get()->LiftItem( Item_None );
        Sound::PlaySong( Song_level9, Sound::MainSongStream, true );
        Sound::PlayEffect( SEffect_boss_roar1, true, Sound::AmbientInstance );
        state = 2;
    }

    visual = Visual_Ganon;
}

void Ganon::UpdateActive()
{
    if ( dyingTimer != 0 )
    {
        UpdateDying();
    }
    else
    {
        CheckCollision();
        PlayBossHitSoundIfHit();

        if ( lastHitTimer != 0 )
            UpdateLastHit();
        else if ( objTimer == 0 )
            UpdateMoveAndShoot();
        else if ( objTimer == 1 )
            ResetPosition();
        else
            visual = Visual_Ganon;
    }
}

void Ganon::UpdateDying()
{
    // This isn't exactly like the original, but the intent is clearer.
    if ( dyingTimer < 0xFF )
        dyingTimer++;

    if ( dyingTimer < 0x50 )
    {
        visual |= Visual_Ganon;
        return;
    }

    if ( dyingTimer == 0x50 )
    {
        SetPilePalette();
        Graphics::UpdatePalettes();
        objX += 8;
        objY += 8;
        MakePieces();
        Sound::PlayEffect( SEffect_boss_hit );
        Sound::StopEffect( Sound::AmbientInstance );
        Sound::PlaySong( Song_ganon, Sound::MainSongStream, false );
    }

    visual |= Visual_Pile;

    if ( dyingTimer < 0xA0 )
    {
        MovePieces();
        visual |= Visual_Pieces;
    }
    else if ( dyingTimer == 0xA0 )
    {
        World::AddUWRoomItem();
        Object* triforce = World::GetObject( ItemSlot );
        triforce->SetX( objX );
        triforce->SetY( objY );
        World::Get()->IncrementRoomKillCount();
        Sound::PlayEffect( SEffect_room_item );
    }
}

void Ganon::CheckCollision()
{
    Player* player = World::GetPlayer();

    if ( player->GetInvincibilityTimer() == 0 )
    {
        CheckPlayerCollisionDirect();
    }

    if ( lastHitTimer != 0 )
    {
        int itemValue = World::GetItem( ItemSlot_Arrow );
        if ( itemValue == 2 )
        {
            // The original checks the state of the arrow here and leaves if <> $10.
            // But, CheckArrow does a similar check (>= $20). As far as I can tell, both are equivalent.
            if ( CheckArrow( ArrowSlot ) )
            {
                dyingTimer = 1;
                invincibilityTimer = 0x28;
                cloudDist = 8;
            }
        }
        return;
    }
    else if ( objTimer != 0 )
        return;

    CheckSword( PlayerSwordSlot );

    if ( decoration != 0 )
    {
        hp = 0xF0;
        lastHitTimer--;
        SetBossPalette( ganonRedPalette );
    }

    if ( invincibilityTimer != 0 )
    {
        PlayBossHitSoundIfHit();
        objTimer = 0x40;
    }

    decoration = 0;
    shoveDir = 0;
    shoveDistance = 0;
    invincibilityTimer = 0;
}

void Ganon::UpdateLastHit()
{
    if ( (GetFrameCounter() & 1) == 1 )
    {
        lastHitTimer--;
        if ( lastHitTimer == 0 )
        {
            ResetPosition();
            SetBossPalette( ganonNormalPalette );
            return;
        }
    }

    if ( lastHitTimer >= 0x30
        || (GetFrameCounter() & 1) == 1 )
    {
        visual |= Visual_Ganon;
    }
}

void Ganon::UpdateMoveAndShoot()
{
    frame++;
    if ( frame == 6 )
        frame = 0;

    MoveAround();

    if ( (GetFrameCounter() & 0x3F) == 0 )
        ShootFireball( Obj_Fireball2, objX, objY );
}

void Ganon::MoveAround()
{
    flashTimer = 1;
    turnTimer++;
    TurnIfNeeded();
    MoveAndCollide();
}

void Ganon::MakePieces()
{
    for ( int i = 0; i < 8; i++ )
    {
        sparksX[i] = objX + 4;
        sparksY[i] = objY + 4;
        piecesDir[i] = Util::GetDirection8( i );
    }
}

void Ganon::MovePieces()
{
    if ( cloudDist != 0 && (GetFrameCounter() & 7) == 0 )
        cloudDist--;

    for ( int i = 0; i < 8; i++ )
    {
        if ( Util::IsHorizontal( piecesDir[i] )
            || Util::IsVertical( piecesDir[i] )
            || (GetFrameCounter() & 3) != 0 )
        {
            Util::MoveSimple8( sparksX[i], sparksY[i], piecesDir[i], 1 );
        }
    }
}

void Ganon::SetBossPalette( const uint8_t* palette )
{
    Graphics::SetColorIndexed( SeaPal, 1, palette[0] );
    Graphics::SetColorIndexed( SeaPal, 2, palette[1] );
    Graphics::SetColorIndexed( SeaPal, 3, palette[2] );
    Graphics::UpdatePalettes();
}

void Ganon::ResetPosition()
{
    objY = 0xA0;
    if ( (GetFrameCounter() & 1) == 0 )
        objX = 0x30;
    else
        objX = 0xB0;
}


//----------------------------------------------------------------------------
//  Zelda
//----------------------------------------------------------------------------

enum
{
    ZeldaX          = 0x78,
    ZeldaLineX1     = 0x70,
    ZeldaLineX2     = 0x80,
    ZeldaY          = 0x88,
    ZeldaLineY      = 0x95,

    LinkX           = 0x88,
    LinkY           = ZeldaY,
};


Object* Zelda::Make()
{
    static const uint8_t xs[] = { 0x60, 0x70, 0x80, 0x90 };
    static const uint8_t ys[] = { 0xB5, 0x9D, 0x9D, 0xB5 };

    for ( int i = 0; i < _countof( xs ); i++ )
    {
        int y = ys[i];

        GuardFire* fire = new GuardFire( xs[i], y );
        World::SetObject( MonsterSlot1 + 1 + i, fire );
    }

    Zelda* zelda = new Zelda();
    return zelda;
}

Zelda::Zelda()
    :   Object( Obj_Zelda ),
        state( 0 )
{
    objX = ZeldaX;
    objY = ZeldaY;

    image.anim = Graphics::GetAnimation( Sheet_Boss, Anim_B3_Zelda_Stand );
}

void Zelda::Update()
{
    Player* player = World::GetPlayer();

    if ( state == 0 )
    {
        int     playerX = player->GetX();
        int     playerY = player->GetY();

        if (   playerX >= ZeldaLineX1 
            && playerX <= ZeldaLineX2 
            && playerY <= ZeldaLineY )
        {
            state = 1;
            player->SetState( Player::Paused );
            player->SetX( LinkX );
            player->SetY( LinkY );
            player->SetFacing( Dir_Left );
            Sound::PlaySong( Song_zelda, Sound::MainSongStream, false );
            objTimer = 0x80;
        }
    }
    else
    {
        // ORIGINAL: Calls $F229. But, I don't see why we need to.
        if ( objTimer == 0 )
        {
            player->SetState( Player::Idle );
            World::WinGame();
        }
    }
}

void Zelda::Draw()
{
    image.Draw( Sheet_Boss, objX, objY, PlayerPalette );
}


//----------------------------------------------------------------------------
//  StandingFire
//----------------------------------------------------------------------------

StandingFire::StandingFire( int x, int y )
    :   Object( Obj_StandingFire )
{
    objX = x;
    objY = y;

    animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Fire );
    animator.durationFrames = 12;
    animator.time = 0;
}

void StandingFire::Update()
{
    CheckPlayerCollision();
    animator.Advance();
}

void StandingFire::Draw()
{
    animator.Draw( Sheet_PlayerAndItems, objX, objY, RedFgPalette );
}


//----------------------------------------------------------------------------
//  GuardFire
//----------------------------------------------------------------------------

GuardFire::GuardFire( int x, int y )
    :   Object( Obj_GuardFire )
{
    objX = x;
    objY = y;

    animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Fire );
    animator.durationFrames = 12;
    animator.time = 0;
}

void GuardFire::Update()
{
    animator.Advance();
    CheckCollisions();
    if ( decoration != 0 )
    {
        DeadDummy* dummy = new DeadDummy( objX, objY );
        World::SetObject( World::GetCurrentObjectSlot(), dummy );
        dummy->SetDecoration( decoration );
    }
}

void GuardFire::Draw()
{
    animator.Draw( Sheet_PlayerAndItems, objX, objY, RedFgPalette );
}


//----------------------------------------------------------------------------
//  RupeeStash
//----------------------------------------------------------------------------

Object* RupeeStash::Make()
{
    static const uint8_t xs[] = { 0x78, 0x70, 0x80, 0x60, 0x70, 0x80, 0x90, 0x70, 0x80, 0x78 };
    static const uint8_t ys[] = { 0x70, 0x80, 0x80, 0x90, 0x90, 0x90, 0x90, 0xA0, 0xA0, 0xB0 };

    for ( int i = 0; i < _countof( xs ); i++ )
    {
        Object* rupee = new RupeeStash( xs[i], ys[i] );
        World::SetObject( i, rupee );
    }

    return World::GetObject( MonsterSlot1 );
}

RupeeStash::RupeeStash( int x, int y )
    :   Object( Obj_RupieStash )
{
    objX = x;
    objY = y;
}

void RupeeStash::Update()
{
    Player* player = World::GetPlayer();
    int distanceX = abs( player->GetX() - objX );
    int distanceY = abs( player->GetY() - objY );

    if (   distanceX <= 8
        && distanceY <= 8 )
    {
        World::PostRupeeWin( 1 );
        World::Get()->IncrementRoomKillCount();
        isDeleted = true;
    }
}

void RupeeStash::Draw()
{
    DrawItemWide( Item_Rupee, objX, objY );
}


//----------------------------------------------------------------------------
//  Fairy
//----------------------------------------------------------------------------

static const uint8_t fairyAnimMap[DirCount] = 
{
    Anim_PI_Fairy,
    Anim_PI_Fairy,
    Anim_PI_Fairy,
    Anim_PI_Fairy
};

FlyerSpec fairySpec = 
{
    fairyAnimMap,
    Sheet_PlayerAndItems,
    RedPal,
    0xA0
};


Fairy::Fairy( int x, int y )
    :   Flyer( Obj_Item, &fairySpec, x, y ),
        timer( 0xFF )
{
    decoration = 0;
    facing = Dir_Up;
    curSpeed = 0x7F;
}

void Fairy::Update()
{
    if ( (GetFrameCounter() & 1) == 1 )
        timer--;

    if ( timer == 0 )
    {
        isDeleted = true;
        return;
    }

    UpdateStateAndMove();

    static const int objSlots[] = { PlayerSlot, BoomerangSlot };
    bool touchedItem = false;

    for ( int i = 0; i < _countof( objSlots ); i++ )
    {
        int slot = objSlots[i];
        Object* obj = World::GetObject( slot );
        if ( obj != nullptr && !obj->IsDeleted() && TouchesObject( obj ) )
        {
            touchedItem = true;
            break;
        }
    }

    if ( touchedItem )
    {
        World::AddItem( Item_Fairy );
        isDeleted = true;
    }
}

void Fairy::UpdateFullSpeedImpl()
{
    GoToState( 3, 6 );
}

int Fairy::GetFrame()
{
    return (moveCounter & 4) >> 2;
}

bool Fairy::TouchesObject( Object* obj )
{
    int distanceX = abs( obj->GetX() - objX );
    int distanceY = abs( obj->GetY() - objY );

    return distanceX <= 8
        && distanceY <= 8;
}


//----------------------------------------------------------------------------
//  PondFairy
//----------------------------------------------------------------------------

enum
{
    PondFairyX              = 0x78,
    PondFairyY              = 0x7D,
    PondFairyLineX1         = 0x70,
    PondFairyLineX2         = 0x80,
    PondFairyLineY          = 0xAD,
    PondFairyRingCenterX    = 0x80,
    PondFairyRingCenterY    = 0x98,
};


PondFairy::PondFairy()
    :   Object( Obj_PondFairy ),
        state( Idle )
{
    objX = PondFairyX;
    objY = PondFairyY;

    animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Fairy );
    animator.time = 0;
    animator.durationFrames = 8;

    memset( heartState, 0, sizeof heartState );
    memset( heartAngle, 0, sizeof heartAngle );

    Sound::PlayEffect( SEffect_item );
}

void PondFairy::Update()
{
    animator.Advance();

    if ( state == Idle )
        UpdateIdle();
    else if ( state == Healing )
        UpdateHealing();
}

void PondFairy::UpdateIdle()
{
    Player* player = World::GetPlayer();
    int playerX = player->GetX();
    int playerY = player->GetY();

    if (   playerY != PondFairyLineY
        || playerX < PondFairyLineX1
        || playerX > PondFairyLineX2 )
        return;

    state = Healing;
    player->SetState( Player::Paused );
}

void PondFairy::UpdateHealing()
{
    static const uint8_t entryAngles[] = { 0, 11, 22, 33, 44, 55, 66, 77 };

    for ( int i = 0; i < _countof( heartState ); i++ )
    {
        if ( heartState[i] == 0 )
        {
            if ( heartAngle[0] == entryAngles[i] )
                heartState[i] = 1;
        }
        else
        {
            heartAngle[i]++;
            if ( heartAngle[i] >= 85 )
                heartAngle[i] = 0;
        }
    }

    Profile& profile = World::GetProfile();
    int maxHeartsValue = profile.GetMaxHeartsValue();

    Sound::PlayEffect( SEffect_character );

    if ( profile.Hearts < maxHeartsValue )
    {
        World::FillHearts( 6 );
    }
    else if ( heartState[7] != 0 )
    {
        state = Healed;
        Player* player = World::GetPlayer();
        player->SetState( Player::Idle );
        World::SetSwordBlocked( false );
    }
}

void PondFairy::Draw()
{
    int xOffset = (16 - animator.anim->width) / 2;
    animator.Draw( Sheet_PlayerAndItems, PondFairyX + xOffset, PondFairyY, RedFgPalette );

    if ( state != Healing )
        return;

    const float Radius = 0x36;
    const float Angle = -Util::_2_PI / 85.0;
    SpriteImage heart;

    heart.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Heart );

    for ( int i = 0; i < _countof( heartState ); i++ )
    {
        if ( heartState[i] == 0 )
            continue;

        int angleIndex = heartAngle[i] + 22;
        float angle = Angle * angleIndex;
        int x = cos( angle ) * Radius + PondFairyRingCenterX;
        int y = sin( angle ) * Radius + PondFairyRingCenterY;

        heart.Draw( Sheet_PlayerAndItems, x, y, RedFgPalette );
    }
}


//----------------------------------------------------------------------------
//  DeadDummy
//----------------------------------------------------------------------------

DeadDummy::DeadDummy( int x, int y )
    :   Object( Obj_DeadDummy )
{
    decoration = 0;
    objX = x;
    objY = y;
}

void DeadDummy::Update()
{
    decoration = 0x10;
    Sound::PlayEffect( SEffect_monster_die );
}

void DeadDummy::Draw()
{
}


//----------------------------------------------------------------------------
//  Statues
//----------------------------------------------------------------------------

uint8_t Statues::timers[];


void Statues::Init()
{
    memset( timers, 0, sizeof timers );
}

void Statues::Update( int pattern )
{
    static const uint8_t statueCounts[Patterns]     = { 4, 2, 2 };
    static const uint8_t startTimers[MaxStatues]    = { 0x50, 0x80, 0xF0, 0x60 };
    static const uint8_t patternOffsets[Patterns]   = { 0, 4, 6 };
    static const uint8_t xs[] = { 0x24, 0xC8, 0x24, 0xC8, 0x64, 0x88, 0x48, 0xA8 };
    static const uint8_t ys[] = { 0xC0, 0xBC, 0x64, 0x5C, 0x94, 0x8C, 0x82, 0x86 };

    if ( pattern < 0 || pattern >= Patterns )
        return;

    int slot = World::FindEmptyMonsterSlot();
    if ( slot < MonsterSlot1 + 5 )
        return;

    Player* player = World::GetPlayer();
    int statueCount = statueCounts[pattern];

    for ( int i = 0; i < statueCount; i++ )
    {
        int timer = timers[i];
        timers[i]--;

        if ( timer != 0 )
            continue;

        int r = Util::GetRandom( 256 );
        if ( r >= 0xF0 )
            continue;

        int j = r & 3;
        timers[i] = startTimers[j];

        int offset = i + patternOffsets[pattern];
        int x = xs[offset];
        int y = ys[offset];

        if (   abs( x - player->GetX() ) >= 0x18
            || abs( y - player->GetY() ) >= 0x18 )
        {
            ShootFireball( Obj_Fireball, x, y );
        }
    }
}


//----------------------------------------------------------------------------
//  MakeMonster
//----------------------------------------------------------------------------

Object* MakeMonster( ObjType type, int x, int y )
{
    Object* obj = nullptr;

    switch ( type )
    {
    case Obj_BlueLynel: obj = new StdChaseWalker( type, &blueLynelSpec, x, y ); break;
    case Obj_RedLynel: obj = new StdChaseWalker( type, &redLynelSpec, x, y ); break;
    case Obj_BlueMoblin: obj = new StdWanderer( type, &blueMoblinSpec, 0xA0, x, y ); break;
    case Obj_RedMoblin: obj = new StdWanderer( type, &redMoblinSpec, 0xA0, x, y ); break;
    case Obj_BlueGoriya: obj = new Goriya( Obj_BlueGoriya, &blueGoriyaSpec, x, y ); break;
    case Obj_RedGoriya: obj = new Goriya( Obj_RedGoriya, &redGoriyaSpec, x, y ); break;
    case Obj_RedSlowOctorock: obj = new DelayedWanderer( type, &redSlowOctorockSpec, 0x70, x, y ); break;
    case Obj_RedFastOctorock: obj = new DelayedWanderer( type, &redFastOctorockSpec, 0x70, x, y ); break;
    case Obj_BlueSlowOctorock:obj = new DelayedWanderer( type, &blueSlowOctorockSpec, 0xA0, x, y );break;
    case Obj_BlueFastOctorock:obj = new DelayedWanderer( type, &blueFastOctorockSpec, 0xA0, x, y );break;
    case Obj_RedDarknut: obj = Darknut::MakeRedDarknut( x, y ); break;
    case Obj_BlueDarknut: obj = Darknut::MakeBlueDarknut( x, y ); break;
    case Obj_BlueTektite: obj = new Jumper( Obj_BlueTektite, &blueTektiteSpec, x, y ); break;
    case Obj_RedTektite: obj = new Jumper( Obj_RedTektite, &redTektiteSpec, x, y ); break;
    case Obj_BlueLeever: obj = new BlueLeever( x, y ); break;
    case Obj_RedLeever: obj = new RedLeever( x, y ); break;
    case Obj_Zora: obj = new Zora(); break;
    case Obj_Vire: obj = new Vire( x, y ); break;
    case Obj_Zol: obj = new Zol( x, y ); break;
    case Obj_Gel: obj = new Gel( Obj_Gel, x, y, Dir_None, 0 ); break;
    case Obj_PolsVoice: obj = new PolsVoice( x, y ); break;
    case Obj_LikeLike: obj = new LikeLike( x, y ); break;
    case Obj_Peahat: obj = new Peahat( x, y ); break;
    case Obj_BlueKeese: obj = Keese::MakeBlueKeese( x, y ); break;
    case Obj_BlackKeese: obj = Keese::MakeBlackKeese( x, y ); break;
    case Obj_Armos: obj = new Armos( x, y ); break;
    case Obj_Boulders: obj = new Boulders(); break;
    case Obj_Boulder: obj = new Jumper( Obj_Boulder, &boulderSpec, x, y ); break;
    case Obj_Ghini: obj = new Ghini( x, y ); break;
    case Obj_FlyingGhini: obj = new FlyingGhini( x, y ); break;
    case Obj_BlueWizzrobe: obj = new BlueWizzrobe( x, y ); break;
    case Obj_RedWizzrobe: obj = new RedWizzrobe(); break;
    case Obj_Wallmaster: obj = new Wallmaster(); break;
    case Obj_Rope: obj = new Rope( x, y ); break;
    case Obj_Stalfos: obj = new Stalfos( x, y ); break;
    case Obj_Bubble1: 
    case Obj_Bubble2: 
    case Obj_Bubble3: 
        obj = new Bubble( type, x, y );
        break;
    case Obj_PondFairy: obj = new PondFairy(); break;
    case Obj_Gibdo: obj = new Gibdo( x, y ); break;
    case Obj_3Dodongos:
    case Obj_1Dodongo: obj = new Dodongo( x, y ); break;
    case Obj_BlueGohma:
    case Obj_RedGohma: obj = new Gohma( type ); break;
    case Obj_RupieStash: obj = RupeeStash::Make(); break;
    case Obj_Zelda: obj = Zelda::Make(); break;
    case Obj_Digdogger1: obj = Digdogger::Make( x, y, 3 ); break;
    case Obj_Digdogger2: obj = Digdogger::Make( x, y, 1 ); break;
    case Obj_RedLamnola: obj = Lamnola::MakeSet( Obj_RedLamnola ); break;
    case Obj_BlueLamnola: obj = Lamnola::MakeSet( Obj_BlueLamnola ); break;
    case Obj_Manhandla: obj = Manhandla::Make( x, y ); break;
    case Obj_Aquamentus: obj = new Aquamentus(); break;
    case Obj_Ganon: obj = new Ganon( x, y ); break;
    case Obj_Moldorm: obj = Moldorm::MakeSet(); break;
    case Obj_Gleeok1:
    case Obj_Gleeok2:
    case Obj_Gleeok3:
    case Obj_Gleeok4:
        obj = new Gleeok( type, x, y );
        break;
    case Obj_Patra1:
    case Obj_Patra2: obj = Patra::MakePatra( type ); break;
    case Obj_Trap: obj = Trap::MakeSet( 6 ); break;
    case Obj_TrapSet4: obj = Trap::MakeSet( 4 ); break;
    default: assert( false ); break;
    }

    return obj;
}

void ClearRoomMonsterData()
{
    RedLeever::ClearRoomData();
    Boulders::ClearRoomData();
    Manhandla::ClearRoomData();
    Statues::Init();
}
