/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Profile.h"


enum
{
    MaxProfiles = 3,
};


struct ProfileSummary
{
    uint8_t     NameLength;
    uint8_t     Name[MaxNameLength];
    uint8_t     Quest;
    uint8_t     Deaths;
    uint8_t     HeartContainers;

    bool IsActive() const
    {
        return NameLength > 0;
    }
};


struct ProfileSummarySnapshot
{
    ProfileSummary  Summaries[MaxProfiles];
};


class SaveFolder
{
public:
    static void ReadSummaries( ProfileSummarySnapshot& summaries );
    static bool ReadProfile( int index, Profile& profile );
    static bool WriteProfile( int index, const Profile& profile );
};
