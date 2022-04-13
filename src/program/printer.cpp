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

#include "printer.hpp"

#include <iostream>

namespace Program
{

Printer::Printer( bool v )
    : verbose( v )
{
// Empty
}

Printer& Printer::operator =( const Printer& printer ) noexcept
{
    verbose = printer.verbose;
    return *this;
}

Printer::~Printer()
{
    // Empty
}

// I don't want to bring a printer in every functions call. Too many modifications just for that!
// I will keep this simple by using this shitty global variable
namespace
{
    Printer printer( false );
}


bool setVerbose( bool v ) noexcept
{
    printer = Printer( v );
    return v;
}

Printer getPrinter() noexcept
{
    return printer;
}

}
