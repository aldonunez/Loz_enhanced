/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Menu.h"
#include "SaveFolder.h"


class RegisterMenu : public Menu
{
    std::shared_ptr<ProfileSummarySnapshot> summaries;
    int selectedIndex;
    int namePos;
    int charPosCol;
    int charPosRow;
    bool origActive[MaxProfiles];

public:
    RegisterMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries );

    virtual void Update();
    virtual void Draw();

private:
    void SelectNext();
    void MoveNextNamePosition();
    void AddCharToName( char ch );
    char GetSelectedChar();
    void MoveCharSetCursorH( int dir );
    void MoveCharSetCursorV( int dir, bool skip = true );
    void CommitFiles();
};
