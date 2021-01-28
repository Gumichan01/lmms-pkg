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

#include <iostream>
#include <cstdlib>
#include <algorithm>

#include "packager/packager.hpp"
#include "packager/options.hpp"
#include "external/filesystem/filesystem.hpp"


int main( int argc, char * argv[] )
{
    const int MINIMUM_ARGC = 4;

    if ( argc < MINIMUM_ARGC )
    {
        std::cerr << "Invalid number of arguments\n"
                  << "usage: " << fs::basename( argv[0] )
                  << " --import|--export <name>.mmp(z) <destination/path>"
                  << " [--no-sf2] [--no-zip] [--lmms-dir <path/to/lmms/data>]"
                  << " [--lmms-exe <path/to/lmms/exe>] \n"
                  << "/!\\ Some arguments are not active yet \n\n";
        return EXIT_FAILURE;
    }

    const options::Options& options = options::retrieveArguments( argc, argv );
    if ( options.operation == options::OperationType::Export )
    {
        const std::string& package = Packager::pack( options );
        if ( !package.empty() )
        {
            std::cout << "-- LMMS Project exported into \"" << package << "\"\n";
        }
    }
    else if ( options.operation == options::OperationType::Import )    // Import
    {
        /// TODO Import project
    }
    else
    {
        std::cerr << "Waht the f***? This is not reachable\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
