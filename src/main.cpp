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

#include "packager/packager.hpp"
#include "packager/options.hpp"
#include "external/filesystem/filesystem.hpp"

#include <iostream>

void usage ( const std::string& progname );

void usage ( const std::string& progname )
{
    std::cerr << "Invalid number of arguments\n"
              << "usage: " << ghc::filesystem::path( progname ).filename().string()
              << " --import|--export|--check|--info <name>.mmp(z) <destination/path>"
              << " [--no-sf2] [--no-zip] [--lmms-dir <path/to/lmms/data>]"
              << " [--lmms-exe <path/to/lmms/exe>] \n\n";
}

int main( int argc, char * argv[] )
{
    const int MINIMUM_ARGC = 3;

    if ( argc < MINIMUM_ARGC )
    {
        usage( argv[0] );
        return EXIT_FAILURE;
    }

    try
    {
        const options::Options& options = options::retrieveArguments( argc, argv );
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
            if ( !Packager::packageInfo( options ))
            {
                // This could happen with an invalid package
                std::cerr << "Error during information extraction.\n";
                return EXIT_FAILURE;
            }
        }
    }
    catch ( std::invalid_argument& e )
    {
        std::cerr << e.what() << "\n";
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
