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

#include "exceptions.hpp"


namespace exceptions
{

NonExistingFileException::NonExistingFileException( const std::string& what_arg )
    : std::system_error(), msg( what_arg ) {}

NonExistingFileException::NonExistingFileException( const char * what_arg )
    : std::system_error(), msg( what_arg ) {}

const char * NonExistingFileException::what() const noexcept
{
    return msg.c_str();
}


AlreadyExistingFileException::AlreadyExistingFileException( const std::string& what_arg )
    : std::system_error(), msg( what_arg ) {}

AlreadyExistingFileException::AlreadyExistingFileException( const char * what_arg )
    : std::system_error(), msg( what_arg ) {}

const char * AlreadyExistingFileException::what() const noexcept
{
    return msg.c_str();
}


DirectoryCreationException::DirectoryCreationException( const std::string& what_arg )
    : std::system_error(), msg( what_arg ) {}

DirectoryCreationException::DirectoryCreationException( const char * what_arg )
    : std::system_error(), msg( what_arg ) {}

const char * DirectoryCreationException::what() const noexcept
{
    return msg.c_str();
}


InvalidXmlFileException::InvalidXmlFileException( const std::string& what_arg )
    : std::exception(), msg( what_arg ) {}

InvalidXmlFileException::InvalidXmlFileException( const char * what_arg )
    : std::exception(), msg( what_arg ) {}

const char * InvalidXmlFileException::what() const noexcept
{
    return msg.c_str();
}


PackageImportException::PackageImportException( const std::string& what_arg )
    : std::exception(), msg( what_arg ) {}

PackageImportException::PackageImportException( const char * what_arg )
    : std::exception(), msg( what_arg ) {}

const char * PackageImportException::what() const noexcept
{
    return msg.c_str();
}


PackageExportException::PackageExportException( const std::string& what_arg )
    : std::exception(), msg( what_arg ) {}

PackageExportException::PackageExportException( const char * what_arg )
    : std::exception(), msg( what_arg ) {}

const char * PackageExportException::what() const noexcept
{
    return msg.c_str();
}


}
