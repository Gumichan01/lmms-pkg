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

#include "packager.hpp"
#include "pack_priv.hpp"
#include "options.hpp"
#include "mmpz.hpp"
#include "../program/printer.hpp"
#include "../exceptions/exceptions.hpp"
#include "../external/filesystem/filesystem.hpp"

#include <iostream>

using namespace exceptions;
namespace fsys = ghc::filesystem;

namespace Packager
{

const std::string pack( const options::Options& options )
{
    const std::string& project_file = options.project_file;
    const std::string& destination_directory = options.destination_directory;

    const fsys::path lmms_file( project_file );
    const fsys::path package_directory( destination_directory );
    Program::Printer print = Program::getPrinter();

    if ( !fsys::exists( lmms_file ) )
    {
        throw NonExistingFileException( "ERROR: \"" + lmms_file.string() + "\" does not exist.\n" );
    }

    bool dirtectory_created_by_app = false;
    if ( !fsys::exists( package_directory ) )
    {
        print << "-- Creating path: " << package_directory.string() << "\n";
        fsys::create_directories( package_directory );
        dirtectory_created_by_app = true;
    }

    const fsys::path& dest_project_file = copyProjectToDestinationDirectory( lmms_file, options );
    if ( !fsys::exists( dest_project_file ) )
    {
        if ( dirtectory_created_by_app )
        {
            std::error_code ecdir;
            fsys::remove( package_directory, ecdir );
        }
        throw NonExistingFileException( "ERROR: \"" + dest_project_file.string() + "\" does not exist. Packaging aborted.\n" );
    }

    if ( !lmms::checkLMMSProjectFile( dest_project_file ) )
    {
        std::error_code ecfile;
        fsys::remove( dest_project_file, ecfile );

        if ( dirtectory_created_by_app )
        {
            std::error_code ecdir;
            fsys::remove( package_directory, ecdir );
        }

        throw InvalidXmlFileException( "ERROR: Invalid XML file: \"" + fsys::normalize( dest_project_file.string() )
                                       + "\". Packaging aborted.\n" );
    }

    print << "-- Retrieving files to copy...\n";
    const std::vector<fsys::path>& sound_files = retrieveResourcesFromProject( dest_project_file.string() );
    const std::vector<std::string>& dup_files = getDuplicatedFilenames( sound_files );

    print << "\n-- This project has " << sound_files.size() << " file(s) that can be copied.\n\n";

    if ( !sound_files.empty() )
    {
        const fsys::path sample_directory( destination_directory + "resources/" );
        if ( !fsys::exists( sample_directory ) )
        {
            print << "-- Creating resource path: " << sample_directory.string() << "\n";
            fsys::create_directories( sample_directory );
        }

        const auto& copied_files = Packager::copyExportedFilesTo( sound_files, sample_directory.string(), dup_files, options );
        print << "-- " << copied_files.size() << " file(s) copied.\n\n";

        configureExportedProject( dest_project_file, copied_files );
        return fsys::normalize(options.zip ? lmms::zipFile( package_directory ).string() : package_directory.string());
    }
    else
    {
        std::cerr << "-- \"" << dest_project_file.filename().string() << "\" has no external sample or soundfont file to export.\n"
                  << "-- So it does not make sense to export this project.\n"
                  << "-- No package file will be generated, but the generated directory containing the project file is created: \""
                  << package_directory.string() + "\".\n";
        return fsys::normalize(package_directory.string());
    }
}


const std::string unpack( const options::Options& options )
{
    const fsys::path package( options.project_file );
    const fsys::path destination_directory( options.destination_directory );
    Program::Printer print = Program::getPrinter();

    if ( !fsys::exists( package ) )
    {
        throw NonExistingFileException( "ERROR: \"" + package.string() + "\" does not exist.\n" );
    }

    if ( lmms::checkZipFile( package.string() ) )
    {
        print << "-- Package is OK.\n\n";
        if ( !fsys::exists( destination_directory ) )
        {
            fsys::create_directories( destination_directory );
        }

        const fsys::path project_file( lmms::unzipFile( package, destination_directory ) );
        print << "-- Package extracted into \"" << fsys::normalize( destination_directory.string() ) << "\".\n";

        const fsys::path backup_file( project_file.string() + ".backup" );
        fsys::copy( project_file, backup_file );
        print << "-- Backup file created: \"" << fsys::normalize( backup_file.string() ) << "\"\n\n";

        configureImportedProject( project_file, getProjectResourcePaths( destination_directory ) );
        return fsys::normalize(project_file.parent_path().string() + "/");
    }
    else
    {
        throw PackageImportException( "ERROR: Cannot import \"" + fsys::normalize( package.string() )
                                      + "\": invalid package.\n" );
    }
}

bool checkPackage( const options::Options& options )
{
    return lmms::checkZipFile( fsys::path( options.project_file ) );
}

bool packageInfo( const options::Options& options )
{
    return lmms::zipFileInfo( fsys::path( options.project_file ) );
}

}
