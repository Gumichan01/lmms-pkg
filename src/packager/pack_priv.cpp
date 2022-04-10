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

#include "pack_priv.hpp"
#include "options.hpp"
#include "mmpz.hpp"
#include "xml.hpp"

#include "../program/printer.hpp"
#include "../exceptions/exceptions.hpp"

#include <iostream>
#include <unordered_set>
#include <unordered_map>

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
    for ( const tinyxml2::XMLElement * e : elements )
    {
        unique_paths.insert( e->Attribute( "src" ) );
    }

    std::vector<fsys::path> paths;
    for ( const std::string& s :  unique_paths )
    {
        paths.push_back( fsys::path( s ) );
    }

    return paths;
}

const std::unordered_map<std::string, std::string> copyFilesTo( const std::vector<fsys::path>& paths, const fsys::path& directory,
        const std::vector<std::string>& duplicated_filenames,
        const options::Options& options )
{
    std::unordered_map<std::string, std::string> copied_files;
    std::unordered_map<std::string, int> name_counter;
    Program::Printer print = Program::getPrinter();

    for ( const fsys::path& source_path : paths )
    {
        if ( fsys::hasExtension( source_path, ".sf2" ) && !options.sf2_export )
        {
            print << "-- Ignore SoundFont file: \"" << ghc::filesystem::normalize( source_path.string() ) << "\".\n";
        }
        else
        {
            const std::string& src_pathname = source_path.stem().string();
            const fsys::path& destination_path = [&] ()
            {
                if ( std::find( duplicated_filenames.cbegin(), duplicated_filenames.cend(), src_pathname ) != duplicated_filenames.cend() )
                {
                    if ( name_counter.find( src_pathname ) != name_counter.end() )
                    {
                        name_counter[src_pathname] += 1;
                    }
                    else
                    {
                        name_counter[src_pathname] = 1;
                    }

                    return fsys::path( directory.string() + source_path.stem().string()
                                       + "-" + std::to_string( name_counter[src_pathname] ) + source_path.extension().string() );
                }
                else
                {
                    return fsys::path( directory.string() + source_path.filename().string() );
                }
            } ();

            if ( fsys::exists( source_path ) )
            {
                print << "-- Copying \"" << ghc::filesystem::normalize( source_path.string() )
                      << "\" -> \"" << ghc::filesystem::normalize( destination_path.string() ) << "\"...";
                fsys::copy_file( source_path, destination_path );
                copied_files[source_path.string()] = destination_path.filename().string();
                print << "DONE\n";
            }
            else
            {
                bool found = false;
                if ( !options.lmms_directories.empty() )
                {
                    print << "-- Searching for \"" << ghc::filesystem::normalize( source_path.string() )
                              << "\" in LMMS directories...\n";
                }

                for ( const std::string& dir : options.lmms_directories )
                {
                    // Assuming the source_path is relative to the current directory it is located
                    // (example: in LMMS/ or LMMS_Data/)
                    const fsys::path& lmms_source_file = fsys::path( dir + source_path.string() );
                    if ( fsys::exists( lmms_source_file ) )
                    {
                        print << "-- Found \"" << ghc::filesystem::normalize( lmms_source_file.string() ) << "\"\n";
                        print << "-- Copying \"" << ghc::filesystem::normalize( lmms_source_file.string() ) << "\" -> \""
                              << ghc::filesystem::normalize( destination_path.string() ) << "\"...";
                        fsys::copy_file( lmms_source_file, destination_path );
                        copied_files[source_path.string()] = destination_path.filename().string();
                        print << "DONE\n";
                        found = true;
                        break;
                    }
                }
                if ( !found )
                {
                    std::cerr << "-- FILE NOT FOUND: \"" << ghc::filesystem::normalize( source_path.string() ) << "\".\n";
                }
            }
        }
    }
    return copied_files;
}

fsys::path generateProjectFileInPackage( const fsys::path& lmms_file, const options::Options& options )
{
    const std::string& project_file = options.project_file;
    const std::string& destination_directory = options.destination_directory;
    Program::Printer print = Program::getPrinter();

    if ( fsys::hasExtension ( lmms_file, ".mmpz" ) )
    {
        print << "-- This is a compressed project. Using LMMS to decompress it...\n";
        return lmms::decompressProject( project_file, destination_directory, options.lmms_command );
    }
    else
    {
        const fsys::path dest_file( destination_directory + lmms_file.filename().string() );

        if ( fsys::exists( dest_file ) )
        {
            throw AlreadyExistingFileException( "ERROR: \"" + ghc::filesystem::normalize( dest_file.string() ) +
                                                "\" Already exists. You need to export to a fresh directory.\n" );
        }

        print << "-- Copying \"" << ghc::filesystem::normalize( lmms_file.string() )
              << "\" -> \"" << ghc::filesystem::normalize( dest_file.string() ) << "\"...";
        fsys::copy_file( lmms_file, dest_file );
        print << "DONE\n";
        return dest_file;
    }
}


const std::vector<std::string> getDuplicatedFilenames( const std::vector<fsys::path> paths )
{
    std::unordered_set<std::string> names;
    std::vector<std::string> duplicated_names;
    for ( fsys::path p : paths )
    {
        const std::string& name = p.stem().string();

        if ( names.find( name ) != names.end() )
        {
            duplicated_names.push_back( name );
        }
        else
        {
            names.insert( name );
        }
    }
    return duplicated_names;
}

void configureProjectFileInPackage( const fsys::path& project_file, const std::unordered_map<std::string, std::string>& resources )
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile( project_file.string().c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        /// At this point, this part must not be reachable
        throw PackageImportException( "FATAL ERROR: The exported project file is invalid." );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames<tinyxml2::XMLElement>( root, NAMES );

    for ( tinyxml2::XMLElement * e : elements )
    {
        const std::string source( e->Attribute( "src" ) );
        auto element = resources.find( fsys::path( source ).string() );
        if ( element != resources.cend() )
        {
            const std::string& target = element->second;
            e->SetAttribute( "src", target.c_str() );
        }
    }

    tinyxml2::XMLError code = doc.SaveFile( project_file.string().c_str() );
    if ( code != tinyxml2::XMLError::XML_SUCCESS )
    {
        throw PackageExportException( "ERROR: Export failed : cannot save updated configuration into the project" +
                                       std::string( doc.ErrorStr() ) );
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
    Program::Printer print = Program::getPrinter();
    tinyxml2::XMLDocument doc;
    doc.LoadFile( project_file.string().c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        /// At this point, this part must not be reachable
        throw PackageImportException( "ERROR:The imported project file is invalid." );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames<tinyxml2::XMLElement>( root, NAMES );

    for ( tinyxml2::XMLElement * e : elements )
    {
        const std::string source( e->Attribute( "src" ) );
        const std::string& filename = fsys::path( source ).filename().string();
        auto found = std::find_if( resources.cbegin(), resources.cend(), [&filename] ( const fsys::path & resource )
        {
            return resource.filename().string() == filename;
        } );

        if ( found != resources.cend() )
        {
            print << "-- Configure \"" << e->Name() << "\" with \"" << filename << "\" in project. \n";
            const std::string& resource_found = fsys::absolute( ( *found ) ).string();
            print << "-- Set \"" << ghc::filesystem::normalize( resource_found ) << "\" in project file. \n";
            e->SetAttribute( "src", resource_found.c_str() );
        }
    }

    tinyxml2::XMLError code = doc.SaveFile( project_file.string().c_str() );
    if ( code != tinyxml2::XMLError::XML_SUCCESS )
    {
        throw PackageImportException( "ERROR: Import failed : cannot save updated configuration into the project" +
                                      std::string( doc.ErrorStr() ) );
    }
}

}
