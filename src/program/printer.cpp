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

namespace program
{

namespace log
{

// I don't want to bring a printer in every function calls. Too many modifications just for that!
// I will keep this simple by using those shitty global variables and generate a printer when it is requested
namespace
{
    bool verbose = false;
}

Printer& Printer::operator <<( const std::string& ) noexcept
{
    return *this;
}

Printer& Printer::operator <<( const long ) noexcept
{
    return *this;
}

// Fake Printer

Printer& FakePrinter::operator <<( const std::string& ) noexcept
{
    return *this;
}

Printer& FakePrinter::operator <<( const long ) noexcept
{
    return *this;
}

// Verbose Printer

Printer& VerbosePrinter::operator <<( const std::string& text ) noexcept
{
    os << text;
    return *this;
}

Printer& VerbosePrinter::operator <<( const long num ) noexcept
{
    os << num;
    return *this;
}

bool setVerbose( bool v ) noexcept
{
    verbose = v;
    return v;
}

Printer getPrinter() noexcept
{
    if ( verbose )
    {
        return VerbosePrinter();
    }
    else
    {
        return FakePrinter();
    }
}


} // log
} // program
