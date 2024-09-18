#include "FooExample.hpp"

using namespace std;

Foo::Foo(double value) : m_value { value }
{
    cout << "Foo::m_value: " << m_value << endl;    
}

MyClassFoo::MyClassFoo(double value) : m_value { value }, m_foo { m_value }
{
    cout << "MyClass::m_value: " << m_value << endl;
}

MyClassFooReverse::MyClassFooReverse(double value) : m_value { value }, m_foo { m_value }
{
    cout << "MyClass::m_value: " << m_value << endl;
}
MyClassSolve::MyClassSolve(double value) : m_value { value }, m_foo { value }
{
    cout << "MyClass::m_value: " << m_value << endl;
}