/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Object.h"
#include "Graphics.h"
#include "ItemObj.h"
#include "ObjType.h"
#include "Player.h"
#include "PlayerItemsAnim.h"
#include "Profile.h"
#include "Sound.h"
#include "SoundId.h"
#include "SpriteAnimator.h"
#include "World.h"
#include "Monsters.h"


//----------------------------------------------------------------------------
//  Object
//----------------------------------------------------------------------------

enum DamageType
{
    Damage_Sword        = 1,
    Damage_Boomerang    = 2,
    Damage_Arrow        = 4,
    Damage_Unknown8__   = 8,
    Damage_MagicWave    = 0x10,
    Damage_Bomb         = 0x20,
};

enum TileCollisionStep
{
    TileCollision_CheckTile,
    TileCollision_NextDir,
};


Object::Object( ObjType type )
    :   type( type ),
        isDeleted( false ),
        decoration( 1 ),
        invincibilityTimer( 0 ),
        invincibilityMask( 0 ),
        shoveDir( 0 ),
        shoveDistance( 0 ),
        facing( Dir_None ),
        objX( 0 ),
        objY( 0 ),
        tileOffset( 0 ),
        fraction( 0 ),
        moving( 0 ),
        objTimer( 0 ),
        stunTimer( 0 ),
        objFlags( 0 )
{
    assert( type != 0 );

    if ( type < Obj_Person_End 
        && type != Obj_Armos && type != Obj_FlyingGhini )
    {
        int slot = World::GetCurrentObjectSlot();
        int time = slot + 1;
        objTimer = time;
    }

    hp = World::Get()->GetObjectMaxHP( type );
}

Object::~Object()
{
}

ObjType Object::GetType()
{
    return (ObjType) type;
}

bool Object::IsDeleted()
{
    return isDeleted;
}

void Object::SetDeleted()
{
    isDeleted = true;
}

bool Object::IsPlayer()
{
    return GetType() == Obj_Player;
}

int Object::GetDecoration()
{
    return decoration;
}

void Object::SetDecoration( int decoration )
{
    this->decoration = decoration;
}

Direction Object::GetFacing()
{
    return facing;
}

Direction Object::GetMoving()
{
    return (Direction) moving;
}

uint Object::GetX()
{
    return objX;
}

uint Object::GetY()
{
    return objY;
}

void Object::SetX( uint x )
{
    objX = x;
}

void Object::SetY( uint y )
{
    objY = y;
}

int Object::GetTileOffset()
{
    return tileOffset;
}

void Object::SetTileOffset( int value )
{
    tileOffset = value;
}

uint Object::GetObjectTimer()
{
    return objTimer;
}

void Object::SetObjectTimer( uint value )
{
    objTimer = value;
}

uint Object::GetStunTimer()
{
    return stunTimer;
}

void Object::SetStunTimer( uint value )
{
    stunTimer = value;
}

void Object::SetShoveDir( Direction dir )
{
    shoveDir = dir;
}

void Object::SetShoveDistance( int distance )
{
    shoveDistance = distance;
}

ObjFlags Object::GetFlags()
{
    return objFlags;
}

void* Object::GetInterface( ObjInterfaces iface )
{
    return nullptr;
}

void Object::DecrementObjectTimer()
{
    if ( objTimer != 0 )
        objTimer--;
}

void Object::DecrementStunTimer()
{
    if ( stunTimer != 0 )
        stunTimer--;
}

bool Object::DecoratedUpdate()
{
    if ( invincibilityTimer > 0 && (GetFrameCounter() & 1) == 0 )
        invincibilityTimer--;

    if ( decoration == 0 )
    {
        Update();

        if ( decoration == 0 )
        {
            int slot = World::GetCurrentObjectSlot();
            ObjectAttr attrs = World::Get()->GetObjectAttrs( GetType() );

            if ( slot < BufferSlot && !attrs.GetCustomCollision() )
            {
                // ORIGINAL: flag 4 if custom draw. If not set, then call $77D4.
                // But, this stock drawing code is used for very few objects. This includes 
                // lynel, moblin, goriya, and some env objects like block. See $ECFE.

                CheckCollisions();
            }
        }
    }
    else if ( decoration == 0x10 )
    {
        objTimer = 6;
        decoration++;
    }
    else
    {
        if ( objTimer == 0 )
        {
            objTimer = 6;
            decoration++;
        }

        if ( decoration == 4 )
            decoration = 0;
        else if ( decoration == 0x14 )
        {
            isDeleted = true;
            return true;
        }
    }
    return false;
}

void Object::DecoratedDraw()
{
    if ( decoration == 0 )
    {
        Draw();
    }
    else if ( decoration < 0x10 )
    {
        int frame = decoration - 1;
        SpriteAnimator animator;
        animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Cloud );
        animator.DrawFrame( Sheet_PlayerAndItems, GetX(), GetY(), BlueFgPalette, frame );
    }
    else
    {
        int counter = (GetFrameCounter() >> 1) & 3;
        int frame = (decoration + 1) % 2;
        int pal = PlayerPalette + (ForegroundPalCount - counter - 1);
        SpriteAnimator animator;
        animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Sparkle );
        animator.DrawFrame( Sheet_PlayerAndItems, GetX(), GetY(), pal, frame );
    }
}

int Object::CalcPalette( int wantedPalette )
{
    if ( invincibilityTimer == 0 )
        return wantedPalette;

    return PlayerPalette + (invincibilityTimer & 3);
}

bool Object::IsStunned()
{
    if ( World::GetItem( ItemSlot_Clock ) != 0 )
        return true;

    return stunTimer != 0;
}

bool Object::CheckCollisions()
{
    ObjectAttr attr = World::Get()->GetObjectAttrs( type );

    if ( !attr.GetInvincibleToWeapons() )
    {
        if ( invincibilityTimer != 0 )
            return false;

        CheckBoomerang( BoomerangSlot );
        CheckWave( PlayerSwordShotSlot );
        CheckBombAndFire( BombSlot );
        CheckBombAndFire( BombSlot2 );
        CheckBombAndFire( FireSlot );
        CheckBombAndFire( FireSlot2 );
        CheckSword( PlayerSwordSlot );
        CheckArrow( ArrowSlot );
    }

    return CheckPlayerCollision();

    // The original game checked special Wallmaster, Like-Like, and Goriya states here.
    // But, we check instead in each of those classes.
}

void Object::CheckBoomerang( int slot )
{
    Point box = { 0xA, 0xA };
    Point weaponCenter = { 4, 8 };

    CollisionContext context;
    context.WeaponSlot = slot;
    context.DamageType = Damage_Boomerang;
    context.Damage     = 0;

    CheckCollisionCustomNoShove( context, box, weaponCenter );
}

static const uint8_t swordPowers[] = { 0, 0x10, 0x20, 0x40 };

void Object::CheckWave( int slot )
{
    Object* weaponObj = World::GetObject( slot );
    if ( weaponObj == nullptr )
        return;

    if ( weaponObj->GetType() == Obj_PlayerSwordShot )
    {
        PlayerSwordShot* wave = (PlayerSwordShot*) weaponObj;
        if ( wave->GetLifetimeState() != Shot::Flying )
            return;
    }

    Point box = { 0xC, 0xC };

    CollisionContext context;
    context.WeaponSlot = slot;

    if ( weaponObj->GetType() == Obj_MagicWave )
    {
        context.DamageType  = Damage_MagicWave;
        context.Damage      = 0x20;
    }
    else
    {
        int itemValue       = World::GetItem( ItemSlot_Sword );
        context.DamageType  = Damage_Sword;
        context.Damage      = swordPowers[itemValue];
    }

    if ( CheckCollisionNoShove( context, box ) )
    {
        ShoveCommon( context );

        if ( weaponObj->GetType() == Obj_PlayerSwordShot )
        {
            PlayerSwordShot* wave = (PlayerSwordShot*) weaponObj;
            wave->SpreadOut();
        }
        else if ( weaponObj->GetType() == Obj_MagicWave )
        {
            MagicWave* wave = (MagicWave*) weaponObj;
            wave->AddFire();
            wave->SetDeleted();
        }
    }
}

void Object::CheckBombAndFire( int slot )
{
    Object* obj = World::GetObject( slot );
    if ( obj == nullptr )
        return;

    if ( obj->GetType() == Obj_Bomb && ((Bomb*) obj)->GetLifetimeState() != Bomb::Blasting )
        return;

    Point box = { 0xE, 0xE };
    Point weaponCenter = { 8, 8 };

    CollisionContext context;
    context.DamageType  = Damage_Bomb;
    context.WeaponSlot  = slot;
    context.Damage      = 0x10;

    if ( CheckCollisionCustomNoShove( context, box, weaponCenter ) )
    {
        if ( (invincibilityMask & context.DamageType) == 0 )
            Shove( context );
    }
}

void Object::CheckSword( int slot )
{
    PlayerSword* weaponObj = (PlayerSword*) World::GetObject( slot );
    if ( weaponObj == nullptr || weaponObj->GetState() != 1 )
        return;

    Point   box;
    Player* player = World::GetPlayer();

    if ( Util::IsVertical( player->GetFacing() ) )
    {
        box.X = 0xC;
        box.Y = 0x10;
    }
    else
    {
        box.X = 0x10;
        box.Y = 0xC;
    }

    CollisionContext   context;
    context.WeaponSlot = slot;
    context.DamageType = Damage_Sword;
    context.Damage     = 0;

    if ( weaponObj->GetType() == Obj_PlayerSword )
    {
        int     itemValue  = World::GetItem( ItemSlot_Sword );
        int     power      = swordPowers[itemValue];
        context.Damage     = power;
    }
    else if ( weaponObj->GetType() == Obj_Rod )
    {
        context.Damage     = 0x20;
    }

    if ( CheckCollisionNoShove( context, box ) )
        ShoveCommon( context );
}

int arrowPowers[] = { 0, 0x20, 0x40 };

bool Object::CheckArrow( int slot )
{
    Object* weaponObj = World::GetObject( slot );
    if ( weaponObj == nullptr )
        return false;

    Arrow* arrow = (Arrow*) weaponObj;
    if ( arrow->GetLifetimeState() != Arrow::Flying )
        return false;

    int   itemValue = World::GetItem( ItemSlot_Arrow );
    Point box = { 0xB, 0xB };

    CollisionContext context;
    context.WeaponSlot = slot;
    context.DamageType = Damage_Arrow;
    context.Damage     = arrowPowers[itemValue];

    if ( CheckCollisionNoShove( context, box ) )
    {
        ShoveCommon( context );

        if ( GetType() == Obj_PolsVoice )
        {
            hp = 0;
            DealDamage( context );
        }
        else
        {
            arrow->SetSpark();
        }
        return true;
    }
    return false;
}

Point Object::CalcObjMiddle()
{
    if ( GetType() == Obj_Ganon )
    {
        Point p = { GetX() + 0x10, GetY() + 0x10 };
        return p;
    }

    Point p = { GetX(), GetY() + 8 };
    ObjectAttr attr = World::Get()->GetObjectAttrs( type );
    if ( attr.GetHalfWidth() )
        p.X += 4;
    else
        p.X += 8;
    return p;
}

bool Object::CheckCollisionNoShove( 
    CollisionContext& context,
    Point box )
{
    Player* player = World::GetPlayer();
    Point   weaponCenter;

    if ( Util::IsVertical( player->GetFacing() ) )
    {
        weaponCenter.X = 6;
        weaponCenter.Y = 8;
    }
    else
    {
        weaponCenter.X = 8;
        weaponCenter.Y = 6;
    }

    return CheckCollisionCustomNoShove( context, box, weaponCenter );
}

bool Object::CheckCollisionCustomNoShove( 
    CollisionContext& context,
    Point box,
    Point weaponOffset )
{
    Object* weaponObj = World::GetObject( context.WeaponSlot );
    if ( weaponObj == nullptr )
        return false;

    Point objCenter = CalcObjMiddle();
    weaponOffset.X += weaponObj->GetX();
    weaponOffset.Y += weaponObj->GetY();

    if ( !DoObjectsCollide( objCenter, weaponOffset, box, context.Distance ) )
        return false;

    if ( weaponObj->GetType() == Obj_Boomerang )
    {
        Boomerang* boomerang = (Boomerang*) weaponObj;
        boomerang->SetState( 5 );

        if ( (invincibilityMask & context.DamageType) != 0 )
        {
            PlayParrySound();
            return true;
        }

        stunTimer = 0x10;
    }

    HandleCommonHit( context );
    return true;
}

bool Object::DoObjectsCollide( Point obj1, Point obj2, Point box, Point& distance )
{
    distance.X = abs( obj2.X - obj1.X );
    if ( distance.X < box.X )
    {
        distance.Y = abs( obj2.Y - obj1.Y );
        if ( distance.Y < box.Y )
            return true;
    }
    return false;
}

void Object::HandleCommonHit( CollisionContext& context )
{
    if ( (invincibilityMask & context.DamageType) != 0 )
    {
        PlayParrySoundIfSupported( context.DamageType );
        return;
    }

    Object* weaponObj = World::GetObject( context.WeaponSlot );

    if ( GetType() == Obj_BlueGohma || GetType() == Obj_RedGohma )
    {
        if ( context.WeaponSlot == ArrowSlot )
        {
            Arrow* arrow = (Arrow*) weaponObj;
            arrow->SetSpark( 4 );
        }

        Gohma*  gohma = (Gohma*) this;

        if ( ((gohma->GetCurrentCheckPart() == 3) || (gohma->GetCurrentCheckPart() == 4))
            && gohma->GetEyeFrame() == 3
            && weaponObj->GetFacing() == Dir_Up )
        {
            Sound::PlayEffect( SEffect_boss_hit );
            Sound::StopEffect( Sound::AmbientInstance );
            DealDamage( context );
            // The original game plays sounds again. But why, if we already played boss hit effect?
        }

        Sound::PlayEffect( SEffect_parry );
        return;
    }
    else if ( GetType() == Obj_Zol || GetType() == Obj_Vire )
    {
        if ( context.WeaponSlot != BoomerangSlot )
            facing = weaponObj->GetFacing();
    }
    else if ( GetType() == Obj_RedDarknut || GetType() == Obj_BlueDarknut )
    {
        uint combinedDir = facing | weaponObj->GetFacing();

        if ( combinedDir == 3 || combinedDir == 0xC )
        {
            PlayParrySoundIfSupported( context.DamageType );
            return;
        }
    }

    DealDamage( context );
}

void Object::DealDamage( CollisionContext& context )
{
    Sound::PlayEffect( SEffect_monster_hit );

    if ( hp < context.Damage )
    {
        KillObjectNormally( context );
    }
    else
    {
        hp -= context.Damage;
        if ( hp == 0 )
            KillObjectNormally( context );
    }
}

void Object::KillObjectNormally( CollisionContext& context )
{
    // TODO: what deals this damage type?
    bool allowBombDrop = context.DamageType == Damage_Unknown8__;

    World::Get()->IncrementKilledObjectCount( allowBombDrop );

    decoration = 0x10;
    Sound::PlayEffect( SEffect_monster_die );

    stunTimer = 0;
    shoveDir = 0;
    shoveDistance = 0;
    invincibilityTimer = 0;
}

void Object::PlayParrySoundIfSupported( int damageType )
{
    if (   (invincibilityMask & Damage_Bomb) == 0
        && (invincibilityMask & Damage_Unknown8__) == 0 )
    {
        PlayParrySound();
    }
}

void Object::PlayParrySound()
{
    Sound::PlayEffect( SEffect_parry );
}

void Object::PlayBossHitSoundIfHit()
{
    if ( invincibilityTimer == 0x10 )
        Sound::PlayEffect( SEffect_boss_hit );
}

void Object::PlayBossHitSoundIfDied()
{
    if ( decoration != 0 )
    {
        Sound::PlayEffect( SEffect_boss_hit );
        Sound::StopEffect( Sound::AmbientInstance );
    }
}

void Object::ShoveCommon( CollisionContext& context )
{
    if ( GetType() == Obj_RedDarknut || GetType() == Obj_BlueDarknut )
    {
        Object* weaponObj   = World::GetObject( context.WeaponSlot );
        uint    combinedDir = facing | weaponObj->GetFacing();

        if ( combinedDir == 3 || combinedDir == 0xC )
        {
            PlayParrySound();
            return;
        }
    }

    Shove( context );
}

void Object::Shove( CollisionContext& context )
{
    if ( (invincibilityMask & context.DamageType) != 0 )
        return;

    if ( context.WeaponSlot != 0 )
    {
        ShoveObject( context );
    }
    else
    {
        Player* player = World::GetPlayer();

        if ( player->GetInvincibilityTimer() != 0 )
            return;

        bool useY = false;

        if ( player->GetTileOffset() == 0 )
        {
            if ( context.Distance.Y >= 4 )
                useY = true;
        }
        else
        {
            if ( Util::IsVertical( player->GetFacing() ) )
                useY = true;
        }

        Direction dir = Dir_None;

        if ( useY )
        {
            dir = objY < player->GetY() ? Dir_Down : Dir_Up;
        }
        else
        {
            dir = objX < player->GetX() ? Dir_Right : Dir_Left;
        }

        player->SetShoveDir( (Direction) (dir | 0x80) );
        player->SetInvincibilityTimer( 0x18 );
        player->SetShoveDistance( 0x20 );

        if ( World::GetCurrentObjectSlot() >= BufferSlot )
            return;

        ObjectAttr attr = World::Get()->GetObjectAttrs( GetType() );
        if (   attr.GetUnknown80__() 
            || GetType() == Obj_Vire )
            return;

        facing = Util::GetOppositeDir( facing );
    }
}

void Object::ShoveObject( CollisionContext& context )
{
    if ( invincibilityTimer != 0 )
        return;

    Object* weaponObj = World::GetObject( context.WeaponSlot );
    int dir = weaponObj->GetFacing();

    ObjectAttr attr = World::Get()->GetObjectAttrs( GetType() );
    if ( attr.GetUnknown80__() )
        dir |= 0x40;

    if ( GetType() == Obj_BlueGohma || GetType() == Obj_RedGohma )
    {
        Gohma*  gohma = (Gohma*) this;

        if ( ((gohma->GetCurrentCheckPart() != 3) && (gohma->GetCurrentCheckPart() != 4))
            || gohma->GetEyeFrame() != 3
            || weaponObj->GetFacing() != Dir_Up )
            return;
    }

    shoveDir = dir | 0x80;
    shoveDistance = 0x40;
    invincibilityTimer = 0x10;
}

PlayerCollision Object::CheckPlayerCollision()
{
    // The original resets [$C] and [6] here. [6] gets the result of DoObjectsCollide.
    // [$C] takes on the same values as [6], so I don't know why it was needed.

    Player* player = World::GetPlayer();

    if (   IsStunned()
        || player->GetStunTimer() != 0 
        || player->GetInvincibilityTimer() != 0 )
        return false;

    return CheckPlayerCollisionDirect();
}

PlayerCollision Object::CheckPlayerCollisionDirect()
{
    Player* player = World::GetPlayer();

    if (   player->GetState() == Player::Paused 
        || player->IsParalyzed() )
        return false;

    IShot* shot = (IShot*) GetInterface( ObjItf_IShot );
    if ( shot != nullptr && !shot->IsInShotStartState() )
        return false;

    Point   objCenter = CalcObjMiddle();
    Point   playerCenter = player->GetMiddle();
    Point   box = { 9, 9 };
    Point   distance;

    if ( !DoObjectsCollide( objCenter, playerCenter, box, distance ) )
        return false;

    CollisionContext context = { 0 };
    context.Distance = distance;

    if ( GetType() < Obj_Person_End )
    {
        Shove( context );
        player->BeHarmed( this, objCenter );
        return true;
    }

    if ( GetType() == Obj_Fireball2 || GetType() == 0x5A
        || player->GetState() != Player::Idle )
    {
        Shove( context );
        player->BeHarmed( this, objCenter );
        return PlayerCollision( true, true );
    }

    if ( ((facing | player->GetFacing()) & 0xC) != 0xC
        && ((facing | player->GetFacing()) & 3) != 3 )
    {
        Shove( context );
        player->BeHarmed( this, objCenter );
        return PlayerCollision( true, true );
    }

    if ( GetType() >= Obj_Fireball && GetType() < Obj_Arrow
        && World::GetItem( ItemSlot_MagicShield ) == 0 )
    {
        Shove( context );
        player->BeHarmed( this, objCenter );
        return PlayerCollision( true, true );
    }

    Sound::PlayEffect( SEffect_parry );
    return PlayerCollision( false, true );
}

Direction CheckWorldMarginH( int x, Direction dir, bool adjust )
{
    Direction   curDir = Dir_Left;

    if ( adjust )
        x += 0xB;

    if ( x > World::GetMarginLeft() )
    {
        if ( adjust )
            x -= 0x17;

        curDir = Dir_Right;

        if ( x < World::GetMarginRight() )
            return dir;
    }

    if ( (dir & curDir) != 0 )
        return Dir_None;

    return dir;
}

Direction CheckWorldMarginV( int y, Direction dir, bool adjust )
{
    Direction   curDir = Dir_Up;

    if ( adjust )
        y += 0xF;

    if ( y > World::GetMarginTop() )
    {
        if ( adjust )
            y -= 0x21;

        curDir = Dir_Down;

        if ( y < World::GetMarginBottom() )
            return dir;
    }

    if ( (dir & curDir) != 0 )
        return Dir_None;

    return dir;
}

Direction Object::CheckWorldMargin( Direction dir )
{
    int     slot = World::GetCurrentObjectSlot();
    bool    adjust = (slot > BufferSlot) || (GetType() == Obj_Ladder);

    // ORIGINAL: This isn't needed, because the player is first (slot=0).
    if ( slot >= PlayerSlot )
        adjust = false;

    dir = CheckWorldMarginH( objX, dir, adjust );
    return CheckWorldMarginV( objY, dir, adjust );
}

Direction Object::CheckTileCollision( Direction dir )
{
    if ( IsPlayer() )
    {
        if ( World::GetDoorwayDir() != Dir_None )
            return CheckWorldBounds( dir );
        // Original, but seemingly never triggered: if [$E] < 0, leave
    }

    if ( tileOffset != 0 )
        return dir;

    if ( dir != Dir_None )
    {
        return FindUnblockedDir( dir, TileCollision_CheckTile );
    }

    if ( IsPlayer() )
        return dir;

    // The original handles objAttr $10 here, but no object seems to have it.

    dir = (Direction) moving;
    return FindUnblockedDir( dir, TileCollision_NextDir );
}

bool Object::HitsWorldLimit()
{
    if ( moving != 0 )
    {
        Limits playerLimits = Player::GetPlayerLimits();

        int dirOrd = Util::GetDirectionOrd( (Direction) moving );
        Direction singleMoving = Util::GetOrdDirection( dirOrd );
        uint coord = Util::IsVertical( singleMoving ) ? objY : objX;

        if ( coord == playerLimits[dirOrd] )
        {
            facing = singleMoving;
            return true;
        }
    }
    return false;
}

void Object::StopPlayer()
{
    ((Player*) this)->Stop();
}

// F14E
Direction Object::CheckWorldBounds( Direction dir )
{
    if ( IsPlayer() )
    {
        if ( World::GetMode() == Mode_Play 
            && nullptr == World::GetLadder() 
            && tileOffset == 0 )
        {
            if ( HitsWorldLimit() )
            {
                World::LeaveRoom( facing, World::Get()->GetRoomId() );
                dir = Dir_None;
                StopPlayer();
            }
        }
    }
    else
    {
        dir = CheckWorldMargin( dir );
        if ( dir != Dir_None )
        {
            facing = dir;
            return dir;
        }
    }

    return dir;
}

Direction Object::GetSingleMoving()
{
    int dirOrd = Util::GetDirectionOrd( (Direction) moving );
    return Util::GetOrdDirection( dirOrd );
}

Direction Object::FindUnblockedDir( Direction dir, int firstStep )
{
    TileCollision collision;
    int i = 0;

    if ( firstStep == TileCollision_NextDir )
        goto NextDir;

    do
    {
        collision = World::CollidesWithTileMoving( objX, objY, dir, IsPlayer() );
        if ( !collision.Collides )
        {
            dir = CheckWorldBounds( dir );
            if ( IsPlayer() || dir != Dir_None )
                return dir;
        }

        if ( IsPlayer() )
        {
            if ( World::Get()->IsOverworld() )
            {
                PushOWTile( collision );
            }
            dir = Dir_None;
            // ORIGINAL: [$F8] := 0
            if ( World::Get()->IsOverworld() )
                return CheckWorldBounds( dir );
            return dir;
        }

        // The original handles objAttr $10 here, but no object seems to have it.

NextDir:
        dir = GetNextAltDir( i, dir );
    } while ( i != 0 );

    return dir;
}

void Object::TurnToUnblockedDir()
{
    if ( tileOffset != 0 )
        return;

    Direction dir = Dir_None;
    int i = 0;

    while ( true )
    {
        dir = GetNextAltDir( i, dir );
        if ( dir == Dir_None )
            return;

        if ( !World::CollidesWithTileMoving( objX, objY, dir, IsPlayer() ) )
        {
            dir = CheckWorldMargin( dir );
            if ( dir != Dir_None )
            {
                facing = dir;
                return;
            }
        }
    }
}

Direction Object::GetNextAltDir( int& seq, Direction dir )
{
    static const Direction dirs[] = { Dir_Up, Dir_Down, Dir_Left, Dir_Right };

    switch ( seq++ )
    {
    // Choose a random direction perpendicular to facing.
    case 0:
        {
            int index = 0;
            int r = Util::GetRandom( 256 );
            if ( (r & 1) == 0 )
                index++;
            if ( Util::IsVertical( facing ) )
                index += 2;
            return dirs[index];
        }

    case 1:
        return Util::GetOppositeDir( dir );

    case 2:
        facing = Util::GetOppositeDir( facing );
        return facing;

    default:
        seq = 0;
        return Dir_None;
    }
}

// $01:A223
void Object::PushOWTile( TileCollision& collision )
{
    if ( tileOffset != 0 || moving == 0 )
        return;

    // This isn't anologous to the original's code, but the effect is the same.
    World::PushTile( collision.FineRow, collision.FineCol );
}

Direction Object::StopAtPersonWall( Direction dir )
{
    if ( objY < 0x8E && (dir & Dir_Up) != 0 )
        return Dir_None;
    return dir;
}

Direction Object::StopAtPersonWallUW( Direction dir )
{
    // ($6E46) if first object is grumble or person, block movement up above $8E.

    Object* firstObj = World::GetObject( MonsterSlot1 );

    if ( firstObj != nullptr )
    {
        ObjType type = firstObj->GetType();
        if (    type == Obj_Grumble
            || (type >= Obj_Person1 && type < Obj_Person_End) )
        {
            return StopAtPersonWall( dir );
        }
    }

    return dir;
}

// 8ED7
Direction Object::CheckSubroom( Direction dir )
{
    GameMode mode = World::GetMode();

    if ( mode == Mode_PlayCellar )
    {
        if ( objY >= 0x40 || (moving & Dir_Up) == 0 )
            return dir;

        World::LeaveCellar();
        dir = Dir_None;
        StopPlayer();
    }
    else
    {
        dir = StopAtPersonWall( dir );

        // Handling 3 shortcut stairs in shortcut cave is handled by the Person obj, instead of here.

        if ( HitsWorldLimit() )
        {
            World::LeaveCellar();
            dir = Dir_None;
            StopPlayer();
        }
    }

    return dir;
}

static void GetFacingCoords( Object* obj, int& paralCoord, int& orthoCoord )
{
    if ( Util::IsVertical( obj->GetFacing() ) )
    {
        paralCoord = obj->GetY();
        orthoCoord = obj->GetX();
    }
    else
    {
        paralCoord = obj->GetX();
        orthoCoord = obj->GetY();
    }
}

// $05:917C
Direction Object::CheckDoorways( Direction dir )
{
    static const uint8_t orthoCoords[]  = { 0x8D, 0x8D, 0x78, 0x78 };
    static const uint8_t minInside[]    = { 0xCF, 0x00, 0xBD, 0x3D };
    static const uint8_t maxInside[]    = { 0xF1, 0x21, 0xDE, 0x5E };
    static const uint8_t minOutside[]   = { 0xD2, 0x00, 0xBF, 0x3D };
    static const uint8_t maxOutside[]   = { 0xF1, 0x1F, 0xDE, 0x5C };

    int paralCoord;
    int orthoCoord;
    GetFacingCoords( this, paralCoord, orthoCoord );

    if ( World::GetDoorwayDir() != Dir_None )
    {
        int dirOrd = Util::GetDirectionOrd( World::GetDoorwayDir() );

        if (   paralCoord <  minInside[dirOrd] 
            || paralCoord >= maxInside[dirOrd]
            || World::GetDoorwayDir() != facing )
        {
            for ( int i = 0; i < 4; i++ )
            {
                if (   orthoCoord == orthoCoords[i]
                    && paralCoord >= minOutside[i]
                    && paralCoord <  maxOutside[i] )
                {
                    return CheckDoor( dir, i );
                }
            }
            World::SetDoorwayDir( Dir_None );
            return dir;
        }
    }

    for ( int i = 0; i < 4; i++ )
    {
        if (   orthoCoord == orthoCoords[i]
            && paralCoord >= minInside[i]
            && paralCoord <  maxInside[i] )
        {
            return CheckDoor( dir, i );
        }
    }

    World::SetDoorwayDir( Dir_None );
    return dir;
}

// 91D6
Direction Object::CheckDoor( Direction dir, int dirOrd )
{
    Direction movingDir = (Direction) (moving & 0xF);

    if ( movingDir != Util::GetOrdDirection( dirOrd ) )
        return dir;

    // At this point, moving is a cardinal direction.

    DoorType    doorType = World::GetDoorType( (Direction) movingDir );
    bool        blocked = false;
    Player*     player = World::GetPlayer();

    switch ( doorType )
    {
    case DoorType_Open:
        break;

    case DoorType_None:
        blocked = true;
        break;

    case DoorType_FalseWall:
    case DoorType_FalseWall2:
        if ( player->GetObjectTimer() == 0 )
        {
            player->SetObjectTimer( 0x18 );
            blocked = true;
        }
        else if ( player->GetObjectTimer() != 1 )
        {
            blocked = true;
        }
        break;

    case DoorType_Bombable:
        if ( !World::GetDoorState( movingDir ) )
            blocked = true;
        break;

    case DoorType_Key:
    case DoorType_Key2:
        if ( !World::GetDoorState( movingDir ) )
        {
            if ( World::GetTriggeredDoorCmd() != 0 )
            {
                if ( player->GetObjectTimer() != 0 )
                    blocked = true;
                else
                    ;
            }
            else
            {
                bool canOpen = true;

                if (   World::GetItem( ItemSlot_MagicKey ) == 0 )
                {
                    if ( World::GetItem( ItemSlot_Keys ) == 0 )
                        canOpen = false;
                    else
                        World::DecrementItem( ItemSlot_Keys );
                }

                if ( canOpen )
                {
                    // $8ADA
                    World::SetTriggeredDoorDir( movingDir );
                    World::SetTriggeredDoorCmd( 6 );

                    player->SetObjectTimer( 0x20 );
                }

                blocked = true;
            }
        }
        break;

    case DoorType_Shutter:
        if (    World::GetTriggeredDoorCmd() != 0 
            || !World::GetDoorState( movingDir ) )
        {
            blocked = true;
        }
        else
        {
            if ( (movingDir & World::GetShuttersPassedDirs()) != 0 )
            {
                if ( player->GetObjectTimer() != 0 )
                    blocked = true;
            }
            else
            {
                World::SetShuttersPassedDirs( World::GetShuttersPassedDirs() | movingDir );
            }
        }
        break;
    }

    if ( blocked )
        return dir;

    facing = movingDir;
    dir = movingDir;
    World::SetDoorwayDir( movingDir );

    if (   doorType == DoorType_FalseWall
        || doorType == DoorType_FalseWall2
        || doorType == DoorType_Bombable )
    {
        World::LeaveRoom( facing, World::GetRoomId() );
        dir = Dir_None;
        StopPlayer();
    }

    return dir;
}

// 8F7B
Direction Object::HandleLadder( Direction dir )
{
    Player* player = World::GetPlayer();
    Ladder* ladder = World::GetLadder();

    if ( ladder == nullptr )
        return dir;

    // Original: if ladder->GetState() = 0, destroy it. But, I don't see how it can get in that state.

    int distance = 0;

    if ( Util::IsVertical( ladder->GetFacing() ) )
    {
        if ( player->GetX() != ladder->GetX() )
        {
            World::SetLadder( nullptr );
            return dir;
        }
        distance = (player->GetY() + 3) - ladder->GetY();
    }
    else
    {
        if ( (player->GetY() + 3) != ladder->GetY() )
        {
            World::SetLadder( nullptr );
            return dir;
        }
        distance = player->GetX() - ladder->GetX();
    }

    distance = abs( distance );

    if ( distance < 0x10 )
    {
        ladder->SetState( 2 );
        dir = MoveOnLadder( dir, distance );
    }
    else if ( distance != 0x10
        || player->GetFacing() != ladder->GetFacing() )
    {
        World::SetLadder( nullptr );
    }
    else if ( ladder->GetState() == 1 )
    {
        dir = MoveOnLadder( dir, distance );
    }
    else
    {
        World::SetLadder( nullptr );
    }

    return dir;
}

// $05:8FCD
Direction Object::MoveOnLadder( Direction dir, int distance )
{
    if ( moving == 0 )
        return Dir_None;

    Player* player = World::GetPlayer();
    Ladder* ladder = World::GetLadder();

    if ( distance != 0 && player->GetFacing() == ladder->GetFacing() )
        return player->GetFacing();

    if ( ladder->GetFacing() == dir )
        return dir;

    Direction oppositeDir = Util::GetOppositeDir( ladder->GetFacing() );

    if ( oppositeDir == player->GetFacing() )
        return oppositeDir;

    if ( oppositeDir != Dir_Down 
        || moving != Dir_Up )
        return Dir_None;

    // At this point, ladder faces up, and player moving up.

    dir = (Direction) moving;

    if ( World::CollidesWithTileMoving( objX, objY - 8, dir, true ) )
        return Dir_None;

    // ORIGINAL: The routine will run again. It'll finish, because now (ladder.facing = dir), 
    //           which is one of the conditions that ends this function.
    //           But, why not return dir right here?
    return MoveOnLadder( dir, distance );
}

// $01:A13E  stop object, if too close to a block
Direction Object::StopAtBlock( Direction dir )
{
    for ( int i = BufferSlot; i >= MonsterSlot1; i-- )
    {
        Object* obj = World::GetObject( i );
        if ( obj != nullptr )
        {
            IBlocksPlayer* block = (IBlocksPlayer*) obj->GetInterface( ObjItf_IBlocksPlayer );
            if ( block != nullptr )
            {
                if ( block->CheckCollision() == Collision_Blocked )
                    return Dir_None;
            }
        }
    }
    return dir;
}

void Object::ObjMove( int speed )
{
    if ( shoveDir != 0 )
    {
        ObjShove();
        return;
    }

    bool cleanUpMoving = false;
    Direction dir = Dir_None;

    if ( !IsPlayer() )
    {
        if ( IsStunned() )
            return;
        cleanUpMoving = true;
    }
    else if ( tileOffset == 0 )
    {
        cleanUpMoving = true;
    }
    else if ( moving == 0 )
    {
        // Nothing to do.
    }
    else if ( facing != Dir_None )
    {
        dir = facing;
    }

    if ( cleanUpMoving )
    {
        if ( moving != 0 )
        {
            int dirOrd = Util::GetDirectionOrd( (Direction) moving );
            dir = Util::GetOrdDirection( dirOrd );
        }
    }

    dir = (Direction) (dir & 0xF);

    // Original: [$E] := 0
    // Maybe it's only done to set up the call to FindUnblockedDir in CheckTileCollision?

    if ( IsPlayer() )
    {
        // The original game resets ~moving~ here, if player's major state is $10 or $20.
        // What we do instead in that case is to avoid calling ObjMove in Player. I think 
        // that it's clearer this way.

        dir = StopAtBlock( dir );
        dir = StopAtPersonWallUW( dir );

        if ( World::GetDoorwayDir() == Dir_None )
        {
            GameMode mode = World::GetMode();

            if (   mode == Mode_PlayCellar 
                || mode == Mode_PlayCave
                || mode == Mode_PlayShortcuts )
            {
                dir = CheckSubroom( dir );
                mode = World::GetMode();
            }

            if ( mode != Mode_PlayCellar )
            {
                if ( !World::IsOverworld() && World::GetDoorwayDir() == Dir_None )
                    dir = CheckWorldMargin( dir );
            }
        }
    }
    else
    {
        dir = CheckWorldMargin( dir );
    }

    GameMode mode = World::GetMode();

    if ( IsPlayer() 
        && !World::IsOverworld()
        && mode != Mode_PlayCellar )
    {
        dir = CheckDoorways( dir );
    }

    dir = CheckTileCollision( dir );

    if ( IsPlayer() )
    {
        dir = HandleLadder( dir );
    }

    ObjMoveDir( speed, dir );
}

void Object::ObjMoveDir( int speed, Direction dir )
{
    int align = 0x10;

    if ( IsPlayer() )
        align = 8;

    ObjMoveWhole( speed, dir, align );
}

void Object::ObjMoveWhole( int speed, Direction dir, int align )
{
    if ( dir == Dir_None )
        return;

    ObjMoveFourth( speed, dir, align );
    ObjMoveFourth( speed, dir, align );
    ObjMoveFourth( speed, dir, align );
    ObjMoveFourth( speed, dir, align );
}

void Object::ObjMoveFourth( int speed, Direction dir, int align )
{
    int frac = fraction;

    if ( dir == Dir_Down || dir == Dir_Right )
        frac += speed;
    else
        frac -= speed;

    int carry = frac >> 8;
    fraction = frac & 0xFF;

    if ( (tileOffset != align) && (tileOffset != -align) )
    {
        tileOffset += carry;

        if ( dir == Dir_Right || dir == Dir_Left )
            objX += carry;
        else
            objY += carry;
    }
}

void Object::ObjShove()
{
    if ( (shoveDir & 0x80) == 0 )
    {
        if ( shoveDistance != 0 )
        {
            MoveShoveWhole();
        }
        else
        {
            shoveDir = 0;
            shoveDistance = 0;
        }
    }
    else
    {
        shoveDir ^= 0x80;

        bool shoveHoriz = Util::IsHorizontal( (Direction) (shoveDir & 0xF) );
        bool facingHoriz = Util::IsHorizontal( facing );

        if ( (shoveHoriz != facingHoriz) && (tileOffset != 0) && !IsPlayer() )
        {
            shoveDir = 0;
            shoveDistance = 0;
        }
    }
}

void Object::MoveShoveWhole()
{
    Direction cleanDir = (Direction) (shoveDir & 0xF);

    for ( int i = 0; i < 4; i++ )
    {
        if ( tileOffset == 0 )
        {
            objX &= 0xF8;
            objY &= 0xF8;
            objY |= 5;

            if ( World::CollidesWithTileMoving( objX, objY, cleanDir, IsPlayer() ) )
            {
                shoveDir = 0;
                shoveDistance = 0;
                return;
            }
        }

        if (   Dir_None == CheckWorldMargin( cleanDir ) 
            || Dir_None == StopAtPersonWallUW( cleanDir ) )
        {
            shoveDir = 0;
            shoveDistance = 0;
            return;
        }

        int distance = Util::IsGrowingDir( cleanDir ) ? 1 : -1;

        shoveDistance--;
        tileOffset += distance;

        if ( (tileOffset & 0xF) == 0 )
            tileOffset &= 0xF;
        else if ( IsPlayer() && (tileOffset & 7) == 0 )
            tileOffset &= 7;

        if ( Util::IsHorizontal( cleanDir ) )
            objX += distance;
        else
            objY += distance;
    }
}
