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

#include "packager.hpp"
#include "options.hpp"
#include "mmpz.hpp"
#include "xml.hpp"
#include "../exceptions/exceptions.hpp"
#include "../external/filesystem/filesystem.hpp"

#include <iostream>
#include <algorithm>
#include <unordered_set>


using namespace exceptions;
namespace txml = tinyxml2;
namespace fsys = ghc::filesystem;

namespace Packager
{

// Private functions

const std::vector<fsys::path> retrieveResourcesFromXmlFile( const std::string& xml_file );
const std::vector<fsys::path> copyFilesTo( const std::vector<fsys::path>& paths, const fsys::path& directory,
                                           const options::Options & options );
fsys::path generateProjectFileInPackage( const fsys::path& lmms_file, const options::Options& options );


const std::vector<fsys::path> retrieveResourcesFromXmlFile( const std::string& xml_file )
{
    txml::XMLDocument doc;
    doc.LoadFile( xml_file.c_str() );

    const txml::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        throw InvalidXmlFileException( "No root element. Are you sure this file contains an XML content?\n" );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<const txml::XMLElement *>& elements = xml::getAllElementsByNames<const txml::XMLElement>( root, NAMES );

    std::unordered_set<std::string> unique_paths;
    std::for_each( elements.cbegin(), elements.cend(), [&unique_paths]( const txml::XMLElement * e )
    {
        unique_paths.insert( e->Attribute( "src" ) );
    } );

    std::vector<fsys::path> paths;
    std::for_each( unique_paths.cbegin(), unique_paths.cend(), [&paths]( const std::string & s )
    {
        paths.push_back( fsys::path( s ) );
    } );

    return paths;
}

const std::vector<fsys::path> copyFilesTo( const std::vector<fsys::path>& paths, const fsys::path& directory,
                                           const options::Options& options )
{
    std::vector<fsys::path> copied_files;
    std::for_each( paths.cbegin(), paths.cend(),
                   [&directory, &copied_files, &options]( const fsys::path & source_path )
    {
        if ( fsys::hasExtension( source_path, ".sf2" ) && !options.sf2_export )
        {
            std::cout << "-- This following file: \"" << source_path.string() << "\" is a SoundFont file and is ignored.\n";
        }
        else
        {
            const fsys::path& destination_path = fsys::path( directory.string() + source_path.filename().string() );
            if ( fsys::exists( source_path ) )
            {
                std::cout << "-- Copying \"" << source_path.string() << "\" -> \"" << destination_path.string() << "\"...";
                fsys::copy_file( source_path, destination_path );
                copied_files.push_back( destination_path );
                std::cout << "DONE\n";
            }
            else
            {
                for (const std::string& dir : options.lmms_directories)
                {
                    // Assuming the source_path is relative to the current directory the program is launched
                    const fsys::path& lmms_source_file = fsys::path( dir + source_path.string() );
                    std::cout << "-- Trying \"" << lmms_source_file.string() << "\"\n";

                    if ( fsys::exists( lmms_source_file ) )
                    {
                        std::cout << "-- Copying \"" << lmms_source_file.string() << "\" -> \""
                                  << destination_path.string() << "\"...";
                        fsys::copy_file( lmms_source_file, destination_path );
                        copied_files.push_back( destination_path );
                        std::cout << "DONE\n";
                        break;
                    }
                    else
                    {
                        std::cerr << "-- Cannot get \"" << lmms_source_file.string() << "\"\n";
                    }
                }
            }
        }
    } );
    return copied_files;
}

fsys::path generateProjectFileInPackage( const fsys::path& lmms_file, const options::Options& options )
{
    const std::string& project_file = options.project_file;
    const std::string& destination_directory = options.destination_directory;

    if ( fsys::hasExtension ( lmms_file, ".mmpz" ) )
    {
        return lmms::decompressProject( project_file, destination_directory, options.lmms_command );
    }
    else
    {
        const fsys::path filepath( destination_directory + lmms_file.filename().string() );

        if ( fsys::exists( filepath ) )
        {
            throw AlreadyExistingFileException( "ERROR: \"" + filepath.string() +
                                                "\" Already exists. You need to export to a fresh directory.\n" );
        }

        std::cout << "-- Copying \"" << lmms_file.string() << "\" -> \"" << filepath.string() << "\"...";
        fsys::copy_file( lmms_file, filepath );
        std::cout << "DONE\n";
        return filepath;
    }
}


const std::vector<fsys::path> getProjectResourcePaths( const fsys::path& project_directory );
void configureProject( const fsys::path& project_file, const std::vector<fsys::path>& resources );

const std::vector<fsys::path> getProjectResourcePaths( const fsys::path& project_directory )
{
    std::vector<fsys::path> paths;
    for ( auto& file : fsys::recursive_directory_iterator( project_directory ) )
    {
        const fsys::path& filepath = file.path();
        if ( fsys::is_regular_file( filepath ) )
        {
            paths.push_back( filepath );
        }
    }
    return paths;
}

void configureProject( const fsys::path& project_file, const std::vector<fsys::path>& resources )
{
    txml::XMLDocument doc;
    doc.LoadFile( project_file.c_str() );

    const txml::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        /// At this point, this part must not be reachable
        throw PackageImportException( "The imported project file is invalid." );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<txml::XMLElement *>& elements = xml::getAllElementsByNames<txml::XMLElement>( root, NAMES );

    std::for_each( elements.cbegin(), elements.cend(), [&resources]( txml::XMLElement * e )
    {
        const std::string source( e->Attribute( "src" ) );
        const std::string& filename = fsys::path( source ).filename().string();
        auto found = std::find_if( resources.cbegin(), resources.cend(), [&filename] ( const fsys::path & resource )
        {
            return resource.filename().string() == filename;
        } );

        if ( found != resources.cend() )
        {
            std::cout << "-- Configure \"" << e->Name() << "\" with \"" << filename << "\" in project \n";
            const std::string& resource_found = fsys::absolute( ( *found ) );
            std::cout << "-- Set attribute \"src\" to \"" << resource_found << "\" \n";
            e->SetAttribute( "src", resource_found.c_str() );
        }
    } );

    tinyxml2::XMLError code = doc.SaveFile( project_file.c_str() );
    if ( code != tinyxml2::XMLError::XML_SUCCESS )
    {
        throw PackageImportException( "ERROR: Import failed : cannot save updated configuration into the project" +
                                      std::string( doc.ErrorStr() ) );
    }
}

// Public

const std::string pack( const options::Options& options )
{
    const std::string& project_file = options.project_file;
    const std::string& destination_directory = options.destination_directory;

    const fsys::path lmms_file( project_file );
    const fsys::path package_directory( destination_directory );

    if ( !fsys::exists( lmms_file ) )
    {
        throw NonExistingFileException( "ERROR: \"" + lmms_file.string() + "\" does not exist.\n" );
    }

    if ( !fsys::exists( package_directory ) )
    {
        fsys::create_directories( package_directory );
    }

    const fsys::path& project_filepath = generateProjectFileInPackage( lmms_file, options );
    if ( !fsys::exists( project_filepath ) )
    {
        throw NonExistingFileException( "ERROR: \"" + project_filepath.string() + "\" does not exist. Packaging aborted.\n" );
    }

    if ( !xml::isXmlFile( project_filepath ) )
    {
        throw InvalidXmlFileException( "ERROR: Invalid XML file: \"" + project_filepath.string() + "\". Packaging aborted.\n" );
    }

    const std::vector<fsys::path>& files = retrieveResourcesFromXmlFile( project_filepath.string() );
    std::cout << "\n-- This project has " << files.size() << " file(s) to copy.\n\n";

    if ( !files.empty() )
    {
        const fsys::path sample_directory( destination_directory + "resources/" );
        if ( !fsys::exists( sample_directory ) )
        {
            fsys::create_directories( sample_directory );
        }

        const std::vector<fsys::path>& copied_files = Packager::copyFilesTo( files, sample_directory.string(), options );
        std::cout << "-- " << copied_files.size() << " file(s) copied.\n\n";
        return options.zip ? lmms::zipFile( package_directory ) : package_directory.string();
    }
    else
    {
        std::cout << "-- \"" << project_filepath.filename().string() << "\" has no external sample or soundfont file to export.\n"
                  << "-- So it does not make sense to export this project.\n"
                  << "-- No package file will be generated, but the generated directory containing the project file is created: \""
                  << package_directory.string() + "\".\n";
        return package_directory.string();
    }
}


const std::string unpack( const options::Options& options )
{
    const fsys::path package( options.project_file );
    const fsys::path destination_directory( options.destination_directory );

    if ( !fsys::exists( package ) )
    {
        throw NonExistingFileException( "ERROR: \"" + package.string() + "\" does not exist.\n" );
    }

    if ( lmms::checkZipFile( package.string() ) )
    {
        std::cout << "Package is OK.\n\n";
        if ( !fsys::exists( destination_directory ) )
        {
            fsys::create_directories( destination_directory );
        }

        const fsys::path project_file( lmms::unzipFile( package, destination_directory ) );
        std::cout << "Package extracted into \"" << destination_directory.string() << "\".\n";

        const fsys::path backup_file( project_file.native() + ".backup" );
        fsys::copy( project_file, backup_file );
        std::cout << "Backup file created: \"" << backup_file.string() << "\"\n\n";

        configureProject( project_file, getProjectResourcePaths( destination_directory ) );
    }
    else
    {
        throw PackageImportException( "ERROR: Cannot import \"" + package.string() + "\": invalid package.\n" );
    }

    return destination_directory.string();
}

bool checkPackage( const options::Options& options )
{
    return lmms::checkZipFile( options.project_file );
}

}
