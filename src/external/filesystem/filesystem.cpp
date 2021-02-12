
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


#include "filesystem.hpp"
#include "../ghc/filesystem.hpp"

namespace fsys = ghc::filesystem;

namespace fs
{

// Public

std::string basename( const std::string& path )
{
    return fsys::path( normalize( path ), fsys::path::generic_format ).filename();
}
std::string dirname( const std::string& path )
{
    return fsys::path( normalize( path ), fsys::path::generic_format ).parent_path();
}

std::string normalize( const std::string& path ) noexcept
{
#if defined(__WIN32__) || defined(__WIN64__)
    const char WIN_SEP = '\\';
    const char UNIX_SEP = '/';
    std::string normalized_path = path;
    std::replace( normalized_path.begin(), normalized_path.end(), WIN_SEP, UNIX_SEP );
    return normalized_path;
#else
    return path;
#endif
}

std::string copyFile( const std::string& source_path, const std::string& destination_path )
{
    return fsys::copy_file( fsys::path( source_path, fsys::path::generic_format ),
                            fsys::path( destination_path, fsys::path::generic_format ),
                            fsys::copy_options::overwrite_existing ) ? destination_path : "";
}

bool createDir( const std::string& directory ) noexcept
{
    return fsys::create_directories( fsys::path( directory ) );
}

bool hasExtension( const std::string& path, const std::string& extension )
{
    return fsys::path( path ).extension() == extension;
}

bool exists( const std::string& path ) noexcept
{
    return fsys::exists( fsys::path( path ) );
}

}   // fs
