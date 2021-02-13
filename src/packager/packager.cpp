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
#include <algorithm>
#include <unordered_set>

#include "packager.hpp"
#include "options.hpp"
#include "mmpz.hpp"
#include "../exceptions/exceptions.hpp"
#include "../external/tinyxml2/tinyxml2.h"
#include "../external/filesystem/filesystem.hpp"

using namespace exceptions;

namespace Packager
{

// Private functions

bool contains( const std::vector<std::string> names, const std::string& s );
const std::vector<const tinyxml2::XMLElement *> getAllElementsByNames( const tinyxml2::XMLElement * root, const std::vector<std::string>& names );
bool isXmlFile( const std::string& project_file ) noexcept;

const std::vector<std::string> retrievePathsOfFilesFromXMLFile( const std::string& project_file );
const std::vector<std::string> copyFilesTo( const std::vector<std::string>& paths, const std::string& directory, const options::Options& options );


bool contains( const std::vector<std::string> names, const std::string& s )
{
    return std::find( names.cbegin(), names.cend(), s ) != names.cend();
}

const std::vector<const tinyxml2::XMLElement *> getAllElementsByNames( const tinyxml2::XMLElement * root, const std::vector<std::string>& names )
{
    std::vector<const tinyxml2::XMLElement *> retrieved_elements;
    const tinyxml2::XMLElement * element = root->FirstChildElement();

    while ( element != nullptr )
    {
        if ( contains( names, element->Name() ) )
        {
            retrieved_elements.push_back( element );
        }
        const std::vector<const tinyxml2::XMLElement *>& elements = getAllElementsByNames( element, names );
        retrieved_elements.insert( retrieved_elements.end(), elements.cbegin(), elements.cend() );
        element = element->NextSiblingElement();
    }

    return retrieved_elements;
}

bool isXmlFile( const std::string& project_file ) noexcept
{
    return tinyxml2::XMLDocument().LoadFile( project_file.c_str() ) == tinyxml2::XML_SUCCESS;
}


const std::vector<std::string> retrievePathsOfFilesFromXMLFile( const std::string& project_file )
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError code = doc.LoadFile( project_file.c_str() );

    if ( code != tinyxml2::XML_SUCCESS )
    {
        std::cerr << "Ouch! Tinyxml Error code: " << code << "\n";
        return std::vector<std::string>();
    }

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        std::cerr << "No root element. Are you sure this file contains an XML content?\n";
        return std::vector<std::string>();
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<const tinyxml2::XMLElement *>& elements = getAllElementsByNames( root, NAMES );

    std::unordered_set<std::string> unique_paths;
    std::for_each( elements.cbegin(), elements.cend(), [&unique_paths]( const tinyxml2::XMLElement * e )
    {
        unique_paths.insert( e->Attribute( "src" ) );
    } );

    return std::vector<std::string>( unique_paths.cbegin(), unique_paths.cend() );
}

const std::vector<std::string> copyFilesTo( const std::vector<std::string>& paths, const std::string& directory, const options::Options& options )
{
    std::vector<std::string> copied_files;
    std::for_each( paths.cbegin(), paths.cend(),
                   [&directory, &copied_files, &options]( const std::string & source_path )
    {
        if ( fs::hasExtension( source_path, ".sf2" ) && !options.sf2_export )
        {
            std::cout << "-- This following file: \"" << source_path << "\" is a SoundFont file and is ignored.\n";
        }
        else
        {
            const std::string& destination_path = directory + fs::basename( source_path );
            if ( fs::exists( source_path ) )
            {
                std::cout << "-- Copying \"" << source_path << "\" -> \"" << destination_path << "\"...";
                if ( fs::copyFile( source_path, destination_path ) )
                {
                    std::cout << "DONE\n";
                    copied_files.push_back( destination_path );
                }
                else
                {
                    std::cout << "FAILED\n";
                }
            }
            else
            {
                if ( !options.lmms_directory.empty() )
                {
                    std::cout << "-- " << source_path << "\" does not exit.\n";

                    // Assuming the source_path is relative to the current directory the program is launched
                    const std::string& lmms_source_file = options.lmms_directory + source_path;
                    std::cout << "-- Trying \"" << lmms_source_file << "\"\n";

                    if ( fs::exists( lmms_source_file ) )
                    {
                        if ( fs::copyFile( lmms_source_file, destination_path ) )
                        {
                            std::cout << "DONE\n";
                            copied_files.push_back( destination_path );
                        }
                        else
                        {
                            std::cout << "FAILED\n";
                        }
                    }
                    else
                    {
                        std::cerr << "-- Cannot get \"" << lmms_source_file << "\"\n";
                    }
                }
                else
                {
                    std::cerr << "-- Cannot get \"" << source_path << "\" \n";
                }
            }
        }
    } );
    return copied_files;
}


// Public

const std::string pack( const options::Options& options )
{
    const std::string& lmms_file = options.project_file;
    const std::string& package_directory = options.destination_directory;
    const std::string& sample_directory = package_directory + "samples/";

    if ( !fs::exists( lmms_file ) )
    {
        throw NonExistingFileException( "ERROR: \"" + lmms_file + "\" does not exist.\n" );
    }

    if ( !fs::createDir( package_directory ) )
    {
        throw DirectoryCreationException( "ERROR: \"" + package_directory + "\" cannot be created.\n" );
    }
    else if ( !fs::createDir( sample_directory ) )
    {
        throw DirectoryCreationException( "ERROR: \"" + sample_directory + "\" cannot be created.\n" );
    }

    std::string project_file = package_directory + fs::basename( lmms_file );
    if ( fs::hasExtension ( lmms_file, ".mmpz" ) )
    {
        project_file = lmms::decompressProject( lmms_file, package_directory, options.lmms_command );
    }
    else
    {
        std::cout << "-- Copying \"" << lmms_file << "\" -> \"" << project_file << "\"...";
        if ( fs::copyFile( lmms_file, project_file ) )
        {
            std::cout << "DONE\n";
        }
        else
        {
            std::cout << "FAILED\n";
        }
    }

    if ( !fs::exists( project_file ) )
    {
        throw NonExistingFileException( "ERROR: \"" + project_file + "\" does not exist. Packaging aborted.\n" );
    }

    if ( !isXmlFile( project_file ) )
    {
        throw InvalidXmlFileException( "ERROR: Invalid XML file: \"" + project_file + "\". Packaging aborted.\n" );
    }

    const std::vector<std::string>& files = retrievePathsOfFilesFromXMLFile( project_file );
    std::cout << "\n-- This project has " << files.size() << " files to copy.\n\n";
    const std::vector<std::string>& copied_files = Packager::copyFilesTo( files, sample_directory, options );
    std::cout << "-- " << copied_files.size() << " file(s) copied.\n\n";

    // Extra verification
    bool non_existing_files = std::any_of( copied_files.cbegin(), copied_files.cend(), [] ( const std::string & file )
    {
        return !fs::exists( file );
    } );

    if ( non_existing_files )
    {
        throw NonExistingFileException( "ERROR: Some files that must be copied in the package does not exist.\n" );
    }

    return options.zip ? lmms::zipFile( package_directory ) : package_directory;
}


bool checkPackage( const options::Options& options )
{
    return lmms::checkZipFile( options.project_file );
}

}
