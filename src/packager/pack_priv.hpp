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


#include "../external/filesystem/filesystem.hpp"

namespace options
{
struct Options;
}

namespace Packager
{
const std::vector<ghc::filesystem::path> retrieveResourcesFromXmlFile( const std::string& xml_file );
const std::vector<ghc::filesystem::path> copyFilesTo( const std::vector<ghc::filesystem::path>& paths,
                                                      const ghc::filesystem::path& directory,
                                                      const options::Options& options );

ghc::filesystem::path generateProjectFileInPackage( const ghc::filesystem::path& lmms_file, const options::Options& options );

const std::vector<ghc::filesystem::path> getProjectResourcePaths( const ghc::filesystem::path& project_directory );
void configureProject( const ghc::filesystem::path& project_file, const std::vector<ghc::filesystem::path>& resources );
}
