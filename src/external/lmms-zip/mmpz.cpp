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

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "mmpz.hpp"
#include "../filesystem/filesystem.hpp"


namespace lmms
{

std::string unzipProject( const std::string& project_file, const std::string& lmms_command ) noexcept
{
    const std::string& xml_file = project_file.substr( 0, project_file.size() - 1 );
    const std::string& command = lmms_command + " -d " + project_file + " > " + xml_file;

    std::cout << "-- " << command << "\n";
    FILE * fpipe = ( FILE * )popen( command.c_str(), "r" );
    if ( !fpipe )
    {
        perror( "Something is wrong with LMMS" );
        return "";
    }
    pclose( fpipe );

    if ( !fs::exists( xml_file ) )
    {
        std::cerr << "-- ERROR: No file extracted. Something was wrong with the command or the file.\n";
        return "";
    }

    return xml_file;
}

}
