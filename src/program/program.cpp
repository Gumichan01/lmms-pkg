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

#include "program.hpp"
#include "printer.hpp"
#include "../packager/packager.hpp"
#include "../packager/options.hpp"
#include "../external/filesystem/filesystem.hpp"

namespace program
{

void usage ( const std::string& progname )
{
    std::cerr << "Usage: " << ghc::filesystem::path( progname ).filename().string()
              << " $lmms-pkg --check [--verbose] <file>"
              << " $lmms-pkg --info [--verbose] <file>"
              << " $lmms-pkg --export [--no-zip] [--sf2] [--verbose] [--rsc-dirs <path/to/data>] --target <dir> <file>"
              << " $lmms-pkg --import [--verbose] --target <dir> <file>\n\n";
}


void help ( const std::string& progname )
{
    const auto& prog = ghc::filesystem::path( progname ).filename().string();
    std::cerr << "Usage: \n"
              << prog << " --check [--verbose] <file>\n"
              << prog << " --info [--verbose] <file>\n"
              << prog << " --export [--no-zip] [--sf2] [--verbose] [--rsc-dirs <path/to/data>] --target <dir> <file>\n"
              << prog << " --import [--verbose] --target <dir> <file>\n\n"
              << "The LMMS package manager\n\n"
              << "Operations:\n"
              << "--check      " << "Check if the file is valid\n"
              << "--export     " << "Package the file\n"
              << "--info       " << "Get information about the file\n"
              << "--import     " << "Unpack the package and import the project\n"
              << "--help       " << "Display the manual\n\n"
              << "Options:\n"
              << "--target     " << "(Mandatory for import and export) Set the destination directory\n"
              << "--no-zip     " << "Do not compress the destination directory (Export)\n"
              << "--rsc_dirs   " << "Provide directories where some missing external samples are located (Export)\n"
              << "--sf2        " << "Include SoundFont2 files in the package at export (Export)\n"
              << "--verbose    " << "Verbose mode\n\n";

}

namespace
{

inline bool isHelp ( const std::string& s )
{
    return s == "--help" || s == "-h";
}

}

int run( const int argc, const char * argv[] )
{
    const int MINIMUM_ARGC = 3;

    if ( argc < MINIMUM_ARGC )
    {
        if ( argc == 2 && isHelp( std::string( argv[1] ) ) )
        {
            help( argv[1] );
            return EXIT_SUCCESS;
        }

        usage( argv[0] );
        return EXIT_FAILURE;
    }

    try
    {
        const options::Options& options = options::retrieveArguments( argc, argv );
        log::setVerbose( options.verbose );
        if ( options.operation == options::OperationType::Export )
        {
            const std::string& package = Packager::pack( options );
            std::cout << "-- LMMS project exported into \"" << package << "\"\n";
        }
        else if ( options.operation == options::OperationType::Import )
        {
            const std::string& directory = Packager::unpack( options );
            std::cout << "-- LMMS project imported into \"" << directory << "\"\n";
        }
        else if ( options.operation == options::OperationType::Check )
        {
            bool valid = Packager::checkPackage( options );
            std::cout << ( valid ? "-- Valid package.\n" : "Invalid package.\n" );
            if ( !valid )
            {
                return EXIT_FAILURE;
            }
        }
        else if ( options.operation == options::OperationType::Info )
        {
            if ( !Packager::packageInfo( options ) )
            {
                // This could happen with an invalid package
                return EXIT_FAILURE;
            }
        }
    }
    catch ( std::invalid_argument& e )
    {
        std::cerr << "ERROR: Invalid Argument: " << e.what() << "\n";
        usage( argv[0] );
        return EXIT_FAILURE;
    }
    catch ( std::exception& e )
    {
        std::cerr << "\n" << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

}
