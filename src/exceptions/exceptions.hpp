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

#ifndef EXCEPTIONS_HPP_INCLUDED
#define EXCEPTIONS_HPP_INCLUDED

#include <string>
#include <stdexcept>
#include <system_error>

namespace exceptions
{

class NonExistingFileException: public std::system_error
{
    const std::string msg;

public:
    explicit NonExistingFileException( const std::string& what_arg );
    explicit NonExistingFileException( const char * what_arg );

    virtual const char * what() const noexcept;
};


class AlreadyExistingFileException: public std::system_error
{
    const std::string msg;

public:
    explicit AlreadyExistingFileException( const std::string& what_arg );
    explicit AlreadyExistingFileException( const char * what_arg );

    virtual const char * what() const noexcept;
};


class DirectoryCreationException: public std::system_error
{
    const std::string msg;

public:
    explicit DirectoryCreationException( const std::string& what_arg );
    explicit DirectoryCreationException( const char * what_arg );

    virtual const char * what() const noexcept;
};

class InvalidXmlFileException: public std::exception
{
    const std::string msg;

public:
    explicit InvalidXmlFileException( const std::string& what_arg );
    explicit InvalidXmlFileException( const char * what_arg );

    virtual const char * what() const noexcept;
};

class PackageImportException: public std::system_error
{
    const std::string msg;

public:
    explicit PackageImportException( const std::string& what_arg );
    explicit PackageImportException( const char * what_arg );

    virtual const char * what() const noexcept;
};

class PackageExportException: public std::system_error
{
    const std::string msg;

public:
    explicit PackageExportException( const std::string& what_arg );
    explicit PackageExportException( const char * what_arg );

    virtual const char * what() const noexcept;
};

}

#endif  // EXCEPTIONS_HPP_INCLUDED
