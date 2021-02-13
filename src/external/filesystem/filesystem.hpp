
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

#include "../ghc/filesystem.hpp"

namespace ghc::filesystem
{

inline std::string normalize( const std::string& filepath )
{
#if defined(__WIN32__) || defined(__WIN64__)
    const char WIN_SEP = '\\';
    const char UNIX_SEP = '/';
    std::string normalized_path = filepath;
    std::replace( normalized_path.begin(), normalized_path.end(), WIN_SEP, UNIX_SEP );
    return normalized_path;
#else
    return filepath;
#endif
}

inline bool hasExtension( const std::string& filepath, const std::string& extension )
{
    return hasExtension ( path( filepath ), extension );
}

inline bool hasExtension( const path& filepath, const std::string& extension )
{
    return filepath.extension() == extension;
}

}

#endif // FILESYSTEM_HPP_INCLUDED
