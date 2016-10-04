/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace ExtractLoz
{
    class Options
    {
        public string RomPath;
        public string NsfPath;
        public string Function;
        public string OutPath;
        public string Error;

        public static Options Parse( string[] args )
        {
            Options options = new Options();

            if ( args.Length < 2 )
                return options;

            options.RomPath = args[0];
            options.Function = args[1].ToLowerInvariant();
            options.OutPath = Environment.CurrentDirectory;

            for ( int i = 2; i < args.Length; i++ )
            {
                if ( args[i].EqualsIgnore( "-out" ) )
                {
                    if ( i == args.Length - 1 )
                    {
                        options.Error = "Output path is missing.";
                        break;
                    }

                    options.OutPath = GetFullPath( args[i + 1], out options.Error );
                    if ( options.OutPath == null )
                    {
                        options.Error = "Output path: " + options.Error;
                        break;
                    }
                    i++;
                }
                else if ( args[i].EqualsIgnore( "-nsf" ) )
                {
                    if ( i == args.Length - 1 )
                    {
                        options.Error = "Nsf path is missing.";
                        break;
                    }

                    options.NsfPath = GetFullPath( args[i + 1], out options.Error );
                    if ( options.NsfPath == null )
                    {
                        options.Error = "Nsf path: " + options.Error;
                        break;
                    }
                    i++;
                }
            }

            return options;
        }

        static string GetFullPath( string path, out string error )
        {
            try
            {
                error = null;
                return Path.GetFullPath( path );
            }
            catch ( ArgumentException e )
            {
                error = e.Message;
            }
            catch ( System.Security.SecurityException e )
            {
                error = e.Message;
            }
            catch ( NotSupportedException e )
            {
                error = e.Message;
            }
            catch ( PathTooLongException e )
            {
                error = e.Message;
            }

            return null;
        }

        public string MakeOutPath( string relativePath )
        {
            return Path.Combine( OutPath, relativePath );
        }
    }
}
