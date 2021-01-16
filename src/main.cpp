/*
*   LMMS Project Packager
*	Copyright © 2021 Luxon Jean-Pierre
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cstdlib>

#include "packager/packager.hpp"

int main(int argc, char * argv[]) {

    const std::string PACKAGE_PATH = "export";
    const std::vector<std::string>& v = Packager::retrievePathsOfFilesFromXMLFile("D:/git_dir/lmms-pkg/data/demo-evasion.mmp");
    std::cout << "Number of files: " << v.size() << " \n";
    const std::vector<std::string>& v2 = Packager::copyFilesTo(v, PACKAGE_PATH);
    std::cout << "Copied files: " << v2.size() << " \n";
    return EXIT_SUCCESS;
}
