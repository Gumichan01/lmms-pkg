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

#include "xml.hpp"

#include <algorithm>

namespace xml
{

inline bool contains( const std::vector<std::string>& names, const std::string& s )
{
    return std::find( names.cbegin(), names.cend(), s ) != names.cend();
}

// Public

bool isXmlFile( const std::string& project_file ) noexcept
{
    return tinyxml2::XMLDocument().LoadFile( project_file.c_str() ) == tinyxml2::XML_SUCCESS;
}

const std::vector<const tinyxml2::XMLElement *> getAllElementsByNames( const tinyxml2::XMLElement * root,
                                                                       const std::vector<std::string>& names )
{
    std::vector<const tinyxml2::XMLElement *> retrieved_elements;
    const tinyxml2::XMLElement * element = root->FirstChildElement();

    while ( element != nullptr )
    {
        if ( contains( names, element->Name() ) )
        {
            retrieved_elements.push_back( element );
        }
        const std::vector<const tinyxml2::XMLElement *>& elements = getAllElementsByNames( element, names );
        retrieved_elements.insert( retrieved_elements.end(), elements.cbegin(), elements.cend() );
        element = element->NextSiblingElement();
    }

    return retrieved_elements;
}

} // xml
