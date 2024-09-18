#ifndef EVEN_SEQUENCE_HPP
#define EVEN_SEQUENCE_HPP

#include <iostream>
#include <initializer_list>
#include <vector>

class EvenSequence 
{
public:
    EvenSequence(std::initializer_list<double> args);
    void dump() const;
private:
    std::vector<double> m_sequence;

};

#endif // EVEN_SEQUENCE_HPP