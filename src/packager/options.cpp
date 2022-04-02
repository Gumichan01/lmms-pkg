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
        return Options { op, project_file, destination_directory };
    }

    // Assuming this is OperationType::Export
    bool sf2_export = true;
    bool zip = true;
    std::vector<std::string> lmms_dirs;
    std::string lmms_exe = "lmms";
    bool lmms_exe_set = false;

    int argvpos = MIN_AGUMENTS_NUMBER;
    while ( argvpos < argc )
    {
        const std::string& opt = argv[argvpos];
        if ( ( opt == "--no-sf2" ) && ( op == OperationType::Export ) )
        {
            std::cout << "-- Ignore Soundfont2 files\n";
            sf2_export = false;
        }
        else if ( opt == "--no-zip" && op == OperationType::Export )
        {
            std::cout << "-- The destination package will not be zipped\n";
            zip = false;
        }
        else if ( ( opt == "--lmms-dir" ) && ( op == OperationType::Export ) )
        {
            if ( lmms_dirs.empty() )
            {
                while ( ((argvpos + 1 ) < argc) && std::string( argv[argvpos + 1] ).find( "--" ) )
                {
                    std::string directory = addTrailingSlashIfNeeded( fs::normalize( argv[argvpos + 1] ) );
                    lmms_dirs.push_back( directory );
                    std::cout << "-- An LMMS directory has been set: " << directory << "\n";
                    argvpos++;
                }
            }
            else
            {
                std::cout << "-- LMMS directory already set. Directory ignored.\n";
            }
        }
        else if ( opt == "--lmms-exe" )
        {
            if ( !lmms_exe_set )
            {
                lmms_exe = argv[argvpos + 1];
                std::cout << "-- An LMMS executable has been set: " << lmms_exe << "\n";
                lmms_exe_set = true;
                argvpos++;
            }
            else
            {
                std::cout << "LMMS custom executable already set. Executable ignored.\n";
            }
        }
        else
        {
            std::cout << "Parameter \"" << opt << "\" is ignored.\n";
        }
        argvpos++;
    }

    return Options{ op, project_file, destination_directory, sf2_export, zip, lmms_dirs, lmms_exe };

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
        return Options{ op, argv[2] };
    }
    else
    {
        throw std::invalid_argument( "FATAL ERROR: invalid operation selection in this function: "
                                     + std::string( argv[1] ) );
    }
}

}
