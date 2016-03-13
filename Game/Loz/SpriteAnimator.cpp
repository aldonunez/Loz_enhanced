/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "SpriteAnimator.h"
#include "Graphics.h"


void SpriteAnimator::SetDuration( int frames )
{
    if ( time >= frames )
        time = 0;
    durationFrames = frames;
}

void SpriteAnimator::Advance()
{
    time = (time + 1) % durationFrames;
}

void SpriteAnimator::AdvanceFrame()
{
    if ( anim != nullptr && anim->length > 0 && durationFrames > 0 )
    {
        int frameDuration = durationFrames / anim->length;
        time = (time + frameDuration) % durationFrames;
    }
}

void SpriteAnimator::Draw( int sheetSlot, int x, int y, int palette, int flags )
{
    if ( anim != nullptr && anim->length > 0 && durationFrames > 0 )
    {
        int index = (anim->length * time) / durationFrames;

        DrawFrameInternal( sheetSlot, x, y, palette, index, flags );
    }
}

void SpriteAnimator::DrawFrame( int sheetSlot, int x, int y, int palette, int frame, int flags )
{
    if ( anim != nullptr && anim->length > frame )
    {
        DrawFrameInternal( sheetSlot, x, y, palette, frame, flags );
    }
}

void SpriteAnimator::DrawFrameInternal( int sheetSlot, int x, int y, int palette, int frame, int flags )
{
    int index = frame;
    Graphics::DrawSpriteTile(
        sheetSlot,
        anim->frames[index].x,
        anim->frames[index].y,
        anim->width,
        anim->height,
        x,
        y,
        palette,
        anim->frames[index].flags | flags
        );
}


void SpriteImage::Draw( int sheetSlot, int x, int y, int palette, int flags )
{
    Graphics::DrawSpriteTile(
        sheetSlot,
        anim->frames[0].x,
        anim->frames[0].y,
        anim->width,
        anim->height,
        x,
        y,
        palette,
        anim->frames[0].flags | flags
        );
}
