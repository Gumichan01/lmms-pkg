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

#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <fstream>

#include "packager.hpp"
#include "../external/tinyxml2/tinyxml2.h"
#include "../external/filesystem/filesystem.hpp"

namespace Packager
{

// Pack

bool contains( const std::vector<std::string> names, const std::string& s );
const std::vector<const tinyxml2::XMLElement *> getAllElementsByNames( const tinyxml2::XMLElement * root, const std::vector<std::string>& names );

bool contains( const std::vector<std::string> names, const std::string& s )
{
    return std::find( names.cbegin(), names.cend(), s ) != names.cend();
}

const std::vector<const tinyxml2::XMLElement *> getAllElementsByNames( const tinyxml2::XMLElement * root, const std::vector<std::string>& names )
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

const std::vector<std::string> retrievePathsOfFilesFromXMLFile( const std::string& project_file )
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError code = doc.LoadFile( project_file.c_str() );

    if ( code != tinyxml2::XML_SUCCESS )
    {
        std::cerr << "Ouch! Tinyxml Error code: " << code << "\n";
        return std::vector<std::string>();
    }

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        std::cerr << "No root element. Are you sure this file contains an XML content?\n";
        return std::vector<std::string>();
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<const tinyxml2::XMLElement *>& elements = getAllElementsByNames( root, NAMES );

    std::unordered_set<std::string> unique_paths;
    std::for_each( elements.cbegin(), elements.cend(), [&unique_paths]( const tinyxml2::XMLElement * e )
    {
        unique_paths.insert( e->Attribute( "src" ) );
    } );

    return std::vector<std::string>( unique_paths.cbegin(), unique_paths.cend() );
}

const std::vector<std::string> copyFilesTo( const std::vector<std::string>& paths, const std::string& directory )
{
    // For each path
    // 1. if the file exists, copy it to the destination + save path to the copied paths
    // 2. if is does not exists, print a warning message
    std::for_each( paths.cbegin(), paths.cend(),
                   [&directory]( const std::string & path )
    {
        std::ifstream input( path, std::ios_base::in | std::ios_base::binary );
        if ( !input.is_open() )
        {
            std::cerr << "Warning: \"" << path << "\" cannot be open." << "\n";
        }
        else
        {
            const std::string destination_path = directory + fs::SEPARATOR + fs::basename( path );
            std::cout << "Copying " << destination_path << "...";
            std::ofstream output( destination_path, std::ios_base::out | std::ios_base::binary );
            output << input.rdbuf();
            std::cout << "DONE\n";
        }
    } );
    return std::vector<std::string>();
}

}
