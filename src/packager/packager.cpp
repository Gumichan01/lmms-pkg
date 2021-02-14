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

namespace Packager
{

// Private functions

const std::vector<ghc::filesystem::path> retrievePathsOfFilesFromXMLFile( const std::string& xml_file );
const std::vector<ghc::filesystem::path> copyFilesTo( const std::vector<ghc::filesystem::path>& paths,
                                                      const ghc::filesystem::path& directory,
                                                      const options::Options& options );
ghc::filesystem::path generateProjectFileInPackage( const ghc::filesystem::path& lmms_file, const options::Options& options );


const std::vector<ghc::filesystem::path> retrievePathsOfFilesFromXMLFile( const std::string& xml_file )
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile( xml_file.c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        throw InvalidXmlFileException( "No root element. Are you sure this file contains an XML content?\n" );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<const tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames( root, NAMES );

    std::unordered_set<std::string> unique_paths;
    std::for_each( elements.cbegin(), elements.cend(), [&unique_paths]( const tinyxml2::XMLElement * e )
    {
        unique_paths.insert( e->Attribute( "src" ) );
    } );

    std::vector<ghc::filesystem::path> paths;
    std::for_each( unique_paths.cbegin(), unique_paths.cend(), [&paths]( const std::string & s )
    {
        paths.push_back( ghc::filesystem::path( s ) );
    } );

    return paths;
}

const std::vector<ghc::filesystem::path> copyFilesTo( const std::vector<ghc::filesystem::path>& paths,
        const ghc::filesystem::path& directory,
        const options::Options& options )
{
    std::vector<ghc::filesystem::path> copied_files;
    std::for_each( paths.cbegin(), paths.cend(),
                   [&directory, &copied_files, &options]( const ghc::filesystem::path & source_path )
    {
        if ( ghc::filesystem::hasExtension( source_path, ".sf2" ) && !options.sf2_export )
        {
            std::cout << "-- This following file: \"" << source_path.string() << "\" is a SoundFont file and is ignored.\n";
        }
        else
        {
            const ghc::filesystem::path& destination_path = ghc::filesystem::path( directory.string() + source_path.filename().string() );
            if ( ghc::filesystem::exists( source_path ) )
            {
                std::cout << "-- Copying \"" << source_path.string() << "\" -> \"" << destination_path.string() << "\"...";
                if ( ghc::filesystem::copy_file( source_path, destination_path ) )
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
                    std::cout << "-- \"" << source_path.string() << "\" does not exist.\n";

                    // Assuming the source_path is relative to the current directory the program is launched
                    const ghc::filesystem::path& lmms_source_file = ghc::filesystem::path( options.lmms_directory + source_path.string() );
                    std::cout << "-- Trying \"" << lmms_source_file.string() << "\"\n";

                    if ( ghc::filesystem::exists( lmms_source_file ) )
                    {
                        if ( ghc::filesystem::copy_file( lmms_source_file, destination_path ) )
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
                        std::cerr << "-- Cannot get \"" << lmms_source_file.string() << "\"\n";
                    }
                }
                else
                {
                    std::cerr << "-- Cannot get \"" << source_path.string() << "\" \n";
                }
            }
        }
    } );
    return copied_files;
}


ghc::filesystem::path generateProjectFileInPackage( const ghc::filesystem::path& lmms_file, const options::Options& options )
{
    const std::string& project_file = options.project_file;
    const std::string& destination_directory = options.destination_directory;

    if ( ghc::filesystem::hasExtension ( lmms_file, ".mmpz" ) )
    {
        return lmms::decompressProject( project_file, destination_directory, options.lmms_command );
    }
    else
    {
        const ghc::filesystem::path filepath( destination_directory + lmms_file.filename().string() );

        if ( ghc::filesystem::exists( filepath ) )
        {
            throw AlreadyExistingFileException( "ERROR: \"" + filepath.string() +
                                                "\" Already exists. You need to export to a fresh directory.\n" );
        }

        std::cout << "-- Copying \"" << lmms_file.string() << "\" -> \"" << filepath.string() << "\"...";
        if ( ghc::filesystem::copy_file( lmms_file, filepath ) )
        {
            std::cout << "DONE\n";
        }
        else
        {
            std::cout << "FAILED\n";
        }
        return filepath;
    }
}

// Public

const std::string pack( const options::Options& options )
{
    const std::string& project_file = options.project_file;
    const std::string& destination_directory = options.destination_directory;

    const ghc::filesystem::path lmms_file( project_file );
    const ghc::filesystem::path package_directory( destination_directory );

    if ( !ghc::filesystem::exists( lmms_file ) )
    {
        throw NonExistingFileException( "ERROR: \"" + lmms_file.string() + "\" does not exist.\n" );
    }

    if ( !ghc::filesystem::exists( package_directory ) )
    {
        if ( !ghc::filesystem::create_directories( package_directory ) )
        {
            throw DirectoryCreationException( "ERROR: \"" + package_directory.string() + "\" cannot be created.\n" );
        }
    }

    const ghc::filesystem::path& project_filepath = generateProjectFileInPackage( lmms_file, options );
    if ( !ghc::filesystem::exists( project_filepath ) )
    {
        throw NonExistingFileException( "ERROR: \"" + project_filepath.string() + "\" does not exist. Packaging aborted.\n" );
    }

    if ( !xml::isXmlFile( project_filepath ) )
    {
        throw InvalidXmlFileException( "ERROR: Invalid XML file: \"" + project_filepath.string() + "\". Packaging aborted.\n" );
    }

    const std::vector<ghc::filesystem::path>& files = retrievePathsOfFilesFromXMLFile( project_filepath.string() );
    std::cout << "\n-- This project has " << files.size() << " file(s) to copy.\n\n";

    if ( !files.empty() )
    {
        const ghc::filesystem::path sample_directory( destination_directory + "samples/" );
        if ( !ghc::filesystem::exists( sample_directory ) )
        {
            if ( !ghc::filesystem::create_directories( sample_directory ) )
            {
                throw DirectoryCreationException( "ERROR: \"" + sample_directory.string() + "\" cannot be created.\n" );
            }
        }

        const std::vector<ghc::filesystem::path>& copied_files = Packager::copyFilesTo( files, sample_directory.string(), options );
        std::cout << "-- " << copied_files.size() << " file(s) copied.\n\n";
    }
    else
    {
        std::cout << "-- \"" << project_filepath.filename().string() << "\" has no external sample or soundfont file to export.\n"
                  << "-- So it does not make sense to export this project.\n"
                  << "-- No package file will be generated, but the generated directory containing the project file is created: \""
                  << package_directory.string() + "\".\n";
        return package_directory.string();
    }

    return options.zip ? lmms::zipFile( package_directory ) : package_directory.string();
}


bool checkPackage( const options::Options& options )
{
    return lmms::checkZipFile( options.project_file );
}

}
