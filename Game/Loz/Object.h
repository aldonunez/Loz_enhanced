/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum CollisionResponse;
enum ObjType;
struct TileCollision;


enum ObjInterfaces
{
    ObjItf_None,
    ObjItf_IBlocksPlayer,
    ObjItf_IThrower,
    ObjItf_IShot,
};


struct CollisionContext
{
    int     WeaponSlot;
    int     DamageType;
    int     Damage;

    Point   Distance;
};


struct PlayerCollision
{
    bool    Collides;
    bool    ShotCollides;

    PlayerCollision( bool collides, bool shotCollides = false )
        :   Collides( collides ),
            ShotCollides( shotCollides )
    {
    }

    operator bool() const { return Collides; }
};


class Object
{
    uint8_t     type;

protected:
    bool        isDeleted;
    uint8_t     decoration;
    uint8_t     hp;
    uint8_t     invincibilityTimer;
    uint8_t     invincibilityMask;
    uint8_t     shoveDir;
    uint8_t     shoveDistance;
    Direction   facing;
    uint8_t     objX;
    uint8_t     objY;
    int8_t      tileOffset;
    uint8_t     fraction;
    uint8_t     moving;
    uint8_t     objTimer;
    uint8_t     stunTimer;

public:
    Object( ObjType type );
    virtual ~Object();

    ObjType GetType();
    bool IsDeleted();
    void SetDeleted();
    bool IsPlayer();
    int  GetDecoration();
    void SetDecoration( int decoration );
    Direction GetFacing();
    Direction GetMoving();
    uint GetX();
    uint GetY();
    void SetX( uint x );
    void SetY( uint y );
    int  GetTileOffset();
    void SetTileOffset( int value );
    uint GetObjectTimer();
    void SetObjectTimer( uint value );
    uint GetStunTimer();
    void SetStunTimer( uint value );
    void SetShoveDir( Direction dir );
    void SetShoveDistance( int distance );

    void DecrementObjectTimer();
    void DecrementStunTimer();

    bool DecoratedUpdate();
    void DecoratedDraw();

    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void* GetInterface( ObjInterfaces iface );

protected:
    int CalcPalette( int wantedPalette );
    bool IsStunned();

    bool CheckCollisions();
    void CheckSword( int slot );
    void CheckBoomerang( int slot );
    void CheckWave( int slot );
    void CheckBombAndFire( int slot );
    bool CheckArrow( int slot );
    PlayerCollision CheckPlayerCollision();
    PlayerCollision CheckPlayerCollisionDirect();

    void ObjMove( int speed );
    void ObjMoveDir( int speed, Direction dir );
    void ObjShove();
    Direction CheckWorldMargin( Direction dir );
    void TurnToUnblockedDir();
    Direction GetSingleMoving();
    static bool DoObjectsCollide( 
        Point obj1, 
        Point obj2, 
        Point box, 
        Point& distance );
    void Shove( CollisionContext& context );
    void PlayBossHitSoundIfHit();
    void PlayBossHitSoundIfDied();

private:
    Direction CheckWorldBounds( Direction dir );

    Point CalcObjMiddle();
    bool CheckCollisionNoShove( 
        CollisionContext& context,
        Point box );
    bool CheckCollisionCustomNoShove( 
        CollisionContext& context,
        Point box,
        Point weaponOffset );
    void HandleCommonHit( CollisionContext& context );
    void DealDamage( CollisionContext& context );
    void KillObjectNormally( CollisionContext& context );
    void PlayParrySoundIfSupported( int damageType );
    void PlayParrySound();
    void ShoveCommon( CollisionContext& context );
    void ShoveObject( CollisionContext& context );

    void ObjMoveWhole( int speed, Direction dir, int align );
    void ObjMoveFourth( int speed, Direction dir, int align );
    void MoveShoveWhole();

    Direction CheckTileCollision( Direction dir );
    Direction FindUnblockedDir( Direction dir, int firstStep );
    Direction GetNextAltDir( int& seq, Direction dir );

    Direction StopAtPersonWall( Direction dir );
    Direction StopAtPersonWallUW( Direction dir );

    Direction CheckSubroom( Direction dir );
    Direction CheckDoorways( Direction dir );
    Direction CheckDoor( Direction dir, int dirOrd );
    Direction HandleLadder( Direction dir );
    Direction MoveOnLadder( Direction dir, int distance );
    Direction StopAtBlock( Direction dir );
    void PushOWTile( TileCollision& collision );
    bool HitsWorldLimit();
    void StopPlayer();
};


class IBlocksPlayer
{
public:
    virtual CollisionResponse CheckCollision() = 0;
};


class IThrower
{
public:
    virtual void Catch() = 0;
};


class IShot
{
public:
    virtual bool IsInShotStartState() = 0;
};


struct ObjectAttr
{
    enum
    {
        CustomCollision     = 1,
        CustomDraw          = 4,
        Unknown10__         = 0x10,
        InvincibleToWeapons = 0x20,
        HalfWidth           = 0x40,
        Unknown80__         = 0x80,
        WorldCollision      = 0x100,
    };

    uint16_t Data;

    bool GetCustomCollision() const
    {
        return (Data & CustomCollision) != 0;
    }

    bool GetUnknown10__() const
    {
        return (Data & Unknown10__) != 0;
    }

    bool GetInvincibleToWeapons() const
    {
        return (Data & InvincibleToWeapons) != 0;
    }

    bool GetHalfWidth() const
    {
        return (Data & HalfWidth) != 0;
    }

    bool GetUnknown80__() const
    {
        return (Data & Unknown80__) != 0;
    }

    bool GetWorldCollision() const
    {
        return (Data & WorldCollision) != 0;
    }

    int GetItemDropClass() const
    {
        return (Data >> 9) & 7;
    }
};


struct HPAttr
{
    uint8_t Data;

    int GetHP( int type ) const
    {
        if ( (type & 1) == 0 )
            return Data & 0xF0;
        else
            return Data << 4;
    }
};


Direction CheckWorldMarginH( int x, Direction dir, bool adjust );
Direction CheckWorldMarginV( int y, Direction dir, bool adjust );
