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

#include "mmpz.hpp"
#include "xml.hpp"
#include "../program/printer.hpp"
#include "../exceptions/exceptions.hpp"
#include "../external/filesystem/filesystem.hpp"
#include "../external/zutils/zutils.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <system_error>

using namespace exceptions;

namespace lmms
{
const std::string PACKAGE_EXTENSION( ".mmpk" );

ghc::filesystem::path decompressProject( const std::string& project_file,
                                         const std::string& package_directory,
                                         const std::string& lmms_command )
{
    // Assuming the name of the project file has ".mmpz" as an extension
    const std::string& basename = ghc::filesystem::path( project_file ).filename().string();
    const std::string& xml_file = package_directory + basename.substr( 0, basename.size() - 1 );
    const std::string& command = lmms_command + " -d " + project_file + " > " + xml_file;
    program::log::Printer print = program::log::getPrinter();

    if ( ghc::filesystem::exists( xml_file ) )
    {
        throw AlreadyExistingFileException( "ERROR: \"" + xml_file +
                                            "\" Already exists. You need to export to a fresh directory.\n" );
    }

    print << "-- " << command << "\n";
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
    const ghc::filesystem::path dir_parent = ghc::filesystem::absolute( package_directory ).parent_path().parent_path();
    HZIP zip = CreateZip( package_name.c_str(), nullptr );
    program::log::Printer print = program::log::getPrinter();

    for ( const auto& file : ghc::filesystem::recursive_directory_iterator( package_directory ) )
    {
        const std::string& filename = ghc::filesystem::relative( ghc::filesystem::absolute( file.path() ), dir_parent ).string();
        print << "zip: " << ghc::filesystem::normalize( filename ) << "\n";

        if ( ghc::filesystem::is_regular_file( file.path() ) )
        {
            ZipAdd( zip, filename.c_str(), file.path().string().c_str() );
        }
        else if ( ghc::filesystem::is_directory( file.path() ) )
        {
            ZipAddFolder( zip, filename.c_str() );
        }
        else
        {
            std::cerr << file.path().string() << " is something else. It is not zipped into the archive.\n";
        }
    }

    CloseZip( zip );
}


bool checkLMMSProjectFile( const ghc::filesystem::path& lmms_file )
{
    std::ifstream infile( lmms_file.string() );
    if ( infile )
    {
        std::stringstream ss;
        ss << infile.rdbuf();
        infile.close();

        const unsigned int BUFSIZE = 4194304; // 4 Mio, that should be enough to cover most project files
        const std::unique_ptr<char []> buffer = std::make_unique<char []>( BUFSIZE );
        ss.read( buffer.get(), BUFSIZE );
        return xml::checkLMMSProjectBuffer( buffer, BUFSIZE );
    }
    return false;
}


const ghc::filesystem::path zipFile( const ghc::filesystem::path& package_directory )
{
    const std::string& pkg_dir_txt = package_directory.string();
    const std::string& package_name = ( pkg_dir_txt.back() == '/' || pkg_dir_txt.back() == '\\' ) ?
                                        pkg_dir_txt.substr( 0, pkg_dir_txt.size() - 1 ) + PACKAGE_EXTENSION :
                                        pkg_dir_txt + PACKAGE_EXTENSION;

    compressPackage( pkg_dir_txt, package_name );
    return ghc::filesystem::path( package_name );
}

const ghc::filesystem::path unzipFile( const ghc::filesystem::path& package, const ghc::filesystem::path& directory )
{
    program::log::Printer print = program::log::getPrinter();
    HZIP zip = OpenZip( package.string().c_str(), nullptr );
    ZIPENTRY ze;
    GetZipItem( zip, -1, &ze );

    const int numitems = ze.index;
    ghc::filesystem::path project_path( directory );

    for ( int index = 0; index < numitems; index++ )
    {
        ZIPENTRY entry;
        GetZipItem( zip, index, &entry );
        const std::string& filename = entry.name;

        print << "-- Extract \"" << filename << "\".\n";
        const int code = UnzipItem ( zip, index, filename.c_str() );
        if ( code != ZR_OK )
        {
            CloseZip( zip );
            throw PackageImportException( "ERROR: Cannot unzip " + filename + ".\n" );
        }

        if ( ghc::filesystem::hasExtension( ghc::filesystem::path( filename ), ".mmp" ) )
        {
            project_path /= ghc::filesystem::path( filename );
        }
    }

    CloseZip( zip );

    try
    {
        std::error_code ec;
        auto option = ghc::filesystem::copy_options::recursive;
        const ghc::filesystem::path target_path( directory.string() + "/" + package.stem().string() );
        ghc::filesystem::copy( package.stem(), target_path, option );
        ghc::filesystem::remove_all( package.stem(), ec );
    }
    catch ( ghc::filesystem::filesystem_error& e )
    {
        std::error_code ec;
        ghc::filesystem::remove_all( package.stem(), ec );
        throw PackageImportException( "ERROR: " + std::string( e.what() ) );
    }

    return project_path;
}

bool checkZipFile( const ghc::filesystem::path& package_file )
{
    bool valid_project_file = false;
    bool has_resources_dir = false;
    program::log::Printer print = program::log::getPrinter();

    if ( ghc::filesystem::exists( package_file ) )
    {
        HZIP zip = OpenZip( package_file.string().c_str(), nullptr );

        ZIPENTRY ze;
        GetZipItem( zip, -1, &ze );
        const int numitems = ze.index;
        const std::string resources_dir("/resources/");

        if ( numitems <= 0 )
        {
            std::cerr << "ERROR: This package has no items.\n";
            return false;
        }

        print << "-- " << numitems << " item(s).\n";

        for ( int index = 0; index < numitems; index++ )
        {
            ZIPENTRY entry;
            GetZipItem( zip, index, &entry );
            const std::string filename( entry.name );

            if ( ghc::filesystem::hasExtension( ghc::filesystem::path( filename ), ".mmp" ) )
            {
                const unsigned int BUFSIZE = 4194304; // 4 Mio, that should be enough to cover most project files
                const std::unique_ptr<char []> buffer = std::make_unique<char []>( BUFSIZE );

                int code = UnzipItem ( zip, index, buffer.get(), BUFSIZE );
                if ( code == ZR_OK )
                {
                    print << "-- Checking project file...\n";
                    if ( xml::checkLMMSProjectBuffer( buffer, BUFSIZE ) )
                    {
                        valid_project_file = true;
                        print << "-- Project file OK\n";
                        print << "*  " << filename << " OK\n";
                    }
                }
                else
                {
                    /// This block must be unreachable
                    CloseZip( zip );
                    throw std::runtime_error( "Internal error while unzipping the project file. Please contact a developer.\n" );
                }
            }
            else if ( filename.size() >= resources_dir.size() &&
                      filename.substr( filename.size() - resources_dir.size(), resources_dir.size() ) == resources_dir )
            {
                has_resources_dir = true;
                print << "*  " << filename << " OK\n";
            }
            else
            {
                print << "*  " << filename << " OK\n";
            }
        }

        CloseZip( zip );

        if ( !has_resources_dir )
        {
            std::cerr << "ERROR: No resource directory.\n";
        }

        if ( !valid_project_file )
        {
            std::cerr << "ERROR: No project file.\n";
        }

        return valid_project_file && has_resources_dir;
    }
    return false;
}

bool zipFileInfo( const ghc::filesystem::path& package_file )
{
    program::log::Printer print = program::log::getPrinter();

    if ( ghc::filesystem::exists( package_file ) )
    {
        if ( !ghc::filesystem::hasExtension( package_file, ".mmpk" ) )
        {
            std::cerr << "ERROR: This file has not the .mmpk extension.\n";
            return false;
        }

        HZIP zip = OpenZip( package_file.string().c_str(), nullptr );

        ZIPENTRY ze;
        GetZipItem( zip, -1, &ze );
        const int numitems = ze.index;

        if ( numitems <= 0 )
        {
            std::cerr << "ERROR: This package has no items.\n";
            return false;
        }

        std::vector<std::string> filenames;
        for ( int index = 0; index < numitems; index++ )
        {
            ZIPENTRY entry;
            GetZipItem( zip, index, &entry );
            const std::string filename( entry.name );

            if ( ghc::filesystem::hasExtension( ghc::filesystem::path( filename ), ".mmp" ) )
            {
                const unsigned int BUFSIZE = 4194304; // 4 Mio, that should be enough to cover most project files
                const std::unique_ptr<char []> buffer = std::make_unique<char []>( BUFSIZE );

                int code = UnzipItem ( zip, index, buffer.get(), BUFSIZE );
                if ( code == ZR_OK )
                {
                    std::cout << "-- Project: " << ghc::filesystem::path( filename ).filename().string() << "\n";
                    if ( !xml::projectInfo( buffer, BUFSIZE ) )
                    {
                        CloseZip( zip );
                        return false;
                    }
                    filenames.push_back( filename );
                }
                else
                {
                    /// This block must be unreachable
                    CloseZip( zip );
                    throw std::runtime_error( "Internal error while unzipping the project file. Please contact a developer.\n" );
                }
            }
            else
            {
                filenames.push_back( filename );
            }
        }

        print << "\n-- Files: \n";
        for (const std::string& filename : filenames)
        {
            print << "---- " << filename << "\n";
        }

        // The package must have at least two files: the project file and the resources/ directory
        print << "-- Total:\n"
              << "---- " << numitems << " items in the zip file.\n"
              << "---- " << ( numitems >= 2 ? filenames.size() - 2 : filenames.size() )
              << " audio file(s).\n";
        CloseZip( zip );
        return true;
    }
    return false;
}

}
