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

#include "pack_priv.hpp"
#include "options.hpp"
#include "mmpz.hpp"
#include "xml.hpp"

#include "../exceptions/exceptions.hpp"

#include <iostream>
#include <algorithm>
#include <unordered_set>

using namespace exceptions;
namespace fsys = ghc::filesystem;

namespace Packager
{

/// Export

const std::vector<fsys::path> retrieveResourcesFromXmlFile( const std::string& xml_file )
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile( xml_file.c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        throw InvalidXmlFileException( "No root element. Are you sure this file contains an XML content?\n" );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<const tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames<const tinyxml2::XMLElement>( root, NAMES );

    std::unordered_set<std::string> unique_paths;
    std::for_each( elements.cbegin(), elements.cend(), [&unique_paths]( const tinyxml2::XMLElement * e )
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
                for ( const std::string& dir : options.lmms_directories )
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


/// Import


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
    tinyxml2::XMLDocument doc;
    doc.LoadFile( project_file.c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        /// At this point, this part must not be reachable
        throw PackageImportException( "The imported project file is invalid." );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames<tinyxml2::XMLElement>( root, NAMES );

    std::for_each( elements.cbegin(), elements.cend(), [&resources]( tinyxml2::XMLElement * e )
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

}
