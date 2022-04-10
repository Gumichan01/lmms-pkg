

namespace xml
{

template<typename T>
const std::vector<T *> getAllElementsByNames( const tinyxml2::XMLElement * root, const std::vector<std::string>& names )
{
    std::vector<T *> retrieved_elements;
    T * element = const_cast<tinyxml2::XMLElement*>(root->FirstChildElement());

    while ( element != nullptr )
    {
        if ( std::find( names.cbegin(), names.cend(), std::string( element->Name() ) ) != names.cend() )
        {
            retrieved_elements.push_back( element );
        }
        const std::vector<T *>& elements = getAllElementsByNames<T>( element, names );
        retrieved_elements.insert( retrieved_elements.end(), elements.cbegin(), elements.cend() );
        element = element->NextSiblingElement();
    }

    return retrieved_elements;
}

}
