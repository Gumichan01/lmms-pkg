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
#include "exported_file.hpp"
#include "options.hpp"
#include "mmpz.hpp"
#include "xml.hpp"

#include "../program/printer.hpp"
#include "../exceptions/exceptions.hpp"
#include "../external/filesystem/filesystem.hpp"

#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

using namespace exceptions;
namespace fsys = ghc::filesystem;

namespace Packager
{


/// Export

const std::vector<ghc::filesystem::path> retrieveResourcesFromProject( const ghc::filesystem::path& project_file )
{
    const std::vector<std::string>& resources = xml::retrieveResourcesFromXmlFile( project_file.string() );
    std::vector<ghc::filesystem::path> paths;
    for ( const std::string& resource: resources )
    {
        paths.push_back( ghc::filesystem::path( resource ) );
    }
    return paths;
}


const std::vector<ExportedFile> copyExportedFilesTo( const std::vector<ghc::filesystem::path>& paths,
                                                     const ghc::filesystem::path& resource_directory,
                                                     const std::vector<std::string>& duplicated_filenames,
                                                     const options::Options& options )
{
    std::vector<ExportedFile> exported_files;
    std::unordered_map<std::string, int> name_counter;
    program::log::Printer print = program::log::getPrinter();

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

                    return fsys::path( resource_directory.string() + source_path.stem().string()
                                       + "-" + std::to_string( name_counter[src_pathname] ) + source_path.extension().string() );
                }
                else
                {
                    return fsys::path( resource_directory.string() + source_path.filename().string() );
                }
            } ();

            if ( fsys::exists( source_path ) )
            {
                print << "-- Copying \"" << ghc::filesystem::normalize( source_path.string() )
                      << "\" -> \"" << ghc::filesystem::normalize( destination_path.string() ) << "\"...";
                fsys::copy_file( source_path, destination_path );
                exported_files.push_back( ExportedFile{ source_path, destination_path.filename() } );
                print << "DONE\n";
            }
            else
            {
                bool found = false;
                if ( !options.resource_directories.empty() )
                {
                    print << "-- Searching for \"" << ghc::filesystem::normalize( source_path.string() )
                              << "\" in resource directories...\n";
                }

                for ( const std::string& dir : options.resource_directories )
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
                        exported_files.push_back( ExportedFile{ source_path, destination_path.filename() } );
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
    return exported_files;
}

const ghc::filesystem::path copyProjectToDestinationDirectory( const ghc::filesystem::path& lmms_file, const options::Options& options )
{
    const std::string& project_file = options.project_file;
    const std::string& destination_directory = options.destination_directory;
    program::log::Printer print = program::log::getPrinter();

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


const std::vector<std::string> getDuplicatedFilenames( const std::vector<ghc::filesystem::path> paths ) noexcept
{
    std::unordered_set<std::string> names;
    std::vector<std::string> duplicated_names;
    for ( const fsys::path& p : paths )
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

void configureExportedProject( const ghc::filesystem::path& project_file, const std::vector<ExportedFile>& exported_files )
{
    xml::configureExportedXmlFile( project_file.string(), exported_files );
}


/// Import

const std::vector<ghc::filesystem::path> getProjectResourcePaths( const ghc::filesystem::path& project_directory )
{
    std::vector<fsys::path> paths;
    for ( const auto& file : fsys::recursive_directory_iterator( project_directory ) )
    {
        const fsys::path& filepath = file.path();
        if ( fsys::is_regular_file( filepath ) )
        {
            paths.push_back( filepath );
        }
    }
    return paths;
}

void configureImportedProject( const ghc::filesystem::path& project_file, const std::vector<ghc::filesystem::path>& resources )
{
    std::vector<std::string> files;
    for (const fsys::path& p : resources)
    {
        files.push_back( p.string() );
    }
    xml::configureImportedProject( project_file.string(), files );
}

}
