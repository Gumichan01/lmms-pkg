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

#ifndef OPTIONS_HPP_INCLUDED
#define OPTIONS_HPP_INCLUDED

#include <string>
#include <vector>

namespace options
{

enum class OperationType
{
    Import,
    Export,
    Check,
    InvalidOperation
};

struct Options
{
    const OperationType operation = OperationType::InvalidOperation;
    const std::string project_file = "";
    const std::string destination_directory = "";
    // Optional
    const bool sf2_export = true;
    const bool zip = true;
    const std::vector<std::string> lmms_directories {};
    const std::string lmms_command = "";     // Very useful if LMMS is not in the $PATH env
};

const Options retrieveArguments( int argc, char * argv[] );

}

#endif // OPTIONS_HPP_INCLUDED
