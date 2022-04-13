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


#include <vector>
#include <unordered_map>
#include <string>

namespace options
{
struct Options;
}

namespace ghc
{
namespace filesystem
{
class path;
}
}


namespace Packager
{
const std::vector<ghc::filesystem::path> retrieveResourcesFromProject( const ghc::filesystem::path& project_file );
const std::unordered_map<std::string, std::string> copyExportedFilesTo( const std::vector<ghc::filesystem::path>& paths,
                                                                        const ghc::filesystem::path& directory,
                                                                        const std::vector<std::string>& duplicated_filenames,
                                                                        const options::Options& options );

const ghc::filesystem::path copyProjectToDestinationDirectory( const ghc::filesystem::path& lmms_file, const options::Options& options );

const std::vector<std::string> getDuplicatedFilenames(const std::vector<ghc::filesystem::path> paths) noexcept;

void configureExportedProject( const ghc::filesystem::path& project_file, const std::unordered_map<std::string, std::string>& resources );

const std::vector<ghc::filesystem::path> getProjectResourcePaths( const ghc::filesystem::path& project_directory );
void configureImportedProject( const ghc::filesystem::path& project_file, const std::vector<ghc::filesystem::path>& resources );
}
