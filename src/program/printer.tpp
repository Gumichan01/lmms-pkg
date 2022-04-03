
template <typename T>
Printer& Printer::operator <<( const T& text ) noexcept
{
    if ( verbose )
    {
        os << text;
    }
    return *this;
}
