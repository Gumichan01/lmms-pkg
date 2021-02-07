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
#include <memory>

#include "mmpz.hpp"
#include "../external/filesystem/filesystem.hpp"
#include "../external/ghc/filesystem.hpp"
#include "../external/tinyxml2/tinyxml2.h"

#if defined(__WIN32__) || defined(__WIN64__)

#include "../external/zutils-win32/zutils.hpp"

#else

#include "../libzip/libzippp.h"
using namespace libzippp;

#endif

namespace fsys = ghc::filesystem;

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

bool compressPackage( const std::string& package_directory, const std::string& package_name ) noexcept;

bool compressPackage( const std::string& package_directory, const std::string& package_name ) noexcept
{

// I could not compile this fucking libzippp on Windows because of those damned dependencies.
// So I use Zip Utils for Windows
// I don't want to waste my time installing VStudio, I am a FLOSS advocate ( -.-).
#if defined(__WIN32__) || defined(__WIN64__)

    HZIP zip = CreateZip( package_name.c_str(), nullptr );

    for ( auto& file : fsys::recursive_directory_iterator( package_directory ) )
    {
        if ( fsys::is_regular_file( file.path() ) )
        {
            std::cout << "zip: " << file.path().string() << "\n";
            ZipAdd( zip, file.path().string().c_str(), file.path().string().c_str() );
        }
        else
        {
            std::cout << file.path().string() << " is something else\n";
        }
    }

    CloseZip( zip );
#else

    ZipArchive zf( package_name );
    zf.open( ZipArchive::Write );

    if ( zf.isOpen() )
    {
        for ( auto& file : fsys::recursive_directory_iterator( package_directory ) )
        {
            if ( fsys::is_regular_file( file.path() ) )
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
        return false;
    }
    zf.close();

#endif
    return true;
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

    return compressPackage( package_directory, package_name ) ? package_name : "";
}

bool checkZipFile( const std::string& package_file ) noexcept
{
    // 4. Check if the sample directory exists
    if ( fsys::exists( fsys::path( package_file ) ) )
    {
// #if defined(__WIN32__) || defined(__WIN64__)

        HZIP zip = OpenZip( package_file.c_str(), nullptr );

        ZIPENTRY ze;
        GetZipItem( zip, -1, &ze );
        int numitems = ze.index;

        for ( int index = 0; index < numitems; index++ )
        {
            ZIPENTRY entry;
            GetZipItem( zip, index, &entry );
            const std::string& filename = entry.name;

            std::cout << "-- " << filename << "\n";

            if ( fs::hasExtension( filename, ".mmp" ) )
            {
                const unsigned int bufsize = entry.unc_size + 1;
                const std::unique_ptr<char []> buffer = std::make_unique<char []>( bufsize );
                int code = UnzipItem ( zip, index, buffer.get(), bufsize );

                if ( code == ZR_OK )
                {
                    tinyxml2::XMLError tinycode = tinyxml2::XMLDocument().Parse( buffer.get(), bufsize );

                    if ( tinycode != tinyxml2::XML_SUCCESS )
                    {
                        std::cerr << "The project file is not a valid LMMS project file.\n";
                        CloseZip( zip );
                        return false;
                    }
                }
                else
                {
                    /// This block must be unreachable
                    std::cerr << "Internal error while unzipping the project file. Please contact a developer.\n";
                }
            }
        }

        CloseZip( zip );
// #else
        return true;
    }
    return false;
}

}
