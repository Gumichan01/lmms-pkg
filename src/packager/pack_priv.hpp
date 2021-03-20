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

#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

namespace options
{
struct Options;
}

namespace fsys = ghc::filesystem;

namespace Packager
{
const std::vector<fsys::path> retrieveResourcesFromXmlFile( const std::string& xml_file );
const std::unordered_map<std::string, std::string> copyFilesTo( const std::vector<fsys::path>& paths,
                                                                                        const fsys::path& directory,
                                                                                        const std::vector<std::string>& duplicated_filenames,
                                                                                        const options::Options& options );

fsys::path generateProjectFileInPackage( const fsys::path& lmms_file, const options::Options& options );

const std::vector<std::string> getDuplicatedFilenames(const std::vector<fsys::path> paths);

void configureProjectFileInPackage( const fsys::path& project_file, const std::unordered_map<std::string, std::string>& resources );

const std::vector<fsys::path> getProjectResourcePaths( const fsys::path& project_directory );
void configureProject( const fsys::path& project_file, const std::vector<fsys::path>& resources );
}
