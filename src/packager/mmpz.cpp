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
#include "../exceptions/exceptions.hpp"
#include "../external/filesystem/filesystem.hpp"
#include "../external/tinyxml2/tinyxml2.h"
#include "../external/zutils/zutils.hpp"

#include <iostream>
#include <memory>
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

    if ( ghc::filesystem::exists( xml_file ) )
    {
        throw AlreadyExistingFileException( "ERROR: \"" + xml_file +
                                            "\" Already exists. You need to export to a fresh directory.\n" );
    }

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

    const ghc::filesystem::path dir_parent = ghc::filesystem::absolute( package_directory ).parent_path().parent_path();
    for ( auto& file : ghc::filesystem::recursive_directory_iterator( package_directory ) )
    {
        const std::string& filename = ghc::filesystem::relative( ghc::filesystem::absolute( file.path() ), dir_parent ).string();
        std::cout << "zip: " << ghc::filesystem::normalize(filename) << "\n";

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
            std::cout << file.path().string() << " is something else. It is not zipped into the archive.\n";
        }
    }

    CloseZip( zip );
}

bool checkLMMSProjectBuffer(const std::unique_ptr<char []>& buffer, const unsigned int bufsize);

/**
    Structure of a valid LMMS project file:

    ```
    <?xml version="1.0"?>
    <!DOCTYPE lmms-project>
    <lmms-project creator="LMMS" version="1.0" creatorversion="1.2.2" type="song">
        <!-- Some data -->
        ...
        <!-- Some data -->
    </lmms-project>
    ```
    A valid project is a song project generated by a supported version of LMMS.
*/

bool checkLMMSProjectBuffer(const std::unique_ptr<char []>& buffer, const unsigned int bufsize)
{
    const char * ROOT_NAME = "lmms-project";
    const char * PROJECT_TYPE_NAME = "type";
    const char * PROJECT_TYPE_VALUE = "song";
    const char * VERSION_ATTRIBUTE = "creatorversion";
    const std::size_t VSIZE = 3;
    const std::array<std::string, VSIZE> VALID_VERSIONS{"1.2.0", "1.2.1", "1.2.2"};
    const char * VALID_VERSIONS_STR = "{ 1.2.0, 1.2.1, 1.2.2 }";

    bool valid_project = false;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError tinycode = doc.Parse( buffer.get(), bufsize );

    if ( tinycode == tinyxml2::XML_SUCCESS )
    {
        const tinyxml2::XMLElement * root = doc.RootElement();
        if ( root != nullptr )
        {
            const std::string root_name( root->Name() ? root->Name() : "" );
            if ( root_name == ROOT_NAME )
            {
                const char * type_attr_value = root->Attribute( PROJECT_TYPE_NAME );
                const std::string project_type( type_attr_value ? type_attr_value : "" );
                if ( project_type == PROJECT_TYPE_VALUE )
                {
                    const char * version_attr_value = root->Attribute( VERSION_ATTRIBUTE );
                    const std::string version( version_attr_value ? version_attr_value : "" );
                    if ( version.empty() || std::find(VALID_VERSIONS.cbegin(), VALID_VERSIONS.cend(), version) != VALID_VERSIONS.cend() )
                    {
                        valid_project = true;
                    }
                    else
                    {
                        std::cerr << "ERROR: This project was generated by a not supported version of LMMS: "
                                  << version << ". Only one of the following versions are supported: "
                                  << VALID_VERSIONS_STR << ".\n";
                    }
                }
                else
                {
                    std::cerr << "ERROR: Invalid project type. It must be a song, not '"<< project_type << "'.\n";
                }
            }
            else
            {
                std::cerr << "ERROR: Not an LMMS project.\n";
            }
        }
        else
        {
            std::cerr << "ERROR: This is not a valid LMMS project file.\n";
        }
    }
    else
    {
        std::cerr << "ERROR: Invalid XML file.\n";
    }

    return valid_project;
}

bool projectInfo(const std::unique_ptr<char []>& buffer, const unsigned int bufsize);

bool projectInfo(const std::unique_ptr<char []>& buffer, const unsigned int bufsize)
{
    const char * HEAD_NAME = "head";
    const char * ROOT_NAME = "lmms-project";
    const char * VERSION_ATTRIBUTE = "creatorversion";
    const char * PROJECT_VERSION_ATTRIBUTE = "version";
    const char * TIME_SIG_NUM_ATTRIBUTE = "timesig_numerator";
    const char * TIME_SIG_DEN_ATTRIBUTE = "timesig_denominator";
    const char * BPM_ATTRIBUTE = "bpm";

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError tinycode = doc.Parse( buffer.get(), bufsize );

    if ( tinycode == tinyxml2::XML_SUCCESS )
    {
        const tinyxml2::XMLElement * root = doc.RootElement();
        if ( root != nullptr )
        {
            const std::string root_name( root->Name() ? root->Name() : "" );
            if ( root_name == ROOT_NAME )
            {
                const char * version_attr_value = root->Attribute( VERSION_ATTRIBUTE );
                const char * project_version_attr_value = root->Attribute( PROJECT_VERSION_ATTRIBUTE );
                const std::string lmms_version( version_attr_value ? version_attr_value : "" );
                const std::string project_version( project_version_attr_value ? project_version_attr_value : "" );

                std::cout << "---- LMMS version: " << lmms_version << "\n";
                std::cout << "---- Project version: " << project_version << "\n";

                const tinyxml2::XMLElement * head = root->FirstChildElement( HEAD_NAME );
                if (head  != nullptr)
                {
                    const char * time_sig_num_attr_value = head->Attribute( TIME_SIG_NUM_ATTRIBUTE );
                    const char * time_sig_den_attr_value = head->Attribute( TIME_SIG_DEN_ATTRIBUTE );
                    const char * bpm_attr_value = head->Attribute( BPM_ATTRIBUTE );
                    const std::string bpm( bpm_attr_value ? bpm_attr_value : "" );
                    const std::string time_sig( std::string( time_sig_num_attr_value ? time_sig_num_attr_value : "" ) + "/" +
                                                ( time_sig_den_attr_value ? time_sig_den_attr_value : "" ) );

                    std::cout << "---- BPM: " << bpm << "\n";
                    std::cout << "---- Time Signature: " << time_sig << "\n";
                }
            }
            else
            {
                std::cerr << "ERROR: Not an LMMS project.\n";
                return false;
            }
        }
        else
        {
            std::cerr << "ERROR: Invalid XML file.\n";
            return false;
        }
    }
    else
    {
        std::cerr << "The project file is not a valid LMMS project file.\n";
        return false;
    }

    return true;
}

std::string zipFile( const ghc::filesystem::path& package_directory )
{
    const std::string& pkg_dir_txt = package_directory.string();
    const std::string& package_name = ( pkg_dir_txt.back() == '/' || pkg_dir_txt.back() == '\\' ) ?
                                      pkg_dir_txt.substr( 0, pkg_dir_txt.size() - 1 ) + PACKAGE_EXTENSION :
                                      pkg_dir_txt + PACKAGE_EXTENSION;

    compressPackage( pkg_dir_txt, package_name );
    return package_name;
}

const ghc::filesystem::path unzipFile( const ghc::filesystem::path& package, const ghc::filesystem::path& directory )
{
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

        std::cout << "-- Extract \"" << filename << "\".\n";
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

bool checkZipFile( const std::string& package_file )
{
    bool valid_project_file = false;
    bool has_resources_dir = false;

    if ( ghc::filesystem::exists( ghc::filesystem::path( package_file ) ) )
    {
        HZIP zip = OpenZip( package_file.c_str(), nullptr );

        ZIPENTRY ze;
        GetZipItem( zip, -1, &ze );
        const int numitems = ze.index;
        const std::string resources_dir("/resources/");

        if ( numitems <= 0 )
        {
            std::cerr << "ERROR: This package has no items.\n";
            return false;
        }

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
                    if ( checkLMMSProjectBuffer( buffer, BUFSIZE ) )
                    {
                        valid_project_file = true;
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

bool zipFileInfo( const std::string& package_file )
{
    const ghc::filesystem::path package( package_file );
    if ( ghc::filesystem::exists( ghc::filesystem::path( package ) ) )
    {
        if ( !ghc::filesystem::hasExtension( package, ".mmpk" ) )
        {
            std::cerr << "ERROR: This file has not the .mmpk extension.\n";
            return false;
        }

        HZIP zip = OpenZip( package_file.c_str(), nullptr );

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
                    if ( !projectInfo( buffer, BUFSIZE ) )
                    {
                        CloseZip( zip );
                        return false;
                    }
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
                filenames.push_back(filename);
            }
        }

        std::cout << "\n-- Files: \n";
        for (const std::string& filename : filenames)
        {
            std::cout << "---- " << filename << "\n";
        }
        std::cout << "-- Total: " << numitems << "\n";
        CloseZip( zip );
        return true;
    }
    return false;
}

}
