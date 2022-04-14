/*
*   LMMS Project Packager
*   Copyright © 2022 Luxon Jean-Pierre
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "options.hpp"
#include "../external/filesystem/filesystem.hpp"

#include <iostream>
#include <stdexcept>


namespace fs = ghc::filesystem;

namespace options
{

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept;

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept
{
    if ( !path.empty() && path.back() != '/' )
    {
        return path + '/';
    }
    return path;
}


Options::~Options()
{
    // Empty
}

const Options retrieveImportExportArguments( const OperationType& op, const int argc, const char * argv[] );

// TODO use a proper argument parser
const Options retrieveImportExportArguments( const OperationType& op, const int argc, const char * argv[] )
{
    const int MIN_AGUMENTS_NUMBER = 4;
    if ( argc < MIN_AGUMENTS_NUMBER )
    {
        throw std::invalid_argument( "Not enough arguments to execute the requested operation." );
    }

    const std::string& operation_str = argv[1];
    const std::string& project_file = fs::normalize( argv[2] );
    const std::string& destination_directory = addTrailingSlashIfNeeded( fs::normalize( argv[3] ) );

    if ( op == OperationType::Import )
    {
        int argvpos = MIN_AGUMENTS_NUMBER;
        while ( argvpos < argc && std::string( argv[argvpos] ) != "--verbose" )
        {
            argvpos++;
        };
        return Options { op, project_file, destination_directory, false, false, (argvpos < argc), {}, "" };
    }

    // Assuming this is OperationType::Export
    bool sf2_export = true;
    bool zip = true;
    bool verbose = false;
    bool resources_set = false;

    // Some resources can be located in the directory where the project is.
    // It is possible that the path to the resource is relative to the project directory,
    // That is why by default the resource directory contains at least the project directory.
    std::vector<std::string> resource_dirs { fs::path( project_file ).parent_path().string() + "/" };
    std::string lmms_exe = "lmms";
    bool lmms_exe_set = false;

    int argvpos = MIN_AGUMENTS_NUMBER;
    while ( argvpos < argc )
    {
        const std::string& opt = argv[argvpos];
        if ( ( opt == "--no-sf2" ) && ( op == OperationType::Export ) )
        {
            sf2_export = false;
        }
        else if ( opt == "--no-zip" && op == OperationType::Export )
        {
            zip = false;
        }
        else if ( ( opt == "--rsc-dirs" ) && ( op == OperationType::Export ) )
        {
            if ( !resources_set )
            {
                while ( ((argvpos + 1 ) < argc) && std::string( argv[argvpos + 1] ).find( "--" ) )
                {
                    std::string directory = addTrailingSlashIfNeeded( fs::normalize( argv[argvpos + 1] ) );
                    resource_dirs.push_back( directory );
                    argvpos++;
                }
                resources_set = true;
            }
            else
            {
                std::cerr << "-- Resource directories already set. Subsequent uses of the option are ignored.\n";
            }
        }
        else if ( opt == "--lmms-exe" )
        {
            if ( !lmms_exe_set )
            {
                lmms_exe = argv[argvpos + 1];
                lmms_exe_set = true;
                argvpos++;
            }
            else
            {
                std::cerr << "LMMS custom executable already set. Executable ignored.\n";
            }
        }
        else if ( opt == "--verbose" )
        {
            verbose = true;
        }
        else
        {
            std::cerr << "Parameter \"" << opt << "\" is ignored.\n";
        }
        argvpos++;
    }

    if ( !sf2_export && verbose )
    {
        std::cout << "-- Ignore Soundfont2 files\n";
    }

    if ( !zip  && verbose )
    {
        std::cout << "-- The destination package will not be zipped\n";
    }

    if ( lmms_exe_set && verbose )
    {
        std::cout << "-- An LMMS executable has been set: " << lmms_exe << "\n";
    }

    if ( !resource_dirs.empty() && verbose )
    {
        std::cout << "-- The following resource directories have been set: \n";
        for ( const std::string& dir : resource_dirs )
        {
            std::cout << "*  " << dir << "\n";
        }
    }

    return Options{ op, project_file, destination_directory, sf2_export, zip, verbose, resource_dirs, lmms_exe };

}

/*
    argv[0]: program
    argv[1]: --export | --import
    argv[2]: project file

    // Depending on operation

    Export:
    argv[3]: destination directory
    argv[4 -> argc - 1]: optional parameters

    Import:
    argv[3]: destination directory
*/
const Options retrieveArguments( const int argc, const char * argv[] )
{
    const OperationType op = [&]()
    {
        const std::string& operation_str = argv[1];
        if ( operation_str == "--import" )
        {
            return OperationType::Import;
        }
        else if ( operation_str == "--export" )
        {
            return OperationType::Export;
        }
        else if ( operation_str == "--check" )
        {
            return OperationType::Check;
        }
        else if ( operation_str == "--info" )
        {
            return OperationType::Info;
        }
        return OperationType::InvalidOperation;
    } ();

    if ( op == OperationType::Import || op == OperationType::Export )
    {
        return retrieveImportExportArguments( op, argc, argv );
    }
    else if ( op == OperationType::Check || op == OperationType::Info )
    {
        bool verbose = false;
        for ( int i = 0; i < argc; i++ )
        {
            const std::string& option_str = argv[i];
            if ( option_str == "--verbose" )
            {
                verbose = true;
            }
        }

        return Options{ op, argv[2], "", false, false, verbose, {}, "" };
    }
    else
    {
        throw std::invalid_argument( "FATAL ERROR: invalid operation selection in this function: "
                                     + std::string( argv[1] ) );
    }
}

}
