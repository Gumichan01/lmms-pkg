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

#include "xml.hpp"
#include "exported_file.hpp"
#include "../program/printer.hpp"
#include "../exceptions/exceptions.hpp"
#include "../external/tinyxml2/tinyxml2.h"
#include "../external/filesystem/filesystem.hpp"

#include <unordered_set>

using namespace exceptions;
namespace fsys = ghc::filesystem;

namespace xml
{

bool checkLMMSProjectBuffer( const std::unique_ptr<char []>& buffer, const unsigned int bufsize )
{
    const char * ROOT_NAME = "lmms-project";
    const char * PROJECT_TYPE_NAME = "type";
    const char * PROJECT_TYPE_VALUE = "song";
    const char * VERSION_ATTRIBUTE = "creatorversion";
    const std::size_t VSIZE = 3;
    const std::array<std::string, VSIZE> VALID_VERSIONS{"1.2.0", "1.2.1", "1.2.2"};
    const char * VALID_VERSIONS_STR = "{ 1.2.0, 1.2.1, 1.2.2 }";
    program::log::Printer print = program::log::getPrinter();

    bool valid_project = false;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError tinycode = doc.Parse( buffer.get(), bufsize );

    if ( tinycode == tinyxml2::XML_SUCCESS )
    {
        print << "-- Valid XML document\n";
        const tinyxml2::XMLElement * root = doc.RootElement();
        if ( root != nullptr )
        {
            const std::string root_name( root->Name() ? root->Name() : "" );
            if ( root_name == ROOT_NAME )
            {
                print << "-- Valid LMMS project file\n";
                const char * type_attr_value = root->Attribute( PROJECT_TYPE_NAME );
                const std::string project_type( type_attr_value ? type_attr_value : "" );
                if ( project_type == PROJECT_TYPE_VALUE )
                {
                    const char * version_attr_value = root->Attribute( VERSION_ATTRIBUTE );
                    const std::string version( version_attr_value ? version_attr_value : "" );
                    if ( version.empty() || std::find(VALID_VERSIONS.cbegin(), VALID_VERSIONS.cend(), version) != VALID_VERSIONS.cend() )
                    {
                        print << "-- Valid LMMS Version of the project\n";
                        valid_project = true;
                    }
                    else
                    {
                        std::cerr << "ERROR: This project was generated by a not supported version of LMMS: "
                                  << version << ". Only one of the following versions are supported: "
                                  << VALID_VERSIONS_STR << ".\n";
                    }
                }
                else
                {
                    std::cerr << "ERROR: Invalid project type. It must be a song, not '"<< project_type << "'.\n";
                }
            }
            else
            {
                std::cerr << "ERROR: This is not a valid LMMS project file.\n";
            }
        }
        else
        {
            std::cerr << "ERROR: Cannot navigate through the XML document.\n";
        }
    }
    else
    {
        std::cerr << "ERROR: Invalid XML file.\n";
    }

    return valid_project;
}


bool projectInfo( const std::unique_ptr<char []>& buffer, const unsigned int bufsize )
{
    const char * HEAD_NAME = "head";
    const char * ROOT_NAME = "lmms-project";
    const char * VERSION_ATTRIBUTE = "creatorversion";
    const char * PROJECT_VERSION_ATTRIBUTE = "version";
    const char * TIME_SIG_NUM_ATTRIBUTE = "timesig_numerator";
    const char * TIME_SIG_DEN_ATTRIBUTE = "timesig_denominator";
    const char * BPM_ATTRIBUTE = "bpm";

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError tinycode = doc.Parse( buffer.get(), bufsize );

    if ( tinycode == tinyxml2::XML_SUCCESS )
    {
        const tinyxml2::XMLElement * root = doc.RootElement();
        if ( root != nullptr )
        {
            const std::string root_name( root->Name() ? root->Name() : "" );
            if ( root_name == ROOT_NAME )
            {
                const char * version_attr_value = root->Attribute( VERSION_ATTRIBUTE );
                const char * project_version_attr_value = root->Attribute( PROJECT_VERSION_ATTRIBUTE );
                const std::string lmms_version( version_attr_value ? version_attr_value : "" );
                const std::string project_version( project_version_attr_value ? project_version_attr_value : "" );

                std::cout << "---- LMMS version: " << lmms_version << "\n";
                std::cout << "---- Project version: " << project_version << "\n";

                const tinyxml2::XMLElement * head = root->FirstChildElement( HEAD_NAME );
                if (head  != nullptr)
                {
                    const char * time_sig_num_attr_value = head->Attribute( TIME_SIG_NUM_ATTRIBUTE );
                    const char * time_sig_den_attr_value = head->Attribute( TIME_SIG_DEN_ATTRIBUTE );
                    const char * bpm_attr_value = head->Attribute( BPM_ATTRIBUTE );
                    const std::string bpm( bpm_attr_value ? bpm_attr_value : "" );
                    const std::string time_sig( std::string( time_sig_num_attr_value ? time_sig_num_attr_value : "" ) + "/" +
                                                ( time_sig_den_attr_value ? time_sig_den_attr_value : "" ) );

                    std::cout << "---- BPM: " << bpm << "\n";
                    std::cout << "---- Time Signature: " << time_sig << "\n";
                }
            }
            else
            {
                std::cerr << "ERROR: Not an LMMS project.\n";
                return false;
            }
        }
        else
        {
            std::cerr << "ERROR: Invalid XML file.\n";
            return false;
        }
    }
    else
    {
        std::cerr << "ERROR: The project file is not a valid LMMS project file.\n";
        return false;
    }

    return true;
}

const std::vector<std::string> retrieveResourcesFromXmlFile( const std::string& xml_file )
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile( xml_file.c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        throw InvalidXmlFileException( "No root element. Are you sure this file contains an XML content?\n" );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<const tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames<const tinyxml2::XMLElement>( root, NAMES );

    std::unordered_set<std::string> unique_paths;
    for ( const tinyxml2::XMLElement * e : elements )
    {
        unique_paths.insert( e->Attribute( "src" ) );
    }

    std::vector<std::string> paths;
    std::copy_if( unique_paths.begin(), unique_paths.end(), std::back_inserter( paths ),
                  [] ( const std::string& p ) { return !p.empty(); } );
    return paths;
}

void configureExportedXmlFile( const std::string& project_file, const std::vector<ExportedFile>& exported_files )
{
    program::log::Printer print = program::log::getPrinter();
    tinyxml2::XMLDocument doc;
    doc.LoadFile( project_file.c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        /// At this point, this part must not be reachable
        throw PackageImportException( "FATAL ERROR: The exported project file is invalid." );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames<tinyxml2::XMLElement>( root, NAMES );

    for ( tinyxml2::XMLElement * e : elements )
    {
        const fsys::path source = std::string( e->Attribute( "src" ) );
        auto exported_file = std::find_if( exported_files.cbegin(), exported_files.cend(), [&source] ( const ExportedFile& f )
        {
            return f.source == source;
        });
        if ( exported_file != exported_files.cend() )
        {
            const std::string& target = exported_file->dest.string();
            print << "-- " << e->Name() << ": \"" << fsys::normalize( target ) << "\".\n";
            e->SetAttribute( "src", target.c_str() );
        }
    }

    tinyxml2::XMLError code = doc.SaveFile( project_file.c_str() );
    if ( code != tinyxml2::XMLError::XML_SUCCESS )
    {
        throw PackageExportException( "ERROR: Export failed : cannot save updated configuration into the project" +
                                       std::string( doc.ErrorStr() ) );
    }
}


void configureImportedProject( const std::string& project_file, const std::vector<std::string>& resources )
{
    program::log::Printer print = program::log::getPrinter();
    tinyxml2::XMLDocument doc;
    doc.LoadFile( project_file.c_str() );

    const tinyxml2::XMLElement * root = doc.RootElement();
    if ( root == nullptr )
    {
        /// At this point, this part must not be reachable
        throw PackageImportException( "ERROR:The imported project file is invalid." );
    }

    const std::vector<std::string> NAMES{ "audiofileprocessor", "sf2player", "sampletco" };
    const std::vector<tinyxml2::XMLElement *>& elements = xml::getAllElementsByNames<tinyxml2::XMLElement>( root, NAMES );

    for ( tinyxml2::XMLElement * e : elements )
    {
        const std::string source( e->Attribute( "src" ) );
        const std::string& filename = fsys::path( source ).filename().string();
        auto found = std::find_if( resources.cbegin(), resources.cend(), [&filename] ( const std::string& resource )
        {
            return fsys::path( resource ).filename().string() == filename;
        } );

        if ( found != resources.cend() )
        {
            print << "-- Configure \"" << e->Name() << "\" with \"" << filename << "\" in project. \n";
            const std::string& resource_found = fsys::absolute( ( *found ) ).string();
            print << "-- Set \"" << fsys::normalize( resource_found ) << "\" in project file. \n";
            e->SetAttribute( "src", resource_found.c_str() );
        }
    }

    tinyxml2::XMLError code = doc.SaveFile( project_file.c_str() );
    if ( code != tinyxml2::XMLError::XML_SUCCESS )
    {
        throw PackageImportException( "ERROR: Import failed : cannot save updated configuration into the project" +
                                      std::string( doc.ErrorStr() ) );
    }
}

} // xml
