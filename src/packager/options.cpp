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
const argparse::ArgumentParser parse( const std::vector<std::string> argv );
OperationType getOperationType( const argparse::ArgumentParser& parser );
const ExportOptions retrieveExportInfo( const argparse::ArgumentParser& parser );

std::string addTrailingSlashIfNeeded( const std::string& path ) noexcept
{
    if ( !path.empty() && path.back() != '/' )
    {
        return path + '/';
    }
    return path;
}

const argparse::ArgumentParser parse( const std::vector<std::string> argv )
{
    return argparse::ArgumentParser()
           .addArgument( "-u", "--unpack" )
           .addArgument( "-p", "--pack" )
           .addArgument( "-c", "--check" )
           .addArgument( "-i", "--info" )
           .addArgument( "-v", "--verbose" )
           .addArgument( "--no-zip" )
           .addArgument( "--sf2" )
           .addArgument( "--lmms-exe", 1 )
           .addArgument( "--rsc-dirs", '+' )
           .addArgument( "-t", "--target", 1 )
           .addFinalArgument( "source", 1 ).useExceptions( true ).parse( argv );
}


OperationType getOperationType( const argparse::ArgumentParser& parser )
{
    const auto& parsed_args = parser.retrieveParsedArguments();

    unsigned int op_count = 0;
    for ( const auto& arg: parsed_args )
    {
        if ( arg.name == "check" || arg.name == "info" || arg.name == "pack" || arg.name == "unpack" )
        {
            op_count++;
        }
    }

    if ( op_count > 1 )
    {
        throw std::invalid_argument("Too many operation types provided. You must provide only one of { pack, unpack, check, info }.\n");
    }
    else if ( op_count == 0 )
    {
        throw std::invalid_argument("Missing operation type. You must provide one of { pack, unpack, check, info }.\n");
    }

    if ( parser.retrieve<bool> ( "check" ) )
    {
        return OperationType::Check;
    }

    if ( parser.retrieve<bool> ( "info" ) )
    {
        return OperationType::Info;
    }

    if ( parser.retrieve<bool> ( "pack" ) )
    {
        return OperationType::Pack;
    }

    if ( parser.retrieve<bool> ( "unpack" ) )
    {
        return OperationType::Unpack;
    }


    throw std::invalid_argument( "Internal error. Please contact a developer." );
}


const ExportOptions retrieveExportInfo( const argparse::ArgumentParser& parser )
{
    const bool zip = !parser.retrieve<bool>( "no-zip" );
    const bool sf2_export = parser.retrieve<bool>( "sf2" );
    const auto& dirs = parser.retrieve<std::vector<std::string> >( "rsc-dirs" );
    const auto& lmms_exe = ( parser.hasParsedArgument( "lmms-exe" ) ? parser.retrieve( "lmms-exe" ) : "lmms" );
    const auto& project_file = fs::normalize( parser.retrieve( "source" ) );
    const bool verbose = parser.retrieve<bool>( "verbose" );
    // Some resources can be located in the directory where the project is.
    // It is possible that the path to the resource is relative to the project directory,
    // That is why by default the resource directory contains at least the project directory.
    std::vector<std::string> resource_dirs { fs::path( project_file ).parent_path().string() + "/" };

    for ( const auto& dir : dirs )
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
    const argparse::ArgumentParser& parser = parse( std::vector<std::string>( argv, argv + argc ) );
    const std::string& project_file = fs::normalize( parser.retrieve( "source" ) );

    if ( project_file.empty() )
    {
        throw std::invalid_argument( "No source project file provided.\n" );
    }

    const OperationType operation = getOperationType( parser );
    const bool verbose = parser.retrieve<bool>( "verbose" );

    if ( operation == OperationType::Check || operation == OperationType::Info )
    {
        return Options { operation, project_file, "", verbose, ExportOptions() };
    }

    if ( operation == OperationType::Pack )
    {
        if ( parser.hasParsedArgument( "target" ) )
        {
            const std::string& destination_directory = addTrailingSlashIfNeeded( parser.retrieve( "target" ) );
            const ExportOptions& export_opt = retrieveExportInfo( parser );
            return Options { operation, project_file, destination_directory, verbose, export_opt };
        }
        else
        {
            throw std::invalid_argument( "No target directory provided. Please specify where you want to export the project.\n" );
        }
    }

    if ( operation == OperationType::Unpack )
    {
        if ( parser.hasParsedArgument( "target" ) )
        {
            const std::string& destination_directory = addTrailingSlashIfNeeded( parser.retrieve( "target" ) );
            return Options { operation, project_file, destination_directory, verbose, ExportOptions() };
        }
        else
        {
            throw std::invalid_argument( "No target directory provided. Please specify where you want to import the project.\n" );
        }
    }

    // Normally, this line must be unreachable
    throw std::invalid_argument( "Invalid Operation. Internal error. Please contact a developer.\n" );
}

}
