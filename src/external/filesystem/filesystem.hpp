
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

#include <string>

namespace fs
{

// Note: Trailing '\' (on Windows) or '/' (on Linux) are not counted as part of the path name.
std::string basename( const std::string& path );
std::string dirname( const std::string& path );
// Normalize the path as a Unix-like path
std::string normalize( const std::string& path ) noexcept;
std::string copyFile( const std::string& source_path, const std::string& destination_path );
bool createDir( const std::string& directory ) noexcept;
bool hasExtension( const std::string& path, const std::string& extension );
bool exists( const std::string& path ) noexcept;

}

#endif // FILESYSTEM_HPP_INCLUDED
