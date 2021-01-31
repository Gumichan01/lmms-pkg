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
#include <experimental/filesystem>

#include "mmpz.hpp"
#include "../libzip/libzippp.h"
#include "../filesystem/filesystem.hpp"

using namespace libzippp;
namespace fsys = std::experimental::filesystem;

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

    ZipArchive zf(package_name);
    zf.open(ZipArchive::Write);

    if (zf.isOpen())
    {
        std::cout << "OK\n";
        for (auto &file : fsys::recursive_directory_iterator(package_directory))
        {
            if (fsys::is_regular_file(file.path()))
            {
                std::cout << "zip: " << file.path().string() << "\n";
                zf.addFile( file.path().string(), file.path().string() );
            }
            else
            {
                std::cout << file.path().string() << " is something else\n";
            }
        }
    }
    else
    {
        std::cerr << "-- ERROR: Cannot zip " << package_name << ".\n";
        return "";
    }
    zf.close();

    return package_name;
}

}
