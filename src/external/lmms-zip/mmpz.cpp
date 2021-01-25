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

std::string decompressProject( const std::string& project_file, const std::string& package_directory,
                               const std::string& lmms_command ) noexcept
{
    const std::string& xml_file = package_directory + fs::basename( project_file.substr( 0, project_file.size() - 1 ) );
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

std::string zipFile( const std::string& package_directory ) noexcept
{
    if ( package_directory.empty() )
    {
        std::cerr << "-- ERROR: No directory provided.\n";
        return "";
    }

    std::string package_name;
    if ( package_directory[package_directory.size() - 1 ] == '/' )
    {
        package_name = package_directory.substr( 0, package_directory.size() - 1 ) + ".zip";
    }
    else
    {
        package_name = package_directory + ".zip";
    }

    const std::string& command = "zip -r " + package_name + " " + package_directory + " 2>&1";

    std::cout << "-- " << command << "\n";
    FILE * fpipe = ( FILE * )popen( command.c_str(), "r" );
    if ( !fpipe )
    {
        perror( "Something is wrong with LMMS" );
        return "";
    }

    const int BUFF_SZ = 1024;
    char buffer[BUFF_SZ] = { '\0' };
    while ( fgets( buffer, BUFF_SZ, fpipe ) != NULL )
    {
        std::cerr << buffer;
    }
    std::cout << "\n";
    pclose( fpipe );
    return package_name;
}

}
