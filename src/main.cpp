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
                  << " --import|--export [--no-sf2] [--lmms-dir <path/to/lmms/data>]"
                  << " [--lmms-exe <path/to/lmms/exe>] <name>.mmp(z) <destination/path> \n"
                  << "/!\\ Some arguments are not active yet \n\n";
        return EXIT_FAILURE;
    }

    const options::Options& options = options::retrieveArguments( argc, argv );
    if ( options.operation == options::OperationType::Export )
    {
        const std::string& package = Packager::pack( options );
        std::cout << "-- LMMS Project packaged into \"" << package << "\"\n";
    }
    else    // Import
    {
        /// TODO Import project
    }
    return EXIT_SUCCESS;
}
