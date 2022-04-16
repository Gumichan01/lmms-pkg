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

namespace program
{

namespace log
{

class Printer
{
protected:
    // Because you cannot manipulate abstract classes by value,
    // I have to use the Printer as a concrete class but with a protected constructor
    // Its virtual methods are default methods, but they are never called.
    Printer() = default;
    Printer& operator =( const Printer& printer ) = default;

public:
    virtual Printer& operator <<( const std::string& text ) noexcept;
    virtual Printer& operator <<( const long num ) noexcept;
    virtual ~Printer() = default;
};

class FakePrinter final: public Printer
{
public:
    FakePrinter() = default;
    FakePrinter& operator =( const FakePrinter& printer ) noexcept;
    virtual Printer& operator <<( const std::string& text ) noexcept;
    virtual Printer& operator <<( const long num ) noexcept;
    virtual ~FakePrinter() = default;
};

class VerbosePrinter final: public Printer
{
private:
    static constexpr std::ostream& os = std::cout;

public:
    VerbosePrinter() = default;
    VerbosePrinter& operator =( const VerbosePrinter& printer ) noexcept;
    virtual Printer& operator <<( const std::string& text ) noexcept;
    virtual Printer& operator <<( const long num ) noexcept;
    virtual ~VerbosePrinter() = default;
};

bool setVerbose( bool v ) noexcept;
Printer getPrinter() noexcept;

}

}

#endif // LOGGER_HPP_INCLUDED
