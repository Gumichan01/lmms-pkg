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

#ifndef MMPZ_HPP_INCLUDED
#define MMPZ_HPP_INCLUDED

#include <string>

namespace ghc
{
namespace filesystem
{
class path;
}
}

namespace lmms
{

ghc::filesystem::path decompressProject( const std::string& project_file,
                                         const std::string& destination_directory,
                                         const std::string& lmms_command = "lmms" );

const ghc::filesystem::path zipFile( const ghc::filesystem::path& package_directory );
const ghc::filesystem::path unzipFile( const ghc::filesystem::path& package, const ghc::filesystem::path& directory );
bool checkZipFile( const ghc::filesystem::path& package_file );
bool zipFileInfo( const ghc::filesystem::path& package_file );
bool checkLMMSProjectFile( const ghc::filesystem::path& lmms_file );
}

#endif // MMPZ_HPP_INCLUDED
