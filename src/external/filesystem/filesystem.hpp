
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

// This file was taken from LunatiX, and has been adapted for this project

#ifndef FILESYSTEM_HPP_INCLUDED
#define FILESYSTEM_HPP_INCLUDED

#include "../utf8/utf8_string.hpp"
#include <string>

namespace fs
{
// I use the slash ('/') as a default separator
const std::string SEPARATOR ( "/" );

// Note: Trailing '\' (on Windows) or '/' (on Linux) are not counted as part of the path name.
std::string basename( const std::string& path ) noexcept;
std::string dirname( const std::string& path ) noexcept;
// Normalize the path as a Unix-like path
std::string normalize( const std::string& path ) noexcept;

}

#endif // FILESYSTEM_HPP_INCLUDED