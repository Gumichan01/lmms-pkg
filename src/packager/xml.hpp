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

#ifndef XML_HPP_INCLUDED
#define XML_HPP_INCLUDED

#include "../external/tinyxml2/tinyxml2.h"

#include <vector>
#include <string>

namespace xml
{
bool isXmlFile( const std::string& project_file ) noexcept;
const std::vector<const tinyxml2::XMLElement *> getAllElementsByNames( const tinyxml2::XMLElement * root,
                                                                       const std::vector<std::string>& names );
}

#endif // XML_HPP_INCLUDED
