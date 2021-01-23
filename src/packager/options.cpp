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

#include "options.hpp"
#include "../external/filesystem/filesystem.hpp"

namespace options
{

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept
{
    if ( !path.empty() && path[path.size() - 1] != '/' )
    {
        return path + '/';
    }
    return path;
}

const Options retrieveArguments( int argc, char * argv[] ) noexcept
{
    // Assuming there are at least 4 arguments
    const std::string& project_file = fs::normalize( argv[argc - 2] );
    const std::string& destination_directory = addTrailingSlashIfNeeded(fs::normalize( argv[argc - 1] ));

    bool sf2_export = true;
    std::string lmms_dir;

    /*
        argv[0]: program
        argv[1]: --export | --import
        argv[2 -> argc - 3]: optional parameters
        argv[argc - 2]: project file
        argv[argc - 1]: destination directory
    */
    int i = 2;
    while ( i < argc - 2 )
    {
        const std::string& opt = argv[i];
        if ( opt == "--no-sf2" )
        {
            std::cout << "-- Ignore Soundfont2 files\n";
            sf2_export = false;
        }
        else if ( ( opt == "--lmms-dir" ) )
        {
            if ( lmms_dir.empty() )
            {
                lmms_dir = addTrailingSlashIfNeeded(fs::normalize( argv[i + 1] ));
                std::cout << "-- An LMMS directory has been set: " << lmms_dir << "\n";
                i++;
            }
            else
            {
                std::cerr << "Warning: LMMS directory already set. Igoring directories specified are ignored \n";
            }
        }
        else
        {
            std::cerr << "Warning: Unkwown parameter: " << opt << ". It was ignored. \n";
        }
        i++;
    }

    const std::string& operation_str = argv[1];
    OperationType op;

    if ( operation_str == "--import" )
    {
        op = OperationType::Import;
    }
    else if ( operation_str == "--export" )
    {
        op = OperationType::Export;
    }

    return Options{ op, project_file, destination_directory, sf2_export, lmms_dir };
}

}
