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
#include <system_error>

#include "mmpz.hpp"
#include "../exceptions/exceptions.hpp"
#include "../external/filesystem/filesystem.hpp"
#include "../external/tinyxml2/tinyxml2.h"
#include "../external/zutils/zutils.hpp"

using namespace exceptions;

namespace lmms
{


ghc::filesystem::path decompressProject( const std::string& project_file,
                                         const std::string& package_directory,
                                         const std::string& lmms_command )
{
    // Assuming the name of the project file has ".mmpz" as an extension
    const std::string& basename = ghc::filesystem::path( project_file ).filename();
    const std::string& xml_file = package_directory + basename.substr( 0, basename.size() - 1 );
    const std::string& command = lmms_command + " -d " + project_file + " > " + xml_file;

    std::cout << "-- " << command << "\n";
    FILE * fpipe = ( FILE * )popen( command.c_str(), "r" );
    if ( !fpipe )
    {
        throw std::system_error( errno, std::system_category(), "Something is wrong with LMMS" );
    }

    pclose( fpipe );
    return ghc::filesystem::path( xml_file );
}

void compressPackage( const std::string& package_directory, const std::string& package_name );

void compressPackage( const std::string& package_directory, const std::string& package_name )
{
    HZIP zip = CreateZip( package_name.c_str(), nullptr );

    for ( auto& file : ghc::filesystem::recursive_directory_iterator( package_directory ) )
    {
        std::cout << "zip: " << file.path().string() << "\n";

        if ( ghc::filesystem::is_regular_file( file.path() ) )
        {
            ZipAdd( zip, file.path().string().c_str(), file.path().string().c_str() );
        }
        else if ( ghc::filesystem::is_directory( file.path() ) )
        {
            ZipAddFolder( zip, file.path().string().c_str() );
        }
        else
        {
            std::cout << file.path().string() << " is something else. It is not zipped into the archive.\n";
        }
    }

    CloseZip( zip );
}

std::string zipFile( const ghc::filesystem::path& package_directory )
{
    const std::string& pkg_dir_txt = package_directory.string();
    const std::string& package_name = ( pkg_dir_txt.back() == '/' || pkg_dir_txt.back() == '\\' ) ?
                                      pkg_dir_txt.substr( 0, pkg_dir_txt.size() - 1 ) + ".zip" :
                                      pkg_dir_txt + ".zip";

    compressPackage( package_directory, package_name );
    return package_name;
}

bool checkZipFile( const std::string& package_file )
{
    bool valid_project_file = false;
    bool has_sample_directory = false;

    if ( ghc::filesystem::exists( ghc::filesystem::path( package_file ) ) )
    {
        HZIP zip = OpenZip( package_file.c_str(), nullptr );

        ZIPENTRY ze;
        GetZipItem( zip, -1, &ze );
        int numitems = ze.index;
        const std::string& samples_dir = "/samples/";

        for ( int index = 0; index < numitems; index++ )
        {
            ZIPENTRY entry;
            GetZipItem( zip, index, &entry );
            const std::string& filename = entry.name;

            std::cout << "-- " << filename << "\n";

            if ( ghc::filesystem::hasExtension( ghc::filesystem::path(filename), ".mmp" ) )
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
                    else
                    {
                        valid_project_file = true;
                    }
                }
                else
                {
                    /// This block must be unreachable
                    throw std::runtime_error( "Internal error while unzipping the project file. Please contact a developer.\n" );
                }
            }
            else if ( filename.substr( filename.size() - samples_dir.size(), samples_dir.size() ) == samples_dir )
            {
                has_sample_directory = true;
            }
        }

        CloseZip( zip );

        if ( !has_sample_directory )
        {
            std::cerr << "The sample directory is not here.\n";
        }

        return valid_project_file && has_sample_directory;
    }
    return false;
}

}
