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


#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#include<string>
#include<iostream>

namespace Program
{

class Printer
{
private:
    bool verbose;
    static constexpr std::ostream& os = std::cout;

public:
    Printer( bool v );
    Printer& operator =( const Printer& printer ) noexcept;
    template <typename T>
    Printer& operator <<( const T& text ) noexcept;
    ~Printer();
};

bool setVerbose( bool v ) noexcept;
Printer getPrinter() noexcept;

#include "printer.tpp"

}

#endif // LOGGER_HPP_INCLUDED