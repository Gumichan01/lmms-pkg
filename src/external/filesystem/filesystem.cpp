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

// FS_impl must be included before the FS_fwd
#include "../ghc/fs_impl.hpp"
#include "filesystem.hpp"

#include <algorithm>


namespace ghc::filesystem
{

std::string normalize( const std::string& filepath )
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

bool hasExtension( const path& filepath, const std::string& extension )
{
    return filepath.extension().string() == extension;
}

}
