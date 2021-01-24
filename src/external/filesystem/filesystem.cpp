
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

// This file was taken from LunatiX, and has been adapted for this project

#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

#include "filesystem.hpp"

namespace fs
{

const char * CURRENT_DIR = "./";
const UTF8string UT8_SEPARATOR ( "/" );

const UTF8string CURRENT ( "." );
const UTF8string PARENT ( ".." );
const UTF8string ROOT ( UT8_SEPARATOR );
const UTF8string SCURS ( UT8_SEPARATOR + CURRENT + UT8_SEPARATOR );
const UTF8string SPARS ( UT8_SEPARATOR + PARENT  + UT8_SEPARATOR );

UTF8string removeTrailingSep ( const UTF8string& u8str ) noexcept;
size_t countSeparator ( const UTF8string& u8str ) noexcept;
bool checkBasename ( const UTF8string& npath );
bool checkDirname ( const UTF8string& npath );
UTF8string utf8Basename ( const UTF8string& path ) noexcept;
UTF8string utf8Dirname ( const UTF8string& path ) noexcept;

// separator: '/' on POSIX system, '\' on Windows
UTF8string removeTrailingSep ( const UTF8string& u8str ) noexcept
{
    const UTF8iterator u8end = u8str.utf8_end();
    UTF8iterator it = u8str.utf8_begin();
    UTF8string u8s;

    while ( it != u8end )
    {
        u8s += *it;

        if ( *it == UT8_SEPARATOR )
        {
            UTF8iterator tmp_it = it + 1;
            while ( tmp_it != u8end && *tmp_it == UT8_SEPARATOR )
            {
                ++tmp_it;
            }

            it = tmp_it;
        }
        else
            ++it;
    }

    return u8s;
}

size_t countSeparator ( const UTF8string& u8str ) noexcept
{
    const char SEP = UT8_SEPARATOR.utf8_sstring()[0];
    const std::string& u8string = u8str.utf8_sstring();
    return static_cast<size_t>( std::count( u8string.begin(), u8string.end(), SEP ) );
}

bool checkBasename ( const UTF8string& npath )
{
    // ".", ".." or root directory → return the path
    // No separator → the path itself
    return npath == CURRENT || npath == PARENT || npath == ROOT
           || npath.utf8_find ( UT8_SEPARATOR ) == UTF8string::npos;
}

bool checkDirname ( const UTF8string& npath )
{
    // Empty string
    if ( npath.utf8_empty() )
        return true;

    const size_t POS = npath.utf8_find ( UT8_SEPARATOR );
    const size_t U8LEN = npath.utf8_length();

    // Current directory or parent → current
    // "." or ".." between separators → current directory
    // OR No separator → CURRENT directory
    // OR The path contains at least one separator
    // and the first separator is at the end of string → it is unique
    return npath == CURRENT || npath == PARENT || npath == SCURS
           || npath == SPARS || POS == UTF8string::npos || POS == U8LEN - 1;
}


UTF8string utf8Basename ( const UTF8string& path ) noexcept
{
    const UTF8string npath = removeTrailingSep ( path );

    // Empty string → current
    if ( npath.utf8_empty() )
        return CURRENT;

    // basic checking
    if ( checkBasename ( npath ) )
        return npath;

    // The path contains at least one separator
    const size_t U8LEN = npath.utf8_length();
    const UTF8iterator beg = npath.utf8_begin();
    UTF8iterator it = -- ( npath.utf8_end() );
    size_t spos = 0;
    bool end_sep = false;

    // If the last character is a separator ('/', '\')
    if ( *it == UT8_SEPARATOR )
    {
        end_sep = true;
        spos = U8LEN - 1;
        --it;
    }
    else
        spos = U8LEN;

    while ( *it != UT8_SEPARATOR && it != beg )
    {
        --it;
        --spos;
    }

    // If there is a separator at the beginning
    if ( *it == UT8_SEPARATOR )
    {
        if ( end_sep )
            return npath.utf8_substr ( spos, U8LEN - spos - 1 );
        else
            return npath.utf8_substr ( spos, U8LEN );
    }

    return npath.utf8_substr ( 0, U8LEN - 1 );
}

UTF8string utf8Dirname ( const UTF8string& path ) noexcept
{
    const UTF8string npath = removeTrailingSep ( path );

    // root directory → return the path
    if ( npath == ROOT )
        return npath;

    // Other particular cases
    if ( checkDirname ( npath ) )
        return CURRENT;

    const size_t POS = npath.utf8_find ( UT8_SEPARATOR );
    const size_t U8LEN = npath.utf8_length();

    // If a unique separator was found and is at position 0 → root
    if ( POS == 0 && countSeparator ( npath ) == 1 )
    {
        return ROOT;
    }
    else if ( countSeparator ( npath ) == 1 )
    {
        return npath.utf8_substr ( 0, POS + 1 );
    }

    // At this point, there are more than 1 separators
    const UTF8iterator beg = npath.utf8_begin();
    UTF8iterator it = -- ( npath.utf8_end() );
    size_t spos = U8LEN - 1;

    // Last character == separator → do not count it
    if ( *it == UT8_SEPARATOR )
    {
        --it;
        --spos;
    }

    while ( it != beg && *it != UT8_SEPARATOR )
    {
        --it;
        --spos;
    }

    return npath.utf8_substr ( 0, ( spos == 0 ? spos + 1 : spos ) );
}

// Public

std::string basename( const std::string& path ) noexcept
{
    return utf8Basename( normalize( path ) ).utf8_sstring();
}
std::string dirname( const std::string& path ) noexcept
{
    return utf8Dirname( normalize( path ) ).utf8_sstring();
}

std::string normalize( const std::string& path ) noexcept
{
#if defined(__WIN32__) || defined(__WIN64__)
    const char WIN_SEP = '\\';
    const char UNIX_SEP = '/';
    std::string normalized_path = path;
    std::replace( normalized_path.begin(), normalized_path.end(), WIN_SEP, UNIX_SEP );
    return normalized_path;
#else
    return path;
#endif
}

std::string copyFile( const std::string& source_path, const std::string& destination_path )
{
    std::ifstream input( source_path, std::ios_base::in | std::ios_base::binary );
    if ( !input.is_open() )
    {
        std::cerr << "\nERROR: \"" << source_path << "\" cannot be open." << "\n";
        return "";
    }
    else
    {
        std::ofstream output( destination_path, std::ios_base::out | std::ios_base::binary );
        output << input.rdbuf();
        return destination_path;
    }
}

bool createDir( const std::string& directory ) noexcept
{
    struct stat st;
    const char * dir = directory.c_str();
    if ( stat( dir, &st ) == 0 )
    {
        return true;
    }
    bool result = ( mkdir( dir ) == 0 );
    int code = errno;

    if ( !result )
    {
        std::cerr << "ERROR: Cannot create \"" << dir << "\": " << strerror( code ) << "\n";
    }

    return result;
}

bool hasExtension( const std::string& path, const std::string& extension ) noexcept
{
    return path.substr( path.size() - extension.size() ) == extension;
}

bool exists( const std::string& path ) noexcept
{
    struct stat st;
    const char * file = path.c_str();
    return stat( file, &st ) == 0;
}

}   // fs

