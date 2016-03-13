/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

struct Line;


class Credits
{
public:
    enum
    {
        StartY       = StdViewHeight,
    };

private:
    enum
    {
        AllLines     = 96,
        AllLineBytes = 12,
    };

    typedef Util::Table<uint8_t> TextTable;

    TextTable   textTable;
    uint8_t     lineBmp[AllLineBytes];
    int         fraction;
    int         tileOffset;
    int         top;
    int         windowTop;
    int         windowTopLine;
    int         windowBottomLine;
    int         windowFirstMappedLine;
    uint8_t     playerLine[32];
    bool        madePlayerLine;

public:
    Credits();

    bool IsDone();
    int  GetTop();

    void Update();
    void Draw();

private:
    int GetTopLineAtEnd();
    void MakePlayerLine( const Line* line );
};
