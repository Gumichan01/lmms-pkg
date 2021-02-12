/*
*   LMMS Project Packager
*   Copyright © 2021 Luxon Jean-Pierre
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

#include <iostream>
#include <stdexcept>

#include "options.hpp"
#include "../external/filesystem/filesystem.hpp"


namespace options
{

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept;

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept
{
    if ( !path.empty() && path[path.size() - 1] != '/' )
    {
        return path + '/';
    }
    return path;
}


const Options retrieveImportExportArguments( const OperationType& op, int argc, char * argv[] );

const Options retrieveImportExportArguments( const OperationType& op, int argc, char * argv[] )
{
    const std::string& operation_str = argv[1];
    // Assuming there are at least 4 arguments
    const std::string& project_file = fs::normalize( argv[2] );
    const std::string& destination_directory = addTrailingSlashIfNeeded( fs::normalize( argv[3] ) );

    if ( op == OperationType::Import )
    {
        return Options { op, project_file, destination_directory };
    }

    // Assuming this is OperationType::Export
    bool sf2_export = true;
    bool zip = true;
    std::string lmms_dir;
    std::string lmms_exe = "lmms";
    bool lmms_exe_set = false;

    int argvpos = 4;
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
            if ( lmms_dir.empty() )
            {
                lmms_dir = addTrailingSlashIfNeeded( fs::normalize( argv[argvpos + 1] ) );
                std::cout << "-- An LMMS directory has been set: " << lmms_dir << "\n";
                argvpos++;
            }
            else
            {
                std::cout << "-- LMMS directory already set. Directory ignored.\n";
            }
        }
        else if ( ( opt == "--lmms-exe" ) )
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

    return Options{ op, project_file, destination_directory, sf2_export, zip, lmms_dir, lmms_exe };

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
const Options retrieveArguments( int argc, char * argv[] )
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
        return OperationType::InvalidOperation;
    } ();

    if ( op == OperationType::Import || op == OperationType::Export )
    {
        return retrieveImportExportArguments( op, argc, argv );
    }
    else if ( op == OperationType::Check )
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
