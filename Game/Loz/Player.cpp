/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Player.h"
#include "PlayerItemsAnim.h"
#include "Graphics.h"
#include "Input.h"
#include "World.h"
#include "ItemObj.h"
#include "ObjType.h"
#include "Profile.h"
#include "Sound.h"
#include "SoundId.h"
#include "TileAttr.h"


const int   ShieldStateCount = 3;
const int   DirCount = 4;
const int   WalkDurationFrames = 12;
const int   GridAlign = 8;
const int   GridAlignMask = ~(GridAlign - 1);
const int   HalfGridAlign = GridAlign / 2;


static uint8_t animMap[ShieldStateCount][DirCount] = 
{
    {
        Anim_PI_LinkWalk_NoShield_Right,
        Anim_PI_LinkWalk_NoShield_Left,
        Anim_PI_LinkWalk_NoShield_Down,
        Anim_PI_LinkWalk_NoShield_Up,
    },

    {
        Anim_PI_LinkWalk_LittleShield_Right,
        Anim_PI_LinkWalk_LittleShield_Left,
        Anim_PI_LinkWalk_LittleShield_Down,
        Anim_PI_LinkWalk_LittleShield_Up,
    },

    {
        Anim_PI_LinkWalk_BigShield_Right,
        Anim_PI_LinkWalk_BigShield_Left,
        Anim_PI_LinkWalk_BigShield_Down,
        Anim_PI_LinkWalk_BigShield_Up,
    }
};

static uint8_t thrustAnimMap[DirCount] = 
{
    Anim_PI_LinkThrust_Right,
    Anim_PI_LinkThrust_Left,
    Anim_PI_LinkThrust_Down,
    Anim_PI_LinkThrust_Up
};

static const Limits playerLimits = { 0xF0, 0x00, 0xDD, 0x3D };

Limits Player::GetPlayerLimits()
{
    return playerLimits;
}


Player::Player()
    :   Object( Obj_Player ),
        state( 0 ),
        speed( WalkSpeed ),
        tileRef( 0 ),
        paralyzed( false ),
        animTimer( 0 ),
        avoidTurningWhenDiag( 0 ),
        keepGoingStraight( 0 ),
        curButtons( 0 )
{
    facing = Dir_Up;
    decoration = 0;

    animator.anim = nullptr;
    animator.time = 0;
    animator.durationFrames = WalkDurationFrames;
}

void Player::DecInvincibleTimer()
{
    if ( invincibilityTimer > 0 && (GetFrameCounter() & 1) == 0 )
        invincibilityTimer--;
}

bool Player::IsParalyzed()
{
    return paralyzed;
}

void Player::SetParalyzed( bool value )
{
    paralyzed = value;
}

SpriteAnimator* Player::GetAnimator()
{
    return &animator;
}

void Player::Update()
{
    // Do this in order to flash while you have the clock. It doesn't matter if it becomes zero, 
    // because foes will check invincibilityTimer AND the clock item.
    // I suspect that the original game did this in the drawing code.
    Profile& profile = World::GetProfile();
    if ( profile.Items[ItemSlot_Clock] != 0 )
        invincibilityTimer += 0x10;

    curButtons = Input::GetButtons();

    // It looks like others set player's state to $40. They don't bitwise-and it with $40.
    if ( (state & 0xC0) == 0x40 )
        return;

    HandleInput();

    if ( paralyzed )
        moving &= 0xF0;

    if ( (state & 0xF0) != 0x10 && (state & 0xF0) != 0x20 )
        ObjMove( speed );

    if ( World::GetMode() == Mode_LeaveCellar )
        return;

    if ( World::GetWhirlwindTeleporting() == 0 )
    {
        CheckWater();
        if ( World::GetMode() == Mode_Play )
            CheckWarp();
        Animate();
    }

    // $6EFB
    // The original game hides part of the player if it's under an underworld doorway.
    // But, we do it differently.

    if ( tileOffset == 0 )
    {
        objX = (objX & 0xF8);
        objY = (objY & 0xF8) | 5;
    }
}

void Player::Animate()
{
    // The original game also didn't animate if gameMode was 4 or $10
    if ( state != 0 )
    {
        if ( animTimer != 0 )
            animTimer--;

        if ( animTimer == 0 )
        {
            if (   (state & 0x30) == 0x10
                || (state & 0x30) == 0x20 )
            {
                animator.time = 0;
                animTimer = state & 0xF;
                state |= 0x30;
            }
            else if ( (state & 0x30) == 0x30 )
            {
                animator.AdvanceFrame();
                state &= 0xC0;
            }
        }
    }
    else if ( moving != 0 )
    {
        animator.Advance();
    }
}

// F23C
void Player::CheckWater()
{
    TileCollision collision;

    GameMode mode = World::GetMode();

    if ( mode == Mode_Leave || mode < Mode_Play )
        return;

    if ( tileOffset != 0 )
    {
        if ( (tileOffset & 7) != 0 )
            return;
        tileOffset = 0;
        if ( mode != Mode_Play )
            return;
        World::SetFromUnderground( 0 );
    }

    if ( mode != Mode_Play )
        return;

    if ( World::IsOverworld() )
    {
        if ( !World::DoesRoomSupportLadder() )
            return;
    }

    if (   World::GetDoorwayDir() != Dir_None
        || World::GetItem( ItemSlot_Ladder ) == 0
        || (state & 0xC0) == 0x40 
        || World::GetLadder() != nullptr )
        return;

    collision = World::CollidesWithTileMoving( objX, objY, facing, true );

    // The original game checked for specific water tiles in the OW and UW.
    int action = World::GetTileAction( collision.TileRef );
    if ( action != TileAction_Ladder )
        return;

    if ( moving == 0 )
        return;

    if ( moving != facing )
        return;

    static const int8_t ladderOffsetsX[] = { 0x10, -0x10, 0x00,  0x00 };
    static const int8_t ladderOffsetsY[] = { 0x03,  0x03, 0x13, -0x05 };

    int dirOrd = Util::GetDirectionOrd( (Direction) moving );

    Ladder* ladder = new Ladder();
    ladder->SetX( objX + ladderOffsetsX[dirOrd] );
    ladder->SetY( objY + ladderOffsetsY[dirOrd] );
    World::SetLadder( ladder );
}

void Player::CheckWarp()
{
    if ( World::GetFromUnderground() != 0 || tileOffset != 0 )
        return;

    if (   World::IsOverworld() 
        && World::GetRoomId() == 0x22 )
    {
        if ( (objX & 7) != 0 )
            return;
    }
    else
    {
        if ( (objX & 0xF) != 0 )
            return;
    }

    if ( (objY & 0xF) != 0xD )
        return;

    int fineRow = (objY + 3 - 0x40) / 8;
    int fineCol = objX / 8;

    World::CoverTile( fineRow, fineCol );
}

static bool IsInOuterBorder( int coord, Direction dir, const uint8_t border[] )
{
    if ( Util::IsHorizontal( dir ) )
    {
        return coord < border[0] || coord >= border[1];
    }
    else
    {
        return coord < border[2] || coord >= border[3];
    }
}

static bool IsInInnerBorder( int coord, Direction dir, const uint8_t border[] )
{
    int dirOrd = Util::GetDirectionOrd( dir );

    if ( Util::IsGrowingDir( dir ) )
    {
        dirOrd ^= 1;
        return coord >= border[dirOrd];
    }
    else
    {
        dirOrd ^= 1;
        return coord < border[dirOrd];
    }
}

// $8D8C
void Player::FilterBorderInput()
{
    // These are reverse from original, because Util::GetDirectionOrd goes in the opposite dir of $7013.
    static const uint8_t outerBorderOW[] = { 0x07, 0xE9, 0x45, 0xD6 };
    static const uint8_t outerBorderUW[] = { 0x17, 0xD9, 0x55, 0xC6 };
    static const uint8_t innerBorder[] =   { 0x1F, 0xD1, 0x54, 0xBE };

    int coord = Util::IsHorizontal( facing ) ? objX : objY;
    const uint8_t* outerBorder = World::IsOverworld() ? outerBorderOW : outerBorderUW;

    if ( IsInOuterBorder( coord, facing, outerBorder ) )
    {
        curButtons.Buttons = 0;
        if ( !World::IsOverworld() )
        {
            if ( Util::IsVertical( facing ) )
                moving &= VerticalMask;
            else
                moving &= HorizontalMask;
        }
    }
    else if ( IsInInnerBorder( coord, facing, innerBorder ) )
    {
        curButtons.Mask( InputButtons::A );
    }
}

void Player::HandleInput()
{
    moving = curButtons.Buttons & 0xF;

    if ( state == 0 )
    {
        FilterBorderInput();

        if ( curButtons.Has( InputButtons::A ) )
            UseWeapon();

        if ( curButtons.Has( InputButtons::B ) )
            UseItem();
    }

    if ( shoveDir != 0 )
        return;

    if ( !World::IsOverworld() )
        SetMovingInDoorway();

    if ( tileOffset != 0 )
        Align();
    else
        CalcAlignedMoving();
}

void Player::SetMovingInDoorway()
{
    if ( World::GetDoorwayDir() != Dir_None && moving != 0 )
    {
        uint dir = moving & facing;
        if ( dir == 0 )
        {
            dir = moving & Util::GetOppositeDir( facing );
            if ( dir == 0 )
                dir = facing;
        }
        moving = dir;
    }
}

// $B38D
void Player::Align()
{
    if ( moving == 0 )
        return;

    Direction singleMoving = GetSingleMoving();

    if ( singleMoving == facing )
    {
        SetSpeed();
        return;
    }

    uint dir = singleMoving | facing;
    if ( dir != OppositeHorizontals && dir != OppositeVerticals )
    {
        if ( keepGoingStraight != 0 )
        {
            SetSpeed();
            return;
        }

        if ( abs( tileOffset ) >= 4 )
            return;

        if ( Util::IsGrowingDir( facing ) )
        {
            if ( tileOffset < 0 )
                return;
        }
        else
        {
            if ( tileOffset >= 0 )
                return;
        }

        facing = Util::GetOppositeDir( facing );

        if ( tileOffset >= 0 )
            tileOffset -= 8;
        else
            tileOffset += 8;
    }
    else
    {
        facing = singleMoving;
        moving = singleMoving;
    }
}

// $B2CF
void Player::CalcAlignedMoving()
{
    Direction   lastDir       = Dir_None;
    Direction   lastClearDir  = Dir_None;
    int         dirCount      = 0;
    int         clearDirCount = 0;

    keepGoingStraight = 0;

    for ( int i = 0; i < 4; i++ )
    {
        Direction dir = Util::GetOrdDirection( i );
        if ( (moving & dir) != 0 )
        {
            lastDir = dir;
            dirCount++;

            TileCollision collision = World::CollidesWithTileMoving( objX, objY, dir, true );
            tileRef = collision.TileRef;
            if ( !collision.Collides )
            {
                lastClearDir = dir;
                clearDirCount++;
            }
        }
    }

    if ( dirCount == 0 )
        return;

    if ( dirCount == 1 )
    {
        avoidTurningWhenDiag = 0;
        facing = lastDir;
        moving = lastDir;
        SetSpeed();
        return;
    }

    if ( clearDirCount == 0 )
    {
        moving = 0;
        return;
    }

    keepGoingStraight++;

    if ( clearDirCount == 1 || World::IsOverworld() )
    {
        avoidTurningWhenDiag = 0;
        facing = lastClearDir;
        moving = lastClearDir;
        SetSpeed();
        return;
    }

    if ( objX == 0x20 || objX == 0xD0 )
    {
        if ( objY != 0x85 || (facing & Dir_Down) == 0 )
            goto TakeFacingPerpDir;
    }

    if ( avoidTurningWhenDiag == 0 )
        goto TakeFacingPerpDir;

    if ( World::IsOverworld() 
        || objX != 0x78
        || objY != 0x5D )
    {
        moving = facing;
        SetSpeed();
        return;
    }

// B34D
TakeFacingPerpDir:
    // moving dir is diagonal. Take the dir component that's perpendicular to facing.
    avoidTurningWhenDiag++;

    static const uint8_t axisMasks[] = { 3, 3, 0xC, 0xC };

    int dirOrd = Util::GetDirectionOrd( facing );
    uint movingInFacingAxis = moving & axisMasks[dirOrd];
    uint perpMoving = moving ^ movingInFacingAxis;
    facing = (Direction) perpMoving;
    moving = (Direction) perpMoving;
    SetSpeed();
}

// $B366
void Player::SetSpeed()
{
    uint8_t newSpeed = WalkSpeed;

    if ( World::IsOverworld() )
    {
        if ( tileRef == Tile_HiStairs )
        {
            newSpeed = StairsSpeed;
            if ( speed != newSpeed )
                fraction = 0;
        }
    }

    speed = newSpeed;
}

void Player::SetFacingAnim()
{
    int shieldState = World::GetItem( ItemSlot_MagicShield ) + 1;
    int dirOrd = Util::GetDirectionOrd( facing );
    if ( (state & 0x30) == 0x10 || (state & 0x30) == 0x20 )
        animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, thrustAnimMap[dirOrd] );
    else
        animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, animMap[shieldState][dirOrd] );
}

void Player::Draw()
{
    uint32_t palette = CalcPalette( PlayerPalette );
    int y = objY;

    if ( World::IsOverworld() || World::GetMode() == Mode_PlayCellar )
        y += 2;

    SetFacingAnim();
    animator.Draw( Sheet_PlayerAndItems, objX, y, palette );
}


void* Player::GetInterface( ObjInterfaces iface )
{
    if ( iface == ObjItf_IThrower )
        return (IThrower*) this;
    return nullptr;
}

int  Player::GetState()
{
    if ( (state & 0xC0) == 0x40 )
        return Paused;
    if ( (state & 0xF0) != 0 )
        return Wielding;
    return Idle;
}

void Player::SetState( State state )
{
    if ( state == Paused )
        this->state = 0x40;
    else if ( state == Idle )
        this->state = 0;
}

void Player::SetFacing( Direction dir )
{
    facing = dir;
}

Bounds Player::GetBounds()
{
    Bounds bounds;
    bounds.X = objX;
    bounds.Y = objY + 8;
    bounds.Width = 16;
    bounds.Height = 8;
    return bounds;
}

Point Player::GetMiddle()
{
    Point p = { objX + 8, objY + 8 };
    return p;
}

int  Player::GetInvincibilityTimer()
{
    return invincibilityTimer;
}

void Player::SetInvincibilityTimer( int value )
{
    invincibilityTimer = value;
}

void Player::ResetShove()
{
    shoveDir = Dir_None;
    shoveDistance = 0;
}

void Player::Catch()
{
    // Original game:
	//   player.state := player.state | $20
	//   player.animTimer := 1

    if ( state == 0 )
    {
        animator.time = 0;
        animTimer = 6;
        state = 0x26;
    }
    else
    {
        animator.time = 0;
        animTimer = 1;
        state = 0x30;
    }
}

void Player::BeHarmed( Object* collider, Point& otherMiddle )
{
    // The original sets [$C] here. [6] was already set to the result of DoObjectsCollide.
    // [$C] takes on the same values as [6], so I don't know why it was needed.

    uint16_t damage = World::GetPlayerDamage( collider->GetType() );
    BeHarmed( collider, otherMiddle, damage );
}

void Player::BeHarmed( Object* collider, Point& otherMiddle, uint16_t damage )
{
    if ( collider->GetType() != Obj_Whirlwind )
        Sound::PlayEffect( SEffect_player_hit );

    Profile& profile = World::GetProfile();
    int ringValue = World::GetItem( ItemSlot_Ring );

    damage >>= ringValue;

    World::ResetKilledObjectCount();

    if ( profile.Hearts <= damage )
    {
        profile.Hearts = 0;
        state = 0;
        facing = Dir_Down;
        World::Die();
    }
    else
    {
        profile.Hearts -= damage;
    }
}

void Player::Stop()
{
    state = 0;
    shoveDir = Dir_None;
    shoveDistance = 0;
    invincibilityTimer = 0;
}

void Player::MoveLinear( Direction dir, int speed )
{
    if ( (tileOffset & 7) == 0 )
        tileOffset = 0;
    ObjMoveDir( speed, dir );
}


//====================================================================================
//  UseItem
//====================================================================================

int UseCandle( int x, int y, Direction facingDir )
{
    int itemValue = World::GetItem( ItemSlot_Candle );
    if ( itemValue == 1 && World::GetCandleUsed() )
        return 0;

    World::SetCandleUsed();

    for ( int i = FirstFireSlot; i < LastFireSlot; i++ )
    {
        if ( nullptr != World::GetObject( i ) )
            continue;

        Util::MoveSimple( x, y, facingDir, 0x10 );

        Sound::PlayEffect( SEffect_fire );

        Fire* fire = new Fire();
        fire->SetX( x );
        fire->SetY( y );
        fire->SetMoving( facingDir );
        World::SetObject( i, fire );
        return 12;
    }
    return 0;
}

int UseBomb( int x, int y, Direction facingDir )
{
    int i;

    for ( i = FirstBombSlot; i < LastBombSlot; i++ )
    {
        Object* obj = World::GetObject( i );
        if ( obj == nullptr || obj->GetType() != Obj_Bomb )
            break;
    }

    if ( i == LastBombSlot )
        return 0;

    int freeSlot = i;
    int otherSlot = FirstBombSlot;

    if ( freeSlot == FirstBombSlot )
        otherSlot++;

    Object* otherObj = World::GetObject( otherSlot );
    if ( otherObj != nullptr 
        && otherObj->GetType() == Obj_Bomb 
        && ((Bomb*) otherObj)->GetLifetimeState() < Bomb::Blasting )
        return 0;

    Util::MoveSimple( x, y, facingDir, 0x10 );

    Bomb* bomb = new Bomb();
    bomb->SetX( x );
    bomb->SetY( y );
    World::SetObject( freeSlot, bomb );
    World::DecrementItem( ItemSlot_Bombs );
    Sound::PlayEffect( SEffect_put_bomb );
    return 7;
}

int UseBoomerang( int x, int y, Direction facingDir )
{
    // ORIGINAL: Trumps food. Look at $05:8E40. The behavior is tied to the statement below.
    //           Skip throw, if there's already a boomerang in the slot. But overwrite Food.
    if ( nullptr != World::GetObject( BoomerangSlot ) )
        return 0;

    int itemValue = World::GetItem( ItemSlot_Boomerang );
    int distance = 0x31;

    Util::MoveSimple( x, y, facingDir, 0x10 );

    if ( itemValue == 2 )
        distance = 0xFF;

    Direction moving = World::GetPlayer()->GetMoving();
    if ( moving != Dir_None )
        facingDir = moving;

    Boomerang* boomerang = MakeBoomerang( 
        x, y, facingDir, distance, 3.0, PlayerSlot, BoomerangSlot );
    World::SetObject( BoomerangSlot, boomerang );
    return 6;
}

int UseArrow( int x, int y, Direction facingDir )
{
    if ( nullptr != World::GetObject( ArrowSlot ) )
        return 0;

    if ( World::GetItem( ItemSlot_Rupees ) == 0 )
        return 0;

    World::PostRupeeLoss( 1 );

    Util::MoveSimple( x, y, facingDir, 0x10 );

    if ( Util::IsVertical( facingDir ) )
        x += 3;

    Object* arrow = MakeProjectile( Obj_Arrow, x, y, facingDir, ArrowSlot );
    World::SetObject( ArrowSlot, arrow );
    Sound::PlayEffect( SEffect_boomerang );
    return 6;
}

int UseFood( int x, int y, Direction facingDir )
{
    if ( nullptr != World::GetObject( FoodSlot ) )
        return 0;

    Util::MoveSimple( x, y, facingDir, 0x10 );

    Food* food = new Food( x, y );
    World::SetObject( FoodSlot, food );
    return 6;
}

int UsePotion( int x, int y, Direction facingDir )
{
    World::DecrementItem( ItemSlot_Potion );
    World::PauseFillHearts();
    return 0;
}

int UseRecorder( int x, int y, Direction facingDir )
{
    World::UseRecorder();
    return 0;
}

int UseLetter( int x, int y, Direction facingDir )
{
    int itemValue = World::GetItem( ItemSlot_Letter );
    if ( itemValue != 1 )
        return 0;

    Object* obj = World::GetObject( MonsterSlot1 );
    if ( obj == nullptr || obj->GetType() != Obj_CaveMedicineShop )
        return 0;

    World::SetItem( ItemSlot_Letter, 2 );
    return 0;
}

int Player::UseItem()
{
    Profile& profile = World::GetProfile();
    if ( profile.SelectedItem == 0 )
        return 0;

    int itemValue = profile.Items[profile.SelectedItem];
    if ( itemValue == 0 )
        return 0;

    if ( profile.SelectedItem == ItemSlot_Rod )
        return UseWeapon( Obj_Rod, ItemSlot_Rod );

    int waitFrames = 0;

    switch ( profile.SelectedItem )
    {
    case ItemSlot_Bombs:    waitFrames = UseBomb( objX, objY, facing ); break;
    case ItemSlot_Arrow:    waitFrames = UseArrow( objX, objY, facing ); break;
    case ItemSlot_Candle:   waitFrames = UseCandle( objX, objY, facing ); break;
    case ItemSlot_Recorder: waitFrames = UseRecorder( objX, objY, facing ); break;
    case ItemSlot_Food:     waitFrames = UseFood( objX, objY, facing ); break;
    case ItemSlot_Potion:   waitFrames = UsePotion( objX, objY, facing ); break;
    case ItemSlot_Letter:   waitFrames = UseLetter( objX, objY, facing ); break;
    case ItemSlot_Boomerang:waitFrames = UseBoomerang( objX, objY, facing ); break;
    }

    if ( waitFrames == 0 )
        return 0;
    animator.time = 0;
    animTimer = 6;
    state = 0x16;
    return waitFrames + 6;
}

int Player::UseWeapon()
{
    if ( World::IsSwordBlocked() || World::GetStunTimer( NoSwordTimerSlot ) != 0 )
        return 0;
    return UseWeapon( Obj_PlayerSword, ItemSlot_Sword );
}

int Player::UseWeapon( ObjType type, int itemSlot )
{
    if ( World::GetItem( itemSlot ) == 0 )
        return 0;

    if ( World::GetObject( PlayerSwordSlot ) != nullptr )
        return 0;

    // The original game did this:
    //   player.animTimer := 1
    //   player.state := $10
    animator.time = 0;
    animTimer = 12;
    state = 0x11;

    PlayerSword* sword = new PlayerSword( type );
    World::SetObject( PlayerSwordSlot, sword );
    Sound::PlayEffect( SEffect_sword );
    return 13;
}
