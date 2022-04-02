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

#ifndef PACKAGER_HPP_INCLUDED
#define PACKAGER_HPP_INCLUDED

#include <string>

namespace options
{
struct Options;
}

namespace Packager
{
const std::string pack( const options::Options& options );
const std::string unpack( const options::Options& options );
bool checkPackage( const options::Options& options );
bool packageInfo( const options::Options& options );
};

#endif // PACKAGER_HPP_INCLUDED
