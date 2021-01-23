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

const Options retrieveArguments( int argc, char * argv[] ) noexcept
{
    // Assuming there are at least 4 arguments
    const std::string& project_file = fs::normalize( argv[argc - 2] );

    std::string path = fs::normalize( argv[argc - 1] );
    if ( !path.empty() && path[path.size() - 1] != '/' )
    {
        path += '/';
    }

    const std::string& destination_directory = path;

    // Loop Between 2 and argc - 2 in order to get [--no-sf2] and [--lmms-data <path/to/lmms/data>]

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

    return Options{ op, project_file, destination_directory };
}

}
