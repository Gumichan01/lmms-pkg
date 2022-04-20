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

#include "options.hpp"
#include "../external/filesystem/filesystem.hpp"
#include "../external/argparse/argparse.hpp"

#include <iostream>
#include <stdexcept>


namespace fs = ghc::filesystem;

namespace options
{

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept;
argparse::ArgumentParser parse( const std::vector<std::string> argv );
OperationType getOperationType( argparse::ArgumentParser& parser );
const ExportOptions retrieveExportInfo( argparse::ArgumentParser& parser );

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept
{
    if ( !path.empty() && path.back() != '/' )
    {
        return path + '/';
    }
    return path;
}

argparse::ArgumentParser parse( const std::vector<std::string> argv )
{
    argparse::ArgumentParser parser;
    parser.addArgument( "--import" );
    parser.addArgument( "--export" );
    parser.addArgument( "--check" );
    parser.addArgument( "--info" );
    parser.addArgument( "--verbose" );
    parser.addArgument( "--no-zip" );
    parser.addArgument( "--sf2" );
    parser.addArgument( "--lmms-exe", 1 );
    parser.addArgument( "--rsc-dirs", '+' );
    parser.addArgument( "-t", "--target", 1 );
    parser.addFinalArgument( "source", 1 );

    parser.useExceptions( true );
    parser.parse( argv );
    return parser;
}

// Known BUG: If you put several args name among them, the first tested name passes
OperationType getOperationType( argparse::ArgumentParser& parser )
{
    if ( parser.retrieve<bool> ( "check" ) )
    {
        return OperationType::Check;
    }

    if ( parser.retrieve<bool> ( "info" ) )
    {
        return OperationType::Info;
    }

    if ( parser.retrieve<bool> ( "export" ) )
    {
        return OperationType::Export;
    }

    if ( parser.retrieve<bool> ( "import" ) )
    {
        return OperationType::Import;
    }

    return OperationType::InvalidOperation;
}


const ExportOptions retrieveExportInfo( argparse::ArgumentParser& parser )
{
    const bool zip = !parser.retrieve<bool>( "no-zip" );
    const bool sf2_export = parser.retrieve<bool>( "sf2" );
    const auto& dirs = parser.retrieve<std::vector<std::string> >( "rsc-dirs" );
    const auto& exe = parser.retrieve<std::string>( "lmms-exe" );
    const auto& lmms_exe = ( !exe.empty() ? exe : "lmms" );
    const bool verbose = parser.retrieve<bool>( "verbose" );
    const auto& project_file = fs::normalize( parser.retrieve<std::string>( "source" ) );
    // Some resources can be located in the directory where the project is.
    // It is possible that the path to the resource is relative to the project directory,
    // That is why by default the resource directory contains at least the project directory.
    std::vector<std::string> resource_dirs { fs::path( project_file ).parent_path().string() + "/" };

    for (const auto& dir : dirs)
    {
        const auto& directory = addTrailingSlashIfNeeded( fs::normalize( dir ) );
        resource_dirs.push_back( directory );
    }


    if ( verbose && !sf2_export )
    {
        std::cout << "-- Ignore Soundfont2 (SF2) files\n";
    }

    if ( !zip && verbose )
    {
        std::cout << "-- The destination package will not be zipped\n";
    }

    if ( verbose && parser.hasParsedArgument( "lmms-exe" ) )
    {
        std::cout << "-- LMMS executable: " << lmms_exe << "\n";
    }

    if ( verbose && !resource_dirs.empty() )
    {
        std::cout << "-- The following resource directories have been set: \n";
        for ( const auto& dir : resource_dirs )
        {
            std::cout << "*  " << dir << "\n";
        }
    }

    return ExportOptions { sf2_export, zip, dirs, lmms_exe };
}

/*
    Commands:

    - $lmms-pkg --check [--verbose] <file>
    - $lmms-pkg --info [--verbose] <file>
    - $lmms-pkg --export [--no-zip] [--sf2] [--verbose] --target <dir> <file>
    - $lmms-pkg --import [--verbose] --target <dir> <file>

*/
const Options retrieveArguments( const int argc, const char * argv[] )
{
    argparse::ArgumentParser parser = parse( std::vector<std::string>( argv, argv + argc ) );
    const OperationType operation = getOperationType( parser );
    const std::string& project_file = fs::normalize( parser.retrieve<std::string>( "source" ) );
    const bool verbose = parser.retrieve<bool>( "verbose" );

    if ( operation == OperationType::Check || operation == OperationType::Info )
    {
        return Options { operation, project_file, "", verbose, ExportOptions() };
    }

    if ( operation == OperationType::Export )
    {
        const std::string& destination_directory = addTrailingSlashIfNeeded( parser.retrieve<std::string>( "target" ) );
        const ExportOptions& export_opt = retrieveExportInfo( parser );
        return Options { operation, project_file, destination_directory, verbose, export_opt };
    }

    if ( operation == OperationType::Import )
    {
        const std::string& destination_directory = addTrailingSlashIfNeeded( parser.retrieve<std::string>( "target" ) );
        return Options { operation, project_file, destination_directory, verbose, ExportOptions() };
    }

    return Options { OperationType::InvalidOperation };
}


}
