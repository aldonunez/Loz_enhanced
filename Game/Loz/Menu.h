/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class Menu
{
public:
    virtual ~Menu() { }
    virtual void Update() = 0;
    virtual void Draw() = 0;
};
