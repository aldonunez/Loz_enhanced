/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Object.h"
#include "SpriteAnimator.h"

#include "Input.h"

typedef std::array<uint8_t, 4> Limits;


class Player : public Object, public IThrower
{
public:
    enum
    {
        WalkSpeed       = 0x60,
        StairsSpeed     = 0x30,
    };

    enum State
    {
        Idle,
        Wielding,
        Paused,
    };

private:
    uint8_t             state;
    uint8_t             speed;
    uint8_t             tileRef;
    bool                paralyzed;
    uint8_t             animTimer;
    uint8_t             avoidTurningWhenDiag;   // 56
    uint8_t             keepGoingStraight;      // 57
    InputButtons        curButtons;
    SpriteAnimator      animator;

public:
    Player();

    virtual void Update();
    virtual void Draw();
    virtual void* GetInterface( ObjInterfaces iface );

    int GetState();
    void SetState( State state );
    void SetFacing( Direction dir );
    Bounds GetBounds();
    Point GetMiddle();
    SpriteAnimator* GetAnimator();
    int  GetInvincibilityTimer();
    void SetInvincibilityTimer( int value );
    bool IsParalyzed();
    void SetParalyzed( bool value );
    void ResetShove();

    void Catch();
    void BeHarmed( Object* collider, Point& otherMiddle );
    void BeHarmed( Object* collider, Point& otherMiddle, uint16_t damage );
    void DecInvincibleTimer();
    void Stop();
    void MoveLinear( Direction dir, int speed );

    static Limits GetPlayerLimits();

private:
    void SetFacingAnim();

    void HandleInput();
    void SetMovingInDoorway();
    void FilterBorderInput();
    void SetSpeed();
    void Align();
    void CalcAlignedMoving();
    void CheckWater();
    void CheckWarp();
    void Animate();

    int UseWeapon();
    int UseWeapon( ObjType type, int itemSlot );
    int UseItem();
};
