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

namespace
{

const std::string VERSION = "0.3.0-dev";

inline bool isHelp ( const std::string& s ) noexcept
{
    return s == "--help" || s == "-h";
}

}

void usage ( const std::string& progname )
{
    const auto& p = ghc::filesystem::path( progname ).filename().string();
    std::cerr << "Usage: \n"
              << p << " --check  [--verbose] <file>\n"
              << p << " --info   [--verbose] <file>\n"
              << p << " --pack   [--no-zip] [--sf2] [--verbose] [--rsc-dirs <path/to/data>] --target <dir> <file>\n"
              << p << " --unpack [--verbose] --target <dir> <file>\n\n";
}


void help ( const std::string& progname )
{
    const auto& p = ghc::filesystem::path( progname ).filename().string();
    std::cerr << "Usage: \n"
              << p << " --check  [--verbose] <file>\n"
              << p << " --info   [--verbose] <file>\n"
              << p << " --pack   [--no-zip] [--sf2] [--verbose] [--rsc-dirs <path/to/data>] --target <dir> <file>\n"
              << p << " --unpack [--verbose] --target <dir> <file>\n\n"
              << "The LMMS package manager\n\n"
              << "Operations:\n"
              << "-c, --check      " << "Check if the file is valid\n"
              << "-i, --info       " << "Get information about the file\n"
              << "-p, --pack       " << "Package the file\n"
              << "-u, --unpack     " << "Unpack the package and import the project\n"
              << "-h, --help       " << "Display the manual\n"
              << "--version        " << "Get the version of the program\n\n"
              << "Options:\n"
              << "--target         " << "(Mandatory for import and export) Set the destination directory\n"
              << "--no-zip         " << "Do not compress the destination directory (Export)\n"
              << "--rsc_dirs       " << "Provide directories where some missing external samples are located (Export)\n"
              << "--sf2            " << "Include SoundFont2 files in the package at export (Export)\n"
              << "-v, --verbose    " << "Verbose mode\n\n";

}


int run( const int argc, const char * argv[] )
{
    const int MINIMUM_ARGC = 3;

    if ( argc < MINIMUM_ARGC )
    {
        if ( argc == 2 )
        {
            const std::string& arg = std::string( argv[1] );
            if ( isHelp( arg ) )
            {
                help( argv[0] );
                return EXIT_SUCCESS;
            }

            if ( arg == "--version" )
            {
                std::cerr << "lmms-pkg (LMMS Project Packager) " << VERSION << "\n";
                return EXIT_SUCCESS;
            }
        }

        usage( argv[0] );
        return EXIT_FAILURE;
    }

    try
    {
        const options::Options& options = options::retrieveArguments( argc, argv );
        log::setVerbose( options.verbose );
        if ( options.operation == options::OperationType::Pack )
        {
            const std::string& package = Packager::pack( options );
            std::cout << "-- LMMS project exported into \"" << package << "\"\n";
        }
        else if ( options.operation == options::OperationType::Unpack )
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
