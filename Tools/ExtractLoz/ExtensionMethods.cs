/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.Collections.Generic;
using System.Text;

namespace ExtractLoz
{
    static class ExtensionMethods
    {
        public static bool EqualsIgnore( this string stringA, string stringB )
        {
            return stringA.Equals( stringB, StringComparison.InvariantCultureIgnoreCase );
        }
    }
}
