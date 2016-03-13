/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Profile.h"


uint Profile::GetMaxHeartsValue()
{
    return GetMaxHeartsValue( Items[ItemSlot_HeartContainers] );
}

uint Profile::GetMaxHeartsValue( uint heartContainers )
{
    return (heartContainers << 8) - 1;
}
