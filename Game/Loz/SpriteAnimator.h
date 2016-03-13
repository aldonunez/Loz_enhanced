/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


struct SpriteAnim;


struct SpriteAnimator
{
    const SpriteAnim*   anim;
    int                 time;
    int                 durationFrames;

    void SetDuration( int frames );
    void Advance();
    void AdvanceFrame();
    void Draw( int sheetSlot, int x, int y, int palette, int flags=0 );
    void DrawFrame( int sheetSlot, int x, int y, int palette, int frame, int flags=0 );

private:
    void DrawFrameInternal( int sheetSlot, int x, int y, int palette, int frame, int flags );
};


struct SpriteImage
{
    const SpriteAnim*   anim;

    void Draw( int sheetSlot, int x, int y, int palette, int flags = 0 );
};
