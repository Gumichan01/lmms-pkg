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
#include "pack_priv.hpp"
#include "options.hpp"
#include "mmpz.hpp"
#include "xml.hpp"
#include "../exceptions/exceptions.hpp"

#include <iostream>


using namespace exceptions;

namespace Packager
{

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
    const std::vector<std::string>& dup_files = getDuplicatedFilenames( files );

    std::cout << "\n-- This project has " << files.size() << " file(s) to copy.\n\n";

    if ( !files.empty() )
    {
        const fsys::path sample_directory( destination_directory + "resources/" );
        if ( !fsys::exists( sample_directory ) )
        {
            fsys::create_directories( sample_directory );
        }

        const std::unordered_map<std::string, std::string>& copied_files = Packager::copyFilesTo( files, sample_directory.string(), dup_files, options );
        std::cout << "-- " << copied_files.size() << " file(s) copied.\n\n";
        configureProjectFileInPackage( project_filepath, copied_files );
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
