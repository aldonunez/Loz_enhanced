/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "ItemObj.h"
#include "Graphics.h"
#include "PlayerItemsAnim.h"
#include "OWNpcsAnim.h"
#include "World.h"
#include "Player.h"
#include "ObjType.h"
#include "Profile.h"
#include "Monsters.h"
#include "Sound.h"
#include "SoundId.h"
#include "TextBox.h"


struct ItemGraphics
{
    uint8_t AnimId;
    uint8_t PaletteAttrs;

    static const uint8_t FlashPalAttr = 0x80;

    uint8_t GetPalette() const  { return PaletteAttrs & 0x7; }
    bool HasFlashAttr() const   { return (PaletteAttrs & FlashPalAttr) != 0 ? true : false; }
};

const int PlayerPal = 4;
const int BluePal   = 5;
const int RedPal    = 6;
const int OtherPal  = 7;

static const uint8_t equippedItemIds[] = 
{
    0,          // Sword
    0xFF,       // Bombs
    7,          // Arrow
    9,          // Bow
    5,          // Candle
    4,          // Recorder
    3,          // Food
    30,         // Potion
    15,         // Rod
    11,         // Raft
    16,         // Book
    17,         // Ring
    12,         // Ladder
    10,         // MagicKey
    19,         // Bracelet
    20,         // Letter
    Item_None,  // Compass
    Item_None,  // Map
    Item_None,  // Compass9
    Item_None,  // Map9
    Item_None,  // Clock
    Item_None,  // Rupees
    Item_None,  // Keys
    Item_None,  // HeartContainers
    Item_None,  // PartialHeart
    Item_None,  // TriforcePieces
    13,         // PowerTriforce
    28,         // Boomerang
};



static bool CrossesProjectileBoundary( int x, int y, Direction dir )
{
    if ( (dir & Dir_Right) != 0
        && x > 0xEB )
        return true;

    if ( (dir & Dir_Left) != 0 
        && x < 6 )
        return true;

    if ( (dir & Dir_Down) != 0 
        && y > 0xDE )
        return true;

    if ( (dir & Dir_Up) != 0 
        && y < 0x3F )
        return true;

    return false;
}

static bool IsPlayerWeapon()
{
    return World::GetCurrentObjectSlot() > BufferSlot;
}


//----------------------------------------------------------------------------
//  Ladder
//----------------------------------------------------------------------------

Ladder::Ladder()
    :   Object( Obj_Ladder ),
        origDir( Dir_Down ),
        state( 1 )
{
    facing = World::GetPlayer()->GetFacing();
    decoration = 0;

    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Ladder );
}

int  Ladder::GetState()
{
    return state;
}

void Ladder::SetState( int state )
{
    this->state = state;
}

void Ladder::Update()
{
}

void Ladder::Draw()
{
    image.Draw( Sheet_PlayerAndItems, objX, objY, PlayerPalette );
}


//----------------------------------------------------------------------------
//  BlockObjBase
//----------------------------------------------------------------------------

const float BlockSpeed = 0.5;

struct BlockSpec
{
    uint8_t blockTile;
    uint8_t floorTile1;
    uint8_t floorTile2;
    uint8_t timerLimit;
    bool    allowHorizontal;
};

BlockSpec rockSpec = 
{
    Tile_Rock,
    Tile_Ground,
    Tile_Ground,
    1,
    false
};

BlockSpec headstoneSpec = 
{
    Tile_Headstone,
    Tile_Ground,
    Tile_Stairs,
    1,
    false
};

BlockSpec blockSpec = 
{
    Tile_Block,
    Tile_Tile,
    Tile_Tile,
    17,
    true
};

BlockObjBase::BlockObjBase( ObjType type, const BlockSpec* spec )
    :   Object( type ),
        timer( 0 ),
        spec( spec ),
        curUpdate( &BlockObjBase::UpdateIdle )
{
    decoration = 0;
}

void BlockObjBase::SetX( int x )
{
    objX = x;
}

void BlockObjBase::SetY( int y )
{
    objY = y;
}

CollisionResponse BlockObjBase::CheckCollision()
{
    if ( curUpdate != &BlockObjBase::UpdateMoving )
        return Collision_Unknown;

    Player* player = World::GetPlayer();
    if ( player == nullptr )
        return Collision_Unknown;

    int playerX = player->GetX();
    int playerY = player->GetY() + 3;

    if (   abs( playerX - objX ) < World::TileWidth
        && abs( playerY - objY ) < World::TileHeight )
        return Collision_Blocked;

    return Collision_Unknown;
}

void BlockObjBase::Update()
{
    (this->*curUpdate)();
}

void BlockObjBase::UpdateIdle()
{
    if ( GetType() == Obj_Rock )
    {
        if ( World::GetItem( ItemSlot_Bracelet ) == 0 )
            return;
    }
    else if ( GetType() == Obj_Block )
    {
        if ( World::HasLivingObjects() )
            return;
    }

    Player* player = World::GetPlayer();
    if ( player == nullptr )
        return;

    Bounds      bounds = player->GetBounds();
    Direction   dir = player->GetMoving();
    bool        pushed = false;

    if ( !spec->allowHorizontal && (dir == Dir_Left || dir == Dir_Right) )
        dir = Dir_None;

    if ( dir != Dir_None )
    {
        uint playerX = player->GetX();
        uint playerY = player->GetY() + 3;

        if ( Util::IsVertical( dir ) )
        {
            if ( objX == playerX
                && abs( objY - playerY ) <= World::TileHeight )
                pushed = true;
        }
        else
        {
            if ( objY == playerY 
                && abs( objX - playerX ) <= World::TileWidth )
                pushed = true;
        }
    }

    if ( pushed )
    {
        timer++;
        if ( timer == spec->timerLimit )
        {
            switch ( dir )
            {
            case Dir_Right: targetPos = objX + World::TileWidth;   break;
            case Dir_Left:  targetPos = objX - World::TileWidth;   break;
            case Dir_Down:  targetPos = objY + World::TileHeight;  break;
            case Dir_Up:    targetPos = objY - World::TileHeight;  break;
            }
            World::SetTile( objX, objY, spec->floorTile1 );
            facing = dir;
            origX = objX;
            origY = objY;
            curUpdate = &BlockObjBase::UpdateMoving;
        }
    }
    else
        timer = 0;
}

void BlockObjBase::UpdateMoving()
{
    bool done = false;

    ObjMoveDir( 0x20, facing );

    if ( facing == Dir_Left || facing == Dir_Right )
    {
        if ( objX == targetPos )
            done = true;
    }
    else
    {
        if ( objY == targetPos )
            done = true;
    }

    if ( done )
    {
        World::OnPushedBlock();
        World::SetTile( objX, objY, spec->blockTile );
        World::SetTile( origX, origY, spec->floorTile2 );
        isDeleted = true;
    }
}

void BlockObjBase::Draw()
{
    if ( curUpdate == &BlockObjBase::UpdateMoving )
    {
        int srcX = (spec->blockTile & 0x0F) * World::TileWidth;
        int srcY = ((spec->blockTile & 0xF0) >> 4) * World::TileHeight;

        Graphics::DrawTile( 
            Sheet_Background,
            srcX,
            srcY,
            World::TileWidth,
            World::TileHeight,
            objX,
            objY,
            World::GetInnerPalette(),
            0 );
    }
}

void* BlockObjBase::GetInterface( ObjInterfaces iface )
{
    if ( iface == ObjItf_IBlocksPlayer )
        return (IBlocksPlayer*) this;
    return nullptr;
}

Rock::Rock()
    :   BlockObjBase( Obj_Rock, &rockSpec )
{
}

Headstone::Headstone()
    :   BlockObjBase( Obj_Headstone, &headstoneSpec )
{
}

Block::Block()
    :   BlockObjBase( Obj_Block, &blockSpec )
{
}


//----------------------------------------------------------------------------
//  Fire
//----------------------------------------------------------------------------

Fire::Fire()
    :   Object( Obj_Fire ),
        state( Moving )
{
    animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Fire );
    animator.time = 0;
    animator.durationFrames = 8;

    decoration = 0;
}

void Fire::SetMoving( Direction dir )
{
    facing = dir;
}

Fire::State Fire::GetLifetimeState()
{
    return state;
}

void Fire::SetLifetimeState( State state )
{
    this->state = state;

    if ( state == Standing )
    {
        World::BeginFadeIn();
    }
}

void Fire::SetX( int x )
{
    objX = x;
}

void Fire::SetY( int y )
{
    objY = y;
}

Point Fire::GetMiddle()
{
    Point p = { objX + 8, objY + 8 };
    return p;
}

void Fire::Update()
{
    if ( state == Moving )
    {
        int origOffset = tileOffset;
        tileOffset = 0;
        ObjMoveDir( 0x20, facing );
        tileOffset += origOffset;

        if ( abs( tileOffset ) == 0x10 )
        {
            objTimer = 0x3F;
            state = Standing;
            World::BeginFadeIn();
        }
    }
    else
    {
        if ( objTimer == 0 )
        {
            isDeleted = true;
            return;
        }
    }

    animator.Advance();

    CheckCollisionWithPlayer();
}

// F8D9
void Fire::CheckCollisionWithPlayer()
{
    Player* player = World::GetPlayer();

    if ( player->GetInvincibilityTimer() == 0 )
    {
        Point   objCenter = GetMiddle();
        Point   playerCenter = player->GetMiddle();
        Point   box = { 0xE, 0xE };
        Point   distance;

        if ( !DoObjectsCollide( objCenter, playerCenter, box, distance ) )
            return;

        CollisionContext context = { 0 };
        context.Distance = distance;

        Shove( context );
        player->BeHarmed( this, objCenter, 0x0080 );
    }
}

void Fire::Draw()
{
    animator.Draw( Sheet_PlayerAndItems, objX, objY, RedFgPalette );
}


//----------------------------------------------------------------------------
//  Tree
//----------------------------------------------------------------------------

Tree::Tree()
    :   Object( Obj_Tree )
{
    decoration = 0;
}

void Tree::SetX( int x )
{
    this->x = x;
}

void Tree::SetY( int y )
{
    this->y = y;
}

int Tree::GetX()
{
    return x;
}

int Tree::GetY()
{
    return y;
}

void Tree::Update()
{
    for ( int i = FirstFireSlot; i < LastFireSlot; i++ )
    {
        Fire* fire = (Fire*) World::GetObject( i );

        if ( fire == nullptr 
            || fire->GetType() != Obj_Fire 
            || fire->IsDeleted()
            || fire->GetLifetimeState() != Fire::Standing 
            || fire->GetObjectTimer() != 2 )
            continue;

        if ( abs( (int) fire->GetX() - x ) >= 16
            || abs( (int) fire->GetY() - y ) >= 16 )
            continue;

        World::SetTile( x, y, Tile_Stairs );
        World::Get()->TakeSecret();
        Sound::PlayEffect( SEffect_secret );
        isDeleted = true;
        break;
    }
}

void Tree::Draw()
{
}


//----------------------------------------------------------------------------
//  Bomb
//----------------------------------------------------------------------------

const int Clouds = 4;
const int CloudFrames = 2;

static Point cloudPositions[CloudFrames][Clouds] = 
{
    {
        { 0, 0 },
        { -13, 0 },
        { 7, -13 },
        { -7, 14 }
    },

    {
        { 0, 0 },
        { 13, 0 },
        { -7, -13 },
        { 7, 14 }
    }
};

Bomb::Bomb()
    :   Object( Obj_Bomb ),
        state( Initing )
{
    facing = World::GetPlayer()->GetFacing();
    decoration = 0;
    animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_BombItem );
    animator.time = 0;
    animator.durationFrames = 1;
}

Bomb::State Bomb::GetLifetimeState()
{
    return state;
}

void Bomb::SetLifetimeState( Bomb::State state )
{
    this->state = state;
}

void Bomb::SetX( int x )
{
    objX = x;
}

void Bomb::SetY( int y )
{
    objY = y;
}

void Bomb::Update()
{
    static const uint8_t times[] = { 0x30, 0x18, 0xC, 0 };

    if ( objTimer == 0 )
    {
        objTimer = times[state];
        state = (State) (state + 1);

        if ( state == Blasting )
        {
            animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Cloud );
            animator.time = 0;
            animator.durationFrames = animator.anim->length;
            Sound::PlayEffect( SEffect_bomb );
        }
        else if ( state == Fading )
        {
            animator.AdvanceFrame();
        }
        else if ( state > Fading )
        {
            isDeleted = true;
            objTimer = 0;
        }
    }
}

void Bomb::Draw()
{
    if ( state == Ticking )
    {
        int offset = (16 - animator.anim->width) / 2;
        animator.Draw( Sheet_PlayerAndItems, objX + offset, objY, BlueFgPalette );
    }
    else
    {
        // TODO: flash
        Point* positions = cloudPositions[objTimer % CloudFrames];

        for ( int i = 0; i < Clouds; i++ )
        {
            animator.Draw( 
                Sheet_PlayerAndItems, objX + positions[i].X, objY + positions[i].Y, BlueFgPalette );
        }
    }
}


//----------------------------------------------------------------------------
//  RockWall
//----------------------------------------------------------------------------

RockWall::RockWall()
    :   Object( Obj_RockWall )
{
    decoration = 0;
}

void RockWall::SetX( int x )
{
    this->x = x;
}

void RockWall::SetY( int y )
{
    this->y = y;
}

int RockWall::GetX()
{
    return x;
}

int RockWall::GetY()
{
    return y;
}

void RockWall::Update()
{
    for ( int i = FirstBombSlot; i < LastBombSlot; i++ )
    {
        Bomb* bomb = (Bomb*) World::GetObject( i );

        if ( bomb == nullptr 
            || bomb->GetType() != Obj_Bomb 
            || bomb->IsDeleted()
            || bomb->GetLifetimeState() != Bomb::Blasting 
            )
            continue;

        if ( abs( (int) bomb->GetX() - x ) >= 16
            || abs( (int) bomb->GetY() - y ) >= 16 )
            continue;

        World::SetTile( x, y, Tile_Cave );
        World::Get()->TakeSecret();
        Sound::PlayEffect( SEffect_secret );
        isDeleted = true;
        return;
    }
}

void RockWall::Draw()
{
}


//----------------------------------------------------------------------------
//  PlayerSword
//----------------------------------------------------------------------------

const int   DirCount = 4;
const int   SwordStates = 5;
const int   LastSwordState = SwordStates - 1;

static Point swordOffsets[SwordStates][DirCount] = 
{
    {
        { -8, -11 },
        {  0, -11 },
        {  1, -14 },
        { -1,  -9 }
    },

    {
        {  11,   3 },
        { -11,   3 },
        {   1,  13 },
        {  -1, -10 }
    },

    {
        {  7,   3 },
        { -7,   3 },
        {  1,   9 },
        { -1,  -9 }
    },

    {
        {  3,  3 },
        { -3,  3 },
        {  1,  5 },
        { -1, -1 }
    },
};

static uint8_t  swordAnimMap[DirCount] = 
{
    Anim_PI_Sword_Right,
    Anim_PI_Sword_Left,
    Anim_PI_Sword_Down,
    Anim_PI_Sword_Up
};

static uint8_t  rodAnimMap[DirCount] = 
{
    Anim_PI_Wand_Right,
    Anim_PI_Wand_Left,
    Anim_PI_Wand_Down,
    Anim_PI_Wand_Up,
};

static uint8_t  swordStateDurations[SwordStates] = 
{
    5,
    8,
    1,
    1,
    1
};


PlayerSword::PlayerSword( ObjType type )
    :   Object( type ),
        state( 0 ),
        timer( 0 )
{
    Put();
    timer = swordStateDurations[state];
    decoration = 0;
}

int PlayerSword::GetState()
{
    return state;
}

void PlayerSword::Put()
{
    Player*   player = World::GetPlayer();
    Direction facingDir = player->GetFacing();
    objX = player->GetX();
    objY = player->GetY();

    int dirOrd = Util::GetDirectionOrd( facingDir );
    Point& offset = swordOffsets[state][dirOrd];
    objX += offset.X;
    objY += offset.Y;
    facing = facingDir;

    const uint8_t* animMap;

    if ( GetType() == Obj_Rod )
        animMap = rodAnimMap;
    else
        animMap = swordAnimMap;

    int animIndex = animMap[dirOrd];
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, animIndex );
}

void PlayerSword::TryMakeWave()
{
    if ( state == 2 )
    {
        bool makeWave = false;
        Object* wave = World::GetObject( PlayerSwordShotSlot );

        if ( GetType() == Obj_Rod )
        {
            if ( wave == nullptr || wave->GetType() != Obj_MagicWave )
            {
                makeWave = true;
                Sound::PlayEffect( SEffect_magic_wave );
            }
        }
        else
        {
            if ( wave == nullptr )
            {
                // The original game skips checking hearts, and shoots, if [$529] is set.
                // But, I haven't found any code that sets it.

                Profile&     profile = World::GetProfile();
                unsigned int neededHeartsValue = (profile.Items[ItemSlot_HeartContainers] << 8) - 0x80;

                if ( profile.Hearts >= neededHeartsValue )
                {
                    makeWave = true;
                    Sound::PlayEffect( SEffect_sword_wave );
                }
            }
        }

        if ( makeWave )
            MakeWave();
    }
}

void PlayerSword::MakeWave()
{
    Player* player = World::GetPlayer();
    int x = player->GetX();
    int y = player->GetY();
    Direction dir = player->GetFacing();

    Util::MoveSimple( x, y, dir, 0x10 );

    if ( Util::IsVertical( dir )
        || (x >= 0x14 && x < 0xEC) )
    {
        Object* wave = nullptr;

        if ( GetType() == Obj_Rod )
            wave = MakeProjectile( Obj_MagicWave, x, y, dir, PlayerSwordShotSlot );
        else
            wave = MakeProjectile( Obj_PlayerSwordShot, x, y, dir, PlayerSwordShotSlot );

        World::SetObject( PlayerSwordShotSlot, wave );
        wave->SetTileOffset( player->GetTileOffset() );
    }
}

void PlayerSword::Update()
{
    timer--;

    if ( timer == 0 )
    {
        if ( state == LastSwordState )
        {
            isDeleted = true;
            return;
        }
        state++;
        timer = swordStateDurations[state];
        // The original game does this: player.animTimer := timer
        // But, we do it differently. The player handles all of its animation.
    }

    if ( state < LastSwordState )
    {
        Put();
        TryMakeWave();
    }
}

void PlayerSword::Draw()
{
    if ( state > 0 && state < LastSwordState )
    {
        int weaponValue = World::GetItem( ItemSlot_Sword );
        int palette = (GetType() == Obj_Rod) ? BlueFgPalette : (PlayerPalette + weaponValue - 1);
        int xOffset = (16 - image.anim->width) / 2;
        image.Draw( Sheet_PlayerAndItems, objX + xOffset, objY, palette );
    }
}


//----------------------------------------------------------------------------
//  Shot
//----------------------------------------------------------------------------

Shot::Shot( ObjType type )
    :   Object( type ),
        state( Flying ),
        bounceDir( Dir_None )
{
    if ( !IsPlayerWeapon() )
        World::SetActiveShots( World::GetActiveShots() + 1 );
}

Shot::~Shot()
{
    if ( !IsPlayerWeapon() )
        World::SetActiveShots( World::GetActiveShots() - 1 );
}

void* Shot::GetInterface( ObjInterfaces iface )
{
    if ( iface == ObjItf_IShot )
        return (IShot*) this;
    return nullptr;
}

bool Shot::IsInShotStartState()
{
    return state == Flying;
}

Shot::State Shot::GetLifetimeState()
{
    return state;
}

void Shot::Move( int speed )
{
    ObjMoveDir( speed, facing );

    if ( (tileOffset & 7) == 0 )
        tileOffset = 0;
}

void Shot::CheckPlayer()
{
    if ( !IsPlayerWeapon() )
    {
        PlayerCollision collision = CheckPlayerCollision();
        if ( collision.Collides )
        {
            DeleteShot();
        }
        else if ( collision.ShotCollides )
        {
            tileOffset = 0;
            state = Bounce;
            bounceDir = World::GetPlayer()->GetFacing();
        }
    }
}

void Shot::UpdateBounce()
{
    static const int xSpeeds[] = {  2, -2, -1,  1 };
    static const int ySpeeds[] = { -1, -1,  2, -2 };

    int dirOrd = Util::GetDirectionOrd( bounceDir );

    objX += xSpeeds[dirOrd];
    objY += ySpeeds[dirOrd];
    tileOffset += 2;

    if ( tileOffset >= 0x20 )
        DeleteShot();
}

void Shot::DeleteShot()
{
    isDeleted = true;
}


//----------------------------------------------------------------------------
//  PlayerSwordShot
//----------------------------------------------------------------------------

PlayerSwordShot::PlayerSwordShot( int x, int y, Direction moving )
    :   Shot( Obj_PlayerSwordShot ),
        distance( 0 )
{
    objX = x;
    objY = y;
    facing = moving;
    decoration = 0;

    int dirOrd = Util::GetDirectionOrd( facing );
    int animIndex = swordAnimMap[dirOrd];
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, animIndex );
}

void PlayerSwordShot::Update()
{
    switch ( state )
    {
    case Flying:    UpdateFlying(); break;
    case Spreading: UpdateSpreading(); break;
    case Bounce:    UpdateBounce(); break;
    }
}

void PlayerSwordShot::UpdateFlying()
{
    if ( Dir_None == CheckWorldMargin( facing ) )
    {
        if ( IsPlayerWeapon() )
            SpreadOut();
        else
            DeleteShot();
        return;
    }

    Move( 0xC0 );
    CheckPlayer();
}

void PlayerSwordShot::UpdateSpreading()
{
    if ( distance == 21 )
    {
        // The original game still drew in this frame, but we won't.
        isDeleted = true;
        return;
    }
    distance++;
}

void PlayerSwordShot::Draw()
{
    int palOffset = GetFrameCounter() % ForegroundPalCount;
    int palette = PlayerPalette + palOffset;
    int yOffset = Util::IsHorizontal( facing ) ? 3 : 0;

    if ( state == Flying )
    {
        int xOffset = (16 - image.anim->width) / 2;
        image.Draw( Sheet_PlayerAndItems, objX + xOffset, objY + yOffset, palette );
    }
    else
    {
        if ( distance != 0 )
        {
            int xOffset = 4;
            int d       = distance - 1;
            int left    = objX - 2 - d + xOffset;
            int right   = objX + 2 + d + xOffset;
            int top     = objY - 2 - d + yOffset;
            int bottom  = objY + 2 + d + yOffset;

            image.Draw( Sheet_PlayerAndItems, left, top, palette, 0 );
            image.Draw( Sheet_PlayerAndItems, right, top, palette, ALLEGRO_FLIP_HORIZONTAL );
            image.Draw( Sheet_PlayerAndItems, left, bottom, palette, ALLEGRO_FLIP_VERTICAL );
            image.Draw( Sheet_PlayerAndItems, right, bottom, palette, 
                ALLEGRO_FLIP_HORIZONTAL | ALLEGRO_FLIP_VERTICAL );
        }
    }
}

void PlayerSwordShot::SpreadOut()
{
    state = Spreading;
    distance = 0;
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Slash );
}


//----------------------------------------------------------------------------
//  FlyingRock
//----------------------------------------------------------------------------

FlyingRock::FlyingRock( int x, int y, Direction moving )
    :   Shot( Obj_FlyingRock )
{
    objX = x;
    objY = y;
    facing = moving;
    decoration = 0;

    image.anim = Graphics::GetAnimation( Sheet_Npcs, Anim_OW_FlyingRock );
}

void FlyingRock::Update()
{
    switch ( state )
    {
    case Flying:    UpdateFlying(); break;
    case Bounce:    UpdateBounce(); break;
    }
}

void FlyingRock::UpdateFlying()
{
    if ( objTimer == 0 )
    {
        if ( World::CollidesWithTileMoving( objX, objY, facing, false ) )
        {
            DeleteShot();
            return;
        }
    }

    if ( Dir_None == CheckWorldMargin( facing ) )
    {
        DeleteShot();
        return;
    }

    Move( 0xC0 );
    CheckPlayer();
}

void FlyingRock::Draw()
{
    int palette = PlayerPalette;
    int xOffset = (16 - image.anim->width) / 2;
    image.Draw( Sheet_Npcs, objX + xOffset, objY, palette );
}


//----------------------------------------------------------------------------
//  Fireball
//----------------------------------------------------------------------------

Direction sector16Dirs[16] = 
{
    Dir_Right,
    Dir_Right | Dir_Down,
    Dir_Right | Dir_Down,
    Dir_Right | Dir_Down,
    Dir_Down,
    Dir_Down | Dir_Left,
    Dir_Down | Dir_Left,
    Dir_Down | Dir_Left,
    Dir_Left,
    Dir_Left | Dir_Up,
    Dir_Left | Dir_Up,
    Dir_Left | Dir_Up,
    Dir_Up,
    Dir_Up | Dir_Right,
    Dir_Up | Dir_Right,
    Dir_Up | Dir_Right,
};


Fireball::Fireball( ObjType type, int x, int y, float speed )
    :   Object( type ),
        state( 0 ),
        x( x ),
        y( y )
{
    objX = x;
    objY = y;
    decoration = 0;
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Fireball );

    Player* player = World::GetPlayer();
    float   xDist = (int) player->GetX() - x;
    float   yDist = (int) player->GetY() - y;

    Util::Rotate( Util::PI_OVER_16, xDist, yDist );
    int         sector  = Util::GetSector16( xDist, yDist );
    float       angle   = Util::PI_OVER_8 * sector;

    facing = sector16Dirs[sector];

    speedX = cos( angle ) * speed;
    speedY = sin( angle ) * speed;
}

void Fireball::SetY( int y )
{
    this->y = y;
    objY = y;
}

void Fireball::Update()
{
    if ( state == 0 )
    {
        objTimer = 0x10;
        state = 1;
        return;
    }

    if ( objTimer == 0 )
    {
        if ( Dir_None == CheckWorldMargin( facing ) )
        {
            isDeleted = true;
            return;
        }

        x += speedX;
        y += speedY;
        objX = x;
        objY = y;
    }

    PlayerCollision collision = CheckPlayerCollision();
    if ( collision.Collides || collision.ShotCollides )
    {
        isDeleted = true;
        return;
    }
}

void Fireball::Draw()
{
    int palOffset = GetFrameCounter() % ForegroundPalCount;
    int palette = PlayerPalette + palOffset;

    image.Draw( Sheet_PlayerAndItems, objX, objY, palette );
}


//----------------------------------------------------------------------------
//  Boomerang
//----------------------------------------------------------------------------

Boomerang::Boomerang( int x, int y, Direction moving, int distance, float speed, int ownerSlot )
    :   Object( Obj_Boomerang ),
        startX( x ),
        startY( y ),
        distanceTarget( distance ),
        ownerSlot( ownerSlot ),
        x( x ),
        y( y ),
        leaveSpeed( speed ),
        state( 1 ),
        animTimer( 3 )
{
    objX = x;
    objY = y;
    facing = moving;
    decoration = 0;

    animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Boomerang );
    animator.time = 0;
    animator.durationFrames = animator.anim->length * 2;

    if ( !IsPlayerWeapon() )
        World::SetActiveShots( World::GetActiveShots() + 1 );
}

Boomerang::~Boomerang()
{
    if ( !IsPlayerWeapon() )
        World::SetActiveShots( World::GetActiveShots() - 1 );
}

void* Boomerang::GetInterface( ObjInterfaces iface )
{
    if ( iface == ObjItf_IShot )
        return (IShot*) this;
    return nullptr;
}

bool Boomerang::IsInShotStartState()
{
    return state == 1;
}

void Boomerang::SetState( int state )
{
    this->state = state;
}

void Boomerang::Update()
{
    switch ( state )
    {
    case 1: UpdateLeaveFast(); break;
    case 2: UpdateSpark(); break;
    case 3: UpdateLeaveSlow(); break;
    case 4: 
    case 5: UpdateReturn(); break;
    }
}

void Boomerang::UpdateLeaveFast()
{
    Util::MoveSimple8( x, y, facing, leaveSpeed );
    objX = x;
    objY = y;

    if ( Dir_None == CheckWorldMargin( facing ) )
    {
        state = 2;
        animTimer = 3;
        CheckCollision();
    }
    else
    {
        if (   abs( startX - objX ) < distanceTarget 
            && abs( startY - objY ) < distanceTarget )
        {
            AdvanceAnimAndCheckCollision();
        }
        else
        {
            distanceTarget = 0x10;
            state = 3;
            animTimer = 3;
            animator.time = 0;
            CheckCollision();
        }
    }
}

void Boomerang::UpdateLeaveSlow()
{
    bool gotoNextState = true;

    if ( (facing & Dir_Left) == 0 || x >= 2 )
    {
        if ( (moving & Dir_Left) != 0 )
            facing = Dir_Left;
        else if ( (moving & Dir_Right) != 0 )
            facing = Dir_Right;

        Util::MoveSimple8( x, y, facing, 1 );
        objX = x;
        objY = y;

        distanceTarget--;
        if ( distanceTarget != 0 )
            gotoNextState = false;
    }

    if ( gotoNextState )
    {
        distanceTarget = 0x20;
        state = 4;
        animator.time = 0;
    }

    AdvanceAnimAndCheckCollision();
}

void Boomerang::UpdateReturn()
{
    Object* owner = World::GetObject( ownerSlot );
    if ( owner == nullptr || owner->GetDecoration() != 0 )
    {
        isDeleted = true;
        return;
    }

    IThrower* thrower = (IThrower*) owner->GetInterface( ObjItf_IThrower );
    if ( thrower == nullptr )
    {
        isDeleted = true;
        return;
    }

    int yDist = owner->GetY() - (int) floor( y );
    int xDist = owner->GetX() - (int) floor( x );

    if ( abs( xDist ) < 9 && abs( yDist ) < 9 )
    {
        thrower->Catch();
        isDeleted = true;
    }
    else
    {
        float angle = atan2( (float) yDist, (float) xDist );
        float speed = 2;

        if ( state == 4 )
        {
            speed = 1;
            distanceTarget--;
            if ( distanceTarget == 0 )
            {
                state = 5;
                animator.time = 0;
            }
        }

        float ySpeed, xSpeed;
        Util::PolarToCart( angle, speed, xSpeed, ySpeed );

        x += xSpeed;
        y += ySpeed;
        objX = x;
        objY = y;

        AdvanceAnimAndCheckCollision();
    }
}

void Boomerang::UpdateSpark()
{
    animTimer--;
    if ( animTimer == 0 )
    {
        state = 5;
        animTimer = 3;
        animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Boomerang );
        animator.time = 0;
    }
    else
    {
        animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Spark );
        animator.time = 0;
    }
}

void Boomerang::AdvanceAnimAndCheckCollision()
{
    animTimer--;
    if ( animTimer == 0 )
    {
        // The original game sets animTimer to 2.
        // But the sound from the NSF doesn't sound right at that speed.
        animTimer = 11;
        if ( ownerSlot == PlayerSlot )
            Sound::PlayEffect( SEffect_boomerang );
    }

    animator.Advance();

    CheckCollision();
}

void Boomerang::CheckCollision()
{
    if ( !IsPlayerWeapon() )
    {
        PlayerCollision collision = CheckPlayerCollision();
        if ( collision.ShotCollides )
        {
            state = 2;
            animTimer = 3;
        }
    }
}

void Boomerang::Draw()
{
    int itemValue = World::GetItem( ItemSlot_Boomerang );
    if ( itemValue == 0 )
        itemValue = 1;
    int pal = (state == 2) ? RedFgPalette : (PlayerPalette + itemValue - 1);
    int xOffset = (16 - animator.anim->width) / 2;
    animator.Draw( Sheet_PlayerAndItems, x + xOffset, y, pal );
}


//----------------------------------------------------------------------------
// MagicWave
//----------------------------------------------------------------------------

static const uint8_t waveAnimMap[DirCount] = 
{
    Anim_PI_Wave_Right,
    Anim_PI_Wave_Left,
    Anim_PI_Wave_Down,
    Anim_PI_Wave_Up,
};


MagicWave::MagicWave( ObjType type, int x, int y, Direction moving )
    :   Shot( type )
{
    objX = x;
    objY = y;
    facing = moving;
    decoration = 0;

    int dirOrd = Util::GetDirectionOrd( moving );
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, waveAnimMap[dirOrd] );
}

void MagicWave::Update()
{
    switch ( state )
    {
    case Flying:    UpdateFlying(); break;
    case Bounce:    UpdateBounce(); break;
    }
}

void MagicWave::UpdateFlying()
{
    if ( Dir_None == CheckWorldMargin( facing ) )
    {
        if ( IsPlayerWeapon() && !World::Get()->IsOverworld() )
            AddFire();

        DeleteShot();
        return;
    }

    Move( 0xA0 );
    CheckPlayer();
}

void MagicWave::Draw()
{
    int pal = 4 + GetFrameCounter() % 4;
    image.Draw( Sheet_PlayerAndItems, objX, objY, pal );
}

void MagicWave::AddFire()
{
    if ( World::GetItem( ItemSlot_Book ) != 0 )
    {
        int fireSlot = World::FindEmptyFireSlot();
        if ( fireSlot >= 0 )
        {
            Fire* fire = new Fire();
            fire->SetX( objX );
            fire->SetY( objY );
            fire->SetMoving( facing );
            fire->SetLifetimeState( Fire::Standing );
            fire->SetObjectTimer( 0x4F );
            World::SetObject( fireSlot, fire );
        }
    }
}


//----------------------------------------------------------------------------
// Arrow
//----------------------------------------------------------------------------

static const uint8_t arrowAnimMap[DirCount] = 
{
    Anim_PI_Arrow_Right,
    Anim_PI_Arrow_Left,
    Anim_PI_Arrow_Down,
    Anim_PI_Arrow_Up,
};


Arrow::Arrow( int x, int y, Direction moving )
    :   Shot( Obj_Arrow ),
        timer( 0 )
{
    objX = x;
    objY = y;
    facing = moving;
    decoration = 0;

    int dirOrd = Util::GetDirectionOrd( moving );
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, arrowAnimMap[dirOrd] );
}

void Arrow::SetSpark( int frames )
{
    state = Spark;
    timer = frames;
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Spark );
}

void Arrow::Update()
{
    switch ( state )
    {
    case Flying:    UpdateArrow();  break;
    case Spark:     UpdateSpark();  break;
    case Bounce:    UpdateBounce(); break;
    }
}

void Arrow::UpdateArrow()
{
    if ( objTimer != 0 )
    {
        // The original game seems to do something if the owner is gone, but I don't see any effect.
        objTimer = 0;
        return;
    }

    if ( Dir_None == CheckWorldMargin( facing ) )
    {
        SetSpark();
    }
    else
    {
        int speed = IsPlayerWeapon() ? 0xC0 : 0x80;
        Move( speed );
        CheckPlayer();
    }
}

void Arrow::UpdateSpark()
{
    timer--;
    if ( timer == 0 )
        DeleteShot();
}

void Arrow::Draw()
{
    int pal = BlueFgPalette;

    if ( state != Spark )
    {
        if ( IsPlayerWeapon() )
        {
            int itemValue = World::GetItem( ItemSlot_Arrow );
            pal = PlayerPalette + itemValue - 1;
        }
        else
        {
            pal = RedFgPalette;
        }
    }

    static const int yOffsets[] = { 3, 3, 0, 0 };

    int dirOrd = Util::GetDirectionOrd( facing );
    int yOffset = yOffsets[dirOrd];

    int x = objX;
    int y = objY + yOffset;

    if ( state == Spark && Util::IsHorizontal( facing ) )
        x += 4;

    image.Draw( Sheet_PlayerAndItems, x, y, pal );
}


//----------------------------------------------------------------------------
//  Person
//----------------------------------------------------------------------------

const int ItemY  = 0x98;
const int PriceY = 0xB0;
static const int itemXs[]  = { 0x58, 0x78, 0x98 };
static const int priceXs[] = { 0x48, 0x68, 0x88 };

static const ItemGraphics sPersonGraphics[] = 
{
    { Anim_PI_OldMan,       RedPal },
    { Anim_PI_OldWoman,     RedPal },
    { Anim_PI_Merchant,     PlayerPal },
    { Anim_PI_Moblin,       RedPal },
};


uint8_t* NumberToStringR( uint8_t number, NumberSign sign, uint8_t* charBuf, int bufLen )
{
    assert( bufLen >= 3 );
    assert( sign == NumberSign_None || bufLen >= 4 );

    uint8_t     n = number;
    uint8_t*    pChar = charBuf + bufLen - 1;

    while ( true )
    {
        int digit = n % 10;
        *pChar = digit;
        pChar--;
        n /= 10;
        if ( n == 0 )
            break;
    }

    if ( sign != NumberSign_None && number != 0 )
    {
        if ( sign == NumberSign_Negative )
            *pChar = Char_Minus;
        else
            *pChar = Char_Plus;
        pChar--;
    }

    uint8_t* strLeft = pChar + 1;

    for ( ; pChar >= charBuf; pChar-- )
    {
        *pChar = Char_Space;
    }

    return strLeft;
}

Person::Person( ObjType type, int x, int y, const CaveSpec* spec )
    :   Object( type ),
        state( Idle ),
        spec( *spec ),
        textBox( nullptr ),
        chosenIndex( 0 ),
        showNumbers( false )
{
    objX = x;
    objY = y;
    hp = 0;

    // This isn't used anymore. The effect is implemented a different way.
    World::SetPersonWallY( 0x8D );

    if ( !World::IsOverworld() )
        Sound::PlayEffect( SEffect_item );

    int stringId = spec->GetStringId();

    if ( stringId == String_DoorRepair && World::Get()->GotItem() )
    {
        isDeleted = true;
        return;
    }

    if ( stringId == String_MoneyOrLife && World::Get()->GotItem() )
    {
        World::Get()->OpenShutters();
        World::SetPersonWallY( 0 );
        isDeleted = true;
        return;
    }

    if ( type == Obj_Grumble && World::Get()->GotItem() )
    {
        World::SetPersonWallY( 0 );
        isDeleted = true;
        return;
    }

    if ( stringId == String_EnterLevel9 )
    {
        if ( World::GetItem( ItemSlot_TriforcePieces ) == 0xFF )
        {
            World::Get()->OpenShutters();
            World::SetPersonWallY( 0 );
            isDeleted = true;
            return;
        }
    }

    if ( spec->GetPickUp() && !spec->GetShowPrices() )
    {
        if ( World::Get()->GotItem() )
        {
            isDeleted = true;
            return;
        }
    }

    int animIndex = spec->DwellerType - Obj_OldMan;
    int animId = sPersonGraphics[animIndex].AnimId;
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, animId );

    textBox = new TextBox( World::GetString( stringId ) );

    memset( priceStrs, Char_Space, sizeof priceStrs );

    if ( this->spec.GetShowPrices() || this->spec.GetSpecial() )
    {
        NumberSign sign = this->spec.GetShowNegative() ? NumberSign_Negative : NumberSign_None;

        for ( int i = 0; i < 3; i++ )
        {
            NumberToStringR( this->spec.Prices[i], sign, priceStrs[i] );
        }
    }

    if ( IsGambling() )
        InitGambling();

    if ( type == Obj_CaveMedicineShop )
    {
        int itemValue = World::GetItem( ItemSlot_Letter );
        if ( itemValue == 2 )
            state = Idle;
        else
            state = WaitingForLetter;
    }

    if ( stringId == String_MoreBombs )
    {
        showNumbers = true;
    }
    else if ( stringId == String_MoneyOrLife )
    {
        showNumbers = true;
    }

    if ( state == Idle )
        World::GetPlayer()->SetState( Player::Paused );
}

void Person::Update()
{
    if ( state == Idle )
    {
        UpdateDialog();
        CheckPlayerHit();

        if ( !World::IsOverworld() )
        {
            CheckCollisions();
            if ( decoration != 0 )
            {
                decoration = 0;
                World::EnablePersonFireballs();
            }
        }
    }
    else if ( state == PickedUp )
    {
        UpdatePickUp();
    }
    else if ( state == WaitingForLetter )
    {
        UpdateWaitForLetter();
    }
    else if ( state == WaitingForFood )
    {
        UpdateWaitForFood();
    }
    else if ( state == WaitingForStairs )
    {
        CheckStairsHit();
    }
}

void Person::UpdateDialog()
{
    if ( textBox->IsDone() )
        return;

    textBox->Update();

    if ( textBox->IsDone() )
    {
        if ( spec.GetStringId() == String_DoorRepair )
        {
            World::PostRupeeLoss( 20 );
            World::Get()->MarkItem();
        }
        else if ( GetType() == Obj_Grumble )
        {
            state = WaitingForFood;
        }
        else if ( GetType() == Obj_CaveShortcut )
        {
            state = WaitingForStairs;
        }

        Player* player = World::GetPlayer();
        if ( player->GetState() == Player::Paused )
            player->SetState( Player::Idle );
    }
}

void Person::CheckPlayerHit()
{
    if ( !spec.GetPickUp() )
        return;

    Player* player = World::GetPlayer();

    int distanceY = abs( ItemY - player->GetY() );
    if ( distanceY >= 6 )
        return;

    for ( int i = 0; i < CaveSpec::Count; i++ )
    {
        int itemId = spec.GetItemId( i );
        if ( itemId != Item_None && player->GetX() == itemXs[i] )
        {
            HandlePlayerHit( i );
            break;
        }
    }
}

void Person::HandlePlayerHit( int index )
{
    if ( spec.GetCheckHearts() )
    {
        int expectedCount = 0;

        if ( GetType() == Obj_Cave3 )
            expectedCount = 5;
        else if ( GetType() == Obj_Cave4 )
            expectedCount = 12;
        else
            assert( false );

        if ( World::GetItem( ItemSlot_HeartContainers ) < expectedCount )
            return;
    }

    if ( spec.GetPay() )
    {
        int price = spec.Prices[index];
        if ( price > World::GetItem( ItemSlot_Rupees ) )
            return;
        World::PostRupeeLoss( price );
    }

    if ( !spec.GetShowPrices() )
        World::Get()->MarkItem();

    if ( spec.GetHint() )
        HandlePickUpHint( index );
    else if ( spec.GetSpecial() )
        HandlePickUpSpecial( index );
    else
        HandlePickUpItem( index );
}

void Person::HandlePickUpItem( int index )
{
    int itemId = spec.GetItemId( index );
    World::AddItem( itemId );
    chosenIndex = index;
    state = PickedUp;
    objTimer = 0x40;
    World::Get()->LiftItem( itemId );
    Sound::PushSong( Song_item_lift );
    spec.ClearShowPrices();
}

void Person::HandlePickUpHint( int index )
{
    int stringId = String_AintEnough;

    if ( index == 2 )
    {
        if ( GetType() == Obj_Cave12 )
            stringId = String_LostHillsHint;
        else if ( GetType() == Obj_Cave13 )
            stringId = String_LostWoodsHint;
        else
            assert( false );
    }

    textBox->Reset( World::GetString( stringId ) );

    spec.ClearShowPrices();
    spec.ClearPickUp();
}

void Person::HandlePickUpSpecial( int index )
{
    if ( IsGambling() )
    {
        int price = spec.Prices[index];
        if ( price > World::GetItem( ItemSlot_Rupees ) )
            return;

        int finalIndex;

        for ( int i = 0; i < CaveSpec::Count; i++ )
        {
            finalIndex = gamblingIndexes[i];
            NumberSign sign = finalIndex != 2 ? NumberSign_Negative : NumberSign_Positive;
            NumberToStringR( gamblingAmounts[finalIndex], sign, priceStrs[i] );
        }

        spec.ClearPickUp();
        finalIndex = gamblingIndexes[index];

        if ( finalIndex == 2 )
            World::PostRupeeWin( gamblingAmounts[finalIndex] );
        else
            World::PostRupeeLoss( gamblingAmounts[finalIndex] );
    }
    else if ( spec.GetStringId() == String_MoreBombs )
    {
        int price = spec.Prices[index];
        if ( price > World::GetItem( ItemSlot_Rupees ) )
            return;

        World::PostRupeeLoss( price );
        World::GetProfile().Items[ItemSlot_MaxBombs] += 4;
        World::GetProfile().Items[ItemSlot_Bombs] = World::GetProfile().Items[ItemSlot_MaxBombs];

        showNumbers = false;
        state = PickedUp;
        objTimer = 0x40;
    }
    else if ( spec.GetStringId() == String_MoneyOrLife )
    {
        int price = spec.Prices[index];
        int itemId = spec.GetItemId( index );

        if ( itemId == Item_Rupee )
        {
            if ( price > World::GetItem( ItemSlot_Rupees ) )
                return;

            World::PostRupeeLoss( price );
        }
        else if ( itemId == Item_HeartContainer )
        {
            if ( price > World::GetItem( ItemSlot_HeartContainers ) )
                return;

            Profile& profile = World::GetProfile();
            if ( profile.Items[ItemSlot_HeartContainers] > 1 )
            {
                profile.Items[ItemSlot_HeartContainers]--;
                if ( profile.Hearts > 0x100 )
                    profile.Hearts -= 0x100;
                Sound::PlayEffect( SEffect_key_heart );
            }
        }
        else
        {
            return;
        }

        World::Get()->MarkItem();
        World::Get()->OpenShutters();

        showNumbers = false;
        state = PickedUp;
        objTimer = 0x40;
    }
    else  // Give money
    {
        uint8_t amount = spec.Prices[index];

        World::PostRupeeWin( amount );
        World::Get()->MarkItem();
        spec.ClearPickUp();
        showNumbers = true;
    }
}

bool Person::IsGambling()
{
    return spec.GetSpecial() && GetType() >= Obj_Cave1 && GetType() < Obj_Cave18;
}

void Person::InitGambling()
{
    for ( int i = 0; i < _countof( gamblingIndexes ); i++ )
        gamblingIndexes[i] = i;

    gamblingAmounts[0] = (Util::GetRandom( 2 ) == 0) ? 10 : 40;
    gamblingAmounts[1] = 10;
    gamblingAmounts[2] = (Util::GetRandom( 2 ) == 0) ? 20 : 50;

    Util::ShuffleArray( gamblingIndexes, _countof( gamblingIndexes ) );
}

void Person::UpdatePickUp()
{
    if ( objTimer == 0 )
    {
        isDeleted = true;

        if ( GetType() == Obj_Grumble )
        {
            World::Get()->MarkItem();
            World::SetItem( ItemSlot_Food, 0 );
            World::SetPersonWallY( 0 );

            Object* food = World::GetObject( FoodSlot );
            if ( food != nullptr && food->GetType() == Obj_Food )
                food->SetDeleted();
        }
    }
}

void Person::UpdateWaitForLetter()
{
    int itemValue = World::GetItem( ItemSlot_Letter );
    if ( itemValue == 2 )
    {
        state = Idle;
        Sound::PlayEffect( SEffect_secret );
    }
}

void Person::UpdateWaitForFood()
{
    Object* food = World::GetObject( FoodSlot );
    if ( food != nullptr && food->GetType() == Obj_Food )
    {
        state = PickedUp;
        objTimer = 0x40;
        Sound::PlayEffect( SEffect_secret );
    }
}

void Person::CheckStairsHit()
{
    static const uint8_t stairsXs[] = { 0x50, 0x80, 0xB0 };

    if ( World::GetPlayer()->GetY() != 0x9D )
        return;

    Util::Array<uint8_t> rooms = World::GetShortcutRooms();
    int             playerX = World::GetPlayer()->GetX();
    int             stairsIndex = -1;

    for ( int i = 0; i < _countof( stairsXs ); i++ )
    {
        if ( playerX == stairsXs[i] )
        {
            stairsIndex = i;
            break;
        }
    }

    if ( stairsIndex < 0 )
        return;

    for ( uint j = 0; j < rooms.Count; j++ )
    {
        if ( rooms.Elems[j] == World::GetRoomId() )
        {
            uint index = j + 1 + stairsIndex;
            if ( index >= rooms.Count )
                index -= rooms.Count;

            World::LeaveCellarByShortcut( rooms.Elems[index] );
            break;
        }
    }
}

void Person::Draw()
{
    if ( state == PickedUp )
    {
        if ( (GetFrameCounter() & 1) == 0 )
            return;
    }
    else if ( state == Idle || state == WaitingForFood || state == WaitingForStairs )
    {
        DrawDialog();
    }

    int animIndex = spec.DwellerType - Obj_OldMan;
    int palette = sPersonGraphics[animIndex].PaletteAttrs;
    palette = CalcPalette( palette );
    image.Draw( Sheet_PlayerAndItems, objX, objY, palette );

    if ( state == WaitingForLetter )
        return;

    for ( int i = 0; i < 3; i++ )
    {
        int itemId = spec.GetItemId( i );

        if ( (itemId < Item_Max) && (state != PickedUp || i != chosenIndex) )
        {
            if ( spec.GetShowItems() )
                DrawItemWide( itemId, itemXs[i], ItemY );
            if ( spec.GetShowPrices() || showNumbers )
                DrawString( priceStrs[i], _countof( priceStrs[i] ), priceXs[i], PriceY, 0 );
        }
    }

    if ( spec.GetShowPrices() )
    {
        DrawItemWide( Item_Rupee, 0x30, 0xAC );
        DrawChar( Char_X, 0x40, 0xB0, 0 );
    }
}

void Person::DrawDialog()
{
    textBox->Draw();
}


//----------------------------------------------------------------------------
//  ItemObj
//----------------------------------------------------------------------------

ItemObj::ItemObj( int itemId, int x, int y, bool isRoomItem )
    :   Object( Obj_Item ),
        itemId( itemId ),
        isRoomItem( isRoomItem ),
        timer( 0 )
{
    objX = x;
    objY = y;
    decoration = 0;

    if ( !isRoomItem )
        timer = 0x1FF;
}

bool ItemObj::TouchesObject( Object* obj )
{
    int distanceX = abs( obj->GetX() + 0 - objX );
    int distanceY = abs( obj->GetY() + 3 - objY );

    return distanceX <= 8
        && distanceY <= 8;
}

void ItemObj::Update()
{
    if ( !isRoomItem )
    {
        timer--;
        if ( timer == 0 )
        {
            isDeleted = true;
            return;
        }
        else if ( timer >= 0x1E0 )
        {
            return;
        }
    }

    bool touchedItem = false;

    if ( TouchesObject( World::GetPlayer() ) )
    {
        touchedItem = true;
    }
    else if ( !isRoomItem )
    {
        static const int weaponSlots[] = { PlayerSwordSlot, BoomerangSlot, ArrowSlot };

        for ( int i = 0; i < _countof( weaponSlots ); i++ )
        {
            int slot = weaponSlots[i];
            Object* obj = World::GetObject( slot );
            if ( obj != nullptr && !obj->IsDeleted() && TouchesObject( obj ) )
            {
                touchedItem = true;
                break;
            }
        }
    }

    if ( touchedItem )
    {
        if ( isRoomItem )
            World::Get()->MarkItem();

        isDeleted = true;

        if ( itemId == Item_PowerTriforce )
        {
            World::OnTouchedPowerTriforce();
            Sound::PlayEffect( SEffect_room_item );
        }
        else
        {
            World::AddItem( itemId );

            if ( itemId == Item_TriforcePiece )
            {
                World::EndLevel();
            }
            else if ( World::IsUWCellar() )
            {
                World::Get()->LiftItem( itemId );
                Sound::PushSong( Song_item_lift );
            }
        }
    }
}

void ItemObj::Draw()
{
    if (   isRoomItem 
        || timer < 0x1E0
        || (timer & 2) != 0 )
    {
        DrawItemWide( itemId, objX, objY );
    }
}


//----------------------------------------------------------------------------
//  Food
//----------------------------------------------------------------------------

Food::Food( int x, int y )
    :   Object( Obj_Food ),
        periods( 3 )
{
    objX = x;
    objY = y;
    decoration = 0;
    objTimer = 0xFF;
}

void Food::Update()
{
    if ( objTimer == 0 )
    {
        periods--;
        if ( periods == 0 )
        {
            isDeleted = true;
            return;
        }
        objTimer = 0xFF;
    }

    // This is how food attracts some monsters.
    int roomObjId = World::Get()->GetRoomObjId();

    if ( (roomObjId >= Obj_BlueMoblin && roomObjId <= Obj_BlueFastOctorock)
        || roomObjId == Obj_Vire
        || roomObjId == Obj_BlueKeese
        || roomObjId == Obj_RedKeese )
    {
        World::Get()->SetObservedPlayerPos( objX, objY );
    }
}

void Food::Draw()
{
    DrawItemWide( Item_Food, objX, objY );
}


//----------------------------------------------------------------------------
//  Whirlwind
//----------------------------------------------------------------------------

Whirlwind::Whirlwind( int x, int y )
    :   Object( Obj_Whirlwind ),
        prevRoomId( 0 )
{
    objX = x;
    objY = y;
    facing = Dir_Right;

    animator.anim = Graphics::GetAnimation( Sheet_Npcs, Anim_OW_Whirlwind );
    animator.durationFrames = 2;
    animator.time = 0;
}

void Whirlwind::SetTeleportPrevRoomId( int roomId )
{
    prevRoomId = roomId;
}

void Whirlwind::Update()
{
    objX += 2;

    Player* player = World::GetPlayer();

    if ( player->GetState() != Player::Paused
        || World::GetWhirlwindTeleporting() == 0 )
    {
        Point thisMiddle = { objX + 8, objY + 5 };
        Point playerMiddle = player->GetMiddle();

        if (   abs( thisMiddle.X - playerMiddle.X ) < 14
            && abs( thisMiddle.Y - playerMiddle.Y ) < 14 )
        {
            player->SetFacing( Dir_Right );
            player->Stop();
            player->SetState( Player::Paused );
            World::SetWhirlwindTeleporting( 1 );

            player->SetY( 0xF8 );
        }
    }
    else
    {
        player->SetX( objX );

        if ( World::GetWhirlwindTeleporting() == 2
            && objX == 0x80 )
        {
            player->SetState( Player::Idle );
            player->SetY( objY );
            World::SetWhirlwindTeleporting( 0 );
            isDeleted = true;
        }
    }

    if ( objX >= 0xF0 )
    {
        isDeleted = true;
        if ( World::GetWhirlwindTeleporting() != 0 )
        {
            World::LeaveRoom( Dir_Right, prevRoomId );
        }
    }

    animator.Advance();
}

void Whirlwind::Draw()
{
    int pal = PlayerPalette + (GetFrameCounter() & 3);
    animator.Draw( Sheet_Npcs, objX, objY, pal );
}


//----------------------------------------------------------------------------
//  Dock
//----------------------------------------------------------------------------

Dock::Dock( int x, int y )
    :   Object( Obj_Dock ),
        state( 0 )
{
    objX = x;
    objY = y;
    decoration = 0;

    raftImage.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Raft );
}

void Dock::Update()
{
    if ( World::GetItem( ItemSlot_Raft ) == 0 )
        return;

    Player* player = World::GetPlayer();

    if ( state == 0 )
    {
        int x;

        if ( World::GetRoomId() == 0x55 )
            x = 0x80;
        else
            x = 0x60;

        if ( x != player->GetX() )
            return;

        objX = x;

        if ( player->GetY() == 0x3D )
            state = 1;
        else if ( player->GetY() == 0x7D )
            state = 2;
        else
            return;

        objY = player->GetY() + 6;

        static const Direction facings[] = { Dir_None, Dir_Down, Dir_Up };

        player->SetState( Player::Paused );
        player->SetFacing( facings[state] );
        Sound::PlayEffect( SEffect_secret );
    }
    else if ( state == 1 )
    {
        // $8FB0
        objY++;
        player->SetY( player->GetY() + 1 );

        if ( player->GetY() == 0x7F )
        {
            player->SetTileOffset( 2 );
            player->SetState( Player::Idle );
            state = 0;
        }

        // Not exactly the same as the original, but close enough.
        player->GetAnimator()->Advance();
    }
    else if ( state == 2 )
    {
        objY--;
        player->SetY( player->GetY() - 1 );

        if ( player->GetY() == 0x3D )
        {
            World::LeaveRoom( player->GetFacing(), World::GetRoomId() );
            player->SetState( Player::Idle );
            state = 0;
        }

        // Not exactly the same as the original, but close enough.
        player->GetAnimator()->Advance();
    }
}

void Dock::Draw()
{
    if ( state != 0 )
        raftImage.Draw( Sheet_PlayerAndItems, objX, objY, PlayerPalette );
}


//----------------------------------------------------------------------------
//  Free functions
//----------------------------------------------------------------------------

static const ItemGraphics sItemGraphics[] = 
{
    { Anim_PI_BombItem,         BluePal },
    { Anim_PI_SwordItem,        PlayerPal },
    { Anim_PI_SwordItem,        BluePal },
    { Anim_PI_MSwordItem,       RedPal },
    { Anim_PI_FleshItem,        RedPal },
    { Anim_PI_RecorderItem,     RedPal },
    { Anim_PI_CandleItem,       BluePal },
    { Anim_PI_CandleItem,       RedPal },
    { Anim_PI_ArrowItem,        PlayerPal },
    { Anim_PI_ArrowItem,        BluePal },
    { Anim_PI_BowItem,          PlayerPal },
    { Anim_PI_MKeyItem,         RedPal },
    { Anim_PI_Raft,             PlayerPal },
    { Anim_PI_Ladder,           PlayerPal },
    { Anim_PI_PowerTriforce,    RedPal | ItemGraphics::FlashPalAttr },
    { Anim_PI_RuppeeItem,       BluePal },
    { Anim_PI_WandItem,         BluePal },
    { Anim_PI_BookItem,         RedPal },
    { Anim_PI_RingItem,         BluePal },
    { Anim_PI_RingItem,         RedPal },
    { Anim_PI_BraceletItem,     RedPal },
    { Anim_PI_MapItem,          BluePal },
    { Anim_PI_Compass,          RedPal },
    { Anim_PI_MapItem,          RedPal },
    { Anim_PI_RuppeeItem,       RedPal | ItemGraphics::FlashPalAttr },
    { Anim_PI_KeyItem,          RedPal },
    { Anim_PI_HeartContainer,   RedPal },
    { Anim_PI_TriforcePiece,    RedPal | ItemGraphics::FlashPalAttr },
    { Anim_PI_MShieldItem,      PlayerPal },
    { Anim_PI_Boomerang,        PlayerPal },
    { Anim_PI_Boomerang,        BluePal },
    { Anim_PI_BottleItem,       BluePal },
    { Anim_PI_BottleItem,       RedPal },
    { Anim_PI_Clock,            RedPal },
    { Anim_PI_Heart,            RedPal | ItemGraphics::FlashPalAttr },
    { Anim_PI_Fairy,            RedPal },
};


int ItemValueToItemId( int slot, int value )
{
    int itemValue = value;

    if ( itemValue == 0 )
        return Item_None;

    if (    slot == ItemSlot_Bombs
        ||  slot == ItemSlot_Letter )
    {
        itemValue = 1;
    }

    uint8_t itemId = equippedItemIds[slot] + itemValue;
    return itemId;
}

int ItemValueToItemId( int slot )
{
    Profile& profile = World::GetProfile();
    return ItemValueToItemId( slot, profile.Items[slot] );
}

Object* MakeProjectile( ObjType type, int x, int y, Direction moving, int slot )
{
    Object* obj = nullptr;
    int origSlot = World::GetCurrentObjectSlot();
    World::SetCurrentObjectSlot( slot );

    switch ( type )
    {
    case Obj_FlyingRock: obj = new FlyingRock( x, y, moving ); break;
    case Obj_PlayerSwordShot: obj = new PlayerSwordShot( x, y, moving ); break;
    case Obj_Arrow: obj = new Arrow( x, y, moving ); break;
    case Obj_MagicWave:
    case Obj_MagicWave2: obj = new MagicWave( type, x, y, moving ); break;
    default: assert( false ); break;
    }

    World::SetCurrentObjectSlot( origSlot );
    return obj;
}

Boomerang* MakeBoomerang( 
    int x, int y, Direction moving, int distance, float speed, int ownerSlot, int slot )
{
    int origSlot = World::GetCurrentObjectSlot();
    World::SetCurrentObjectSlot( slot );
    Boomerang* boomerang = new Boomerang( x, y, moving, distance, speed, ownerSlot );
    World::SetCurrentObjectSlot( origSlot );
    return boomerang;
}

Object* MakePerson( int type, const CaveSpec* spec, int x, int y )
{
    return new Person( (ObjType) type, x, y, spec );
}

Object* MakeItem( int itemId, int x, int y, bool isRoomItem )
{
    Object* obj = nullptr;

    if ( itemId == Item_Fairy )
    {
        obj = new Fairy( x, y );
    }
    else
    {
        obj = new ItemObj( itemId, x, y, isRoomItem );
    }

    return obj;
}

static const ItemGraphics* GetItemGraphics( int itemId )
{
    if ( itemId >= 0x3F )
        return nullptr;
    if ( itemId >= _countof( sItemGraphics ) )
        itemId = 0;

    return &sItemGraphics[itemId];
}

void DrawItem( int itemId, int x, int y, int width )
{
    const ItemGraphics* graphics = GetItemGraphics( itemId );
    if ( graphics == nullptr )
        return;

    SpriteImage image;
    image.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, graphics->AnimId );
    int pal;
    int xOffset = 0;

    if ( width != 0 )
        xOffset = (width - image.anim->width) / 2;

    if ( graphics->HasFlashAttr() )
        pal = (GetFrameCounter() & 8) == 0 ? BluePal : RedPal;
    else
        pal = graphics->GetPalette();

    image.Draw( Sheet_PlayerAndItems, x + xOffset, y, pal );
}

void DrawItemNarrow( int itemId, int x, int y )
{
    DrawItem( itemId, x, y, 8 );
}

void DrawItemWide( int itemId, int x, int y )
{
    DrawItem( itemId, x, y, 16 );
}

void DrawChar( uint8_t ch, int x, int y, int palette )
{
    int srcX = (ch & 0x0F) * 8;
    int srcY = (ch & 0xF0) / 2;

    Graphics::DrawTile( 
        Sheet_Font,
        srcX, srcY,
        8, 8,
        x, y,
        palette,
        0 );
}

void DrawString( const uint8_t* str, int length, int x, int y, int palette )
{
    for ( int i = 0; i < length; i++ )
    {
        DrawChar( str[i], x, y, palette );
        x += 8;
    }
}

void DrawSparkle( int x, int y, int palette, int frame )
{
    SpriteAnimator animator;
    animator.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_Sparkle );
    animator.DrawFrame( Sheet_PlayerAndItems, x, y, palette, frame );
}

void DrawBox( int x, int y, int width, int height )
{
    int x2 = x + width - 8;
    int y2 = y + height - 8;
    int xs[] = { x, x2 };
    int ys[] = { y, y2 };

    DrawChar( 0x69, x,  y,  0 );
    DrawChar( 0x6B, x2, y,  0 );
    DrawChar( 0x6E, x,  y2, 0 );
    DrawChar( 0x6D, x2, y2, 0 );

    for ( int i = 0; i < 2; i++ )
    {
        for ( int _x = x + 8; _x < x2; _x += 8 )
        {
            DrawChar( 0x6A, _x, ys[i], 0 );
        }

        for ( int _y = y + 8; _y < y2; _y += 8 )
        {
            DrawChar( 0x6C, xs[i], _y, 0 );
        }
    }
}

void DrawHearts( uint heartsValue, uint totalHearts, int left, int top )
{
    unsigned int partialValue = heartsValue & 0xFF;
    unsigned int fullHearts = heartsValue >> 8;
    unsigned int fullAndPartialHearts = fullHearts;

    if ( partialValue > 0 )
    {
        fullAndPartialHearts++;

        if ( partialValue >= 0x80 )
            fullHearts++;
    }

    int x = left;
    int y = top;

    for ( unsigned int i = 0; i < totalHearts; i++ )
    {
        int tile;

        if ( i < fullHearts )
            tile = Char_FullHeart;
        else if ( i < fullAndPartialHearts )
            tile = Char_HalfHeart;
        else
            tile = Char_EmptyHeart;

        DrawChar( tile, x, y, RedBgPalette );

        x += 8;
        if ( (i % 8) == 7 )
        {
            x = left;
            y -= 8;
        }
    }
}

void DrawFileIcon( int x, int y, int quest )
{
    if ( quest == 1 )
    {
        SpriteImage sword;
        sword.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_SwordItem );
        sword.Draw( Sheet_PlayerAndItems, x + 12, y - 3, 7 );
    }

    SpriteImage player;
    player.anim = Graphics::GetAnimation( Sheet_PlayerAndItems, Anim_PI_LinkWalk_NoShield_Down );
    player.Draw( Sheet_PlayerAndItems, x, y, PlayerPalette );
}

void SetPilePalette()
{
    const static uint8_t palette[] = { 0, 0x27, 0x06, 0x16 };
    Graphics::SetPaletteIndexed( LevelFgPalette, palette );
}

void PlayItemSound( int itemId )
{
    int soundId = SEffect_item;

    if ( itemId == Item_Heart || itemId == Item_Key )
        soundId = SEffect_key_heart;
    else if ( itemId == Item_5Rupees || itemId == Item_Rupee )
        soundId = SEffect_cursor;
    else if ( itemId == Item_PowerTriforce )
        return;

    Sound::PlayEffect( soundId );
}
