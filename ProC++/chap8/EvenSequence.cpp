#include "EvenSequence.hpp"

EvenSequence::EvenSequence(std::initializer_list<double> args)
{
    if( args.size() % 2 != 0 )
    {
       throw std::invalid_argument("initializer_list should have even number of elements");
    }
    /*
    //old code 
    m_sequence.reserve(args.size());
    for( const auto& value : args )
    {
        m_sequence.push_back(value);
    }
    */
   m_sequence.assign(args);
    
}

void EvenSequence::dump() const
{
    for( const auto& value : m_sequence )
    {
        std::cout << value << ",";
    }
    std::cout << std::endl;
}