/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Object.h"
#include "SpriteAnimator.h"

class TextBox;


class Ladder : public Object
{
    int             state;
    Direction       origDir;
    SpriteImage     image;

public:
    Ladder();

    virtual void Update();
    virtual void Draw();

    int  GetState();
    void SetState( int state );
};


struct BlockSpec;

class BlockObjBase : public Object, public IBlocksPlayer
{
    typedef void (BlockObjBase::*UpdateFunc)();

    int                 timer;
    int                 targetPos;
    const BlockSpec*    spec;
    int                 origX;
    int                 origY;

    UpdateFunc          curUpdate;

public:
    BlockObjBase( ObjType type, const BlockSpec* spec );

    virtual void Update();
    virtual void Draw();
    virtual void* GetInterface( ObjInterfaces iface );

    void SetX( int x );
    void SetY( int y );

    CollisionResponse CheckCollision();

private:
    void UpdateIdle();
    void UpdateMoving();
};

class Rock : public BlockObjBase
{
public:
    Rock();
};

class Headstone : public BlockObjBase
{
public:
    Headstone();
};

class Block : public BlockObjBase
{
public:
    Block();
};


class Fire : public Object
{
public:
    enum State
    {
        Moving,
        Standing
    };

private:
    State           state;
    SpriteAnimator  animator;

public:
    Fire();

    void SetMoving( Direction dir );
    State GetLifetimeState();
    void SetLifetimeState( State state );

    void SetX( int x );
    void SetY( int y );
    Point GetMiddle();

    virtual void Update();
    virtual void Draw();

private:
    void CheckCollisionWithPlayer();
};


class Tree : public Object
{
    int             x;
    int             y;

public:
    Tree();

    void SetX( int x );
    void SetY( int y );

    virtual void Update();
    virtual void Draw();
    virtual int GetX();
    virtual int GetY();
};


class Bomb : public Object
{
public:
    enum State
    {
        Initing,
        Ticking,
        Blasting,
        Fading,
    };

private:
    State           state;
    SpriteAnimator  animator;

public:
    Bomb();

    State GetLifetimeState();
    void SetLifetimeState( State state );

    void SetX( int x );
    void SetY( int y );

    virtual void Update();
    virtual void Draw();
};


class RockWall : public Object
{
    int             x;
    int             y;

public:
    RockWall();

    void SetX( int x );
    void SetY( int y );

    virtual void Update();
    virtual void Draw();
    virtual int GetX();
    virtual int GetY();
};


class PlayerSword : public Object
{
    int             state;
    int             timer;
    SpriteImage     image;

public:
    PlayerSword( ObjType type );

    int GetState();

    virtual void Update();
    virtual void Draw();

private:
    void Put();
    void TryMakeWave();
    void MakeWave();
};


class Shot : public Object, public IShot
{
public:
    enum State
    {
        Flying,
        Spark,
        Bounce,
        Spreading,
    };

protected:
    State       state;

private:
    Direction   bounceDir;

public:
    Shot( ObjType type );
    ~Shot();

    State GetLifetimeState();

    virtual void* GetInterface( ObjInterfaces iface );
    virtual bool IsInShotStartState();

protected:
    void Move( int speed );
    void CheckPlayer();
    void UpdateBounce();
    void DeleteShot();
};


class PlayerSwordShot : public Shot
{
public:
    int             distance;
    SpriteImage     image;

public:
    PlayerSwordShot( int x, int y, Direction moving );

    virtual void Update();
    virtual void Draw();

    void SpreadOut();

private:
    void UpdateFlying();
    void UpdateSpreading();
};


class FlyingRock : public Shot
{
public:
    SpriteImage     image;

public:
    FlyingRock( int x, int y, Direction moving );

    virtual void Update();
    virtual void Draw();

private:
    void UpdateFlying();
};


class Fireball : public Object
{
public:
    int             state;
    float           x;
    float           y;
    float           speedX;
    float           speedY;
    SpriteImage     image;

public:
    Fireball( ObjType type, int x, int y, float speed );

    void SetY( int y );

    virtual void Update();
    virtual void Draw();
};


class Boomerang : public Object, public IShot
{
private:
    int             startX;
    int             startY;
    int             distanceTarget;
    ObjRef          ownerRef;
    float           x;
    float           y;
    float           leaveSpeed;
    int             state;
    int             animTimer;
    SpriteAnimator  animator;

public:
    Boomerang( int x, int y, Direction moving, int distance, float speed, Object* owner );
    ~Boomerang();

    void SetState( int state );

    virtual void Update();
    virtual void Draw();
    virtual void* GetInterface( ObjInterfaces iface );
    virtual bool IsInShotStartState();

private:
    void UpdateLeaveFast();
    void UpdateLeaveSlow();
    void UpdateReturn();
    void UpdateSpark();

    void AdvanceAnimAndCheckCollision();
    void CheckCollision();
};


class MagicWave : public Shot
{
public:
    SpriteImage     image;

public:
    MagicWave( ObjType type, int x, int y, Direction moving );

    virtual void Update();
    virtual void Draw();

    void AddFire();

private:
    void UpdateFlying();
};


class Arrow : public Shot
{
    int             timer;
    SpriteImage     image;

public:
    Arrow( int x, int y, Direction moving );

    void SetSpark( int frames = 3 );

    virtual void Update();
    virtual void Draw();

private:
    void UpdateArrow();
    void UpdateSpark();
};


struct CaveSpec
{
    enum
    {
        Count = 3,
    };

    uint8_t DwellerType;
    uint8_t StringId;
    uint8_t Items[Count];
    uint8_t Prices[Count];

    int GetStringId() const         { return StringId & 0x3F; }
    int GetItemId( int i ) const    { return Items[i] & 0x3F; }
    bool GetPay() const             { return (StringId & 0x80) != 0 ? true : false; }
    bool GetPickUp() const          { return (StringId & 0x40) != 0 ? true : false; }
    bool GetShowNegative() const    { return (Items[0] & 0x80) != 0 ? true : false; }
    bool GetCheckHearts() const     { return (Items[0] & 0x40) != 0 ? true : false; }
    bool GetSpecial() const         { return (Items[1] & 0x80) != 0 ? true : false; }
    bool GetHint() const            { return (Items[1] & 0x40) != 0 ? true : false; }
    bool GetShowPrices() const      { return (Items[2] & 0x80) != 0 ? true : false; }
    bool GetShowItems() const       { return (Items[2] & 0x40) != 0 ? true : false; }

    void ClearPickUp()              { StringId &= ~0x40; }
    void ClearShowPrices()          { Items[2] &= ~0x80; }

    void SetPickUp()                { StringId |= 0x40; }
    void SetShowNegative()          { Items[0] |= 0x80; }
    void SetSpecial()               { Items[1] |= 0x80; }
    void SetShowPrices()            { Items[2] |= 0x80; }
    void SetShowItems()             { Items[2] |= 0x40; }
};


class Person : public Object
{
    enum State
    {
        Idle,
        PickedUp,
        WaitingForLetter,
        WaitingForFood,
        WaitingForStairs,
    };

    State           state;
    SpriteImage     image;

    CaveSpec        spec;
    TextBox*        textBox;
    int             chosenIndex;
    bool            showNumbers;

    uint8_t         priceStrs[3][4];

    uint8_t         gamblingAmounts[3];
    uint8_t         gamblingIndexes[3];

public:
    Person( ObjType type, int x, int y, const CaveSpec* spec );

    virtual void Update();
    virtual void Draw();

private:
    void UpdateDialog();
    void DrawDialog();

    void CheckStairsHit();
    void CheckPlayerHit();
    void HandlePlayerHit( int index );
    void HandlePickUpItem( int index );
    void HandlePickUpHint( int index );
    void HandlePickUpSpecial( int index );
    void UpdatePickUp();

    bool IsGambling();
    void InitGambling();

    void UpdateWaitForLetter();
    void UpdateWaitForFood();
};


class ItemObj : public Object
{
    int     itemId;
    bool    isRoomItem;
    int     timer;

public:
    ItemObj( int itemId, int x, int y, bool isRoomItem );

    virtual void Update();
    virtual void Draw();

private:
    bool TouchesObject( Object* obj );
};


class Food : public Object
{
    int     periods;

public:
    Food( int x, int y );

    virtual void Update();
    virtual void Draw();
};


class Whirlwind : public Object
{
    uint8_t         prevRoomId;
    SpriteAnimator  animator;

public:
    Whirlwind( int x, int y );

    virtual void Update();
    virtual void Draw();

    void SetTeleportPrevRoomId( int roomId );
};


class Dock : public Object
{
    int         state;
    SpriteImage raftImage;

public:
    Dock( int x, int y );

    virtual void Update();
    virtual void Draw();
};


Boomerang* MakeBoomerang( 
    int x, int y, Direction moving, int distance, float speed, Object* owner, int slot );
Object* MakeProjectile( ObjType type, int x, int y, Direction moving, int slot );
Object* MakePerson( int type, const CaveSpec* spec, int x, int y );
Object* MakeItem( int itemId, int x, int y, bool isRoomItem );

const uint8_t   Char_X          = 0x21;
const uint8_t   Char_Space      = 0x24;
const uint8_t   Char_JustSpace  = 0x25;
const uint8_t   Char_Minus      = 0x62;
const uint8_t   Char_Plus       = 0x64;

enum
{
    Char_FullHeart  = 0xF2,
    Char_HalfHeart  = 0x65,
    Char_EmptyHeart = 0x66,

    Char_BoxTL      = 0x69,
    Char_BoxTR      = 0x6B,
    Char_BoxBL      = 0x6E,
    Char_BoxBR      = 0x6D,
};

void DrawItemWide( int itemId, int x, int y );
void DrawItemNarrow( int itemId, int x, int y );
void DrawItem( int itemId, int x, int y, int width );
void DrawChar( uint8_t ch, int x, int y, int palette );
void DrawString( const uint8_t* str, int length, int x, int y, int palette );
void DrawSparkle( int x, int y, int palette, int frame );
void DrawBox( int x, int y, int width, int height );
void DrawHearts( uint heartsValue, uint totalHearts, int x, int y );
void DrawFileIcon( int x, int y, int quest );

void SetPilePalette();

enum NumberSign
{
    NumberSign_None,
    NumberSign_Negative,
    NumberSign_Positive,
};

uint8_t* NumberToStringR( uint8_t number, NumberSign sign, uint8_t* charBuf, int bufLen = 4 );
int ItemValueToItemId( int slot, int value );
int ItemValueToItemId( int slot );

void PlayItemSound( int itemId );
