/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Menu.h"

struct ProfileSummarySnapshot;


class GameMenu : public Menu
{
    std::shared_ptr<ProfileSummarySnapshot> summaries;
    int selectedIndex;

public:
    GameMenu( const std::shared_ptr<ProfileSummarySnapshot>& summaries );

    virtual void Update();
    virtual void Draw();

private:
    void SelectNext();
};
