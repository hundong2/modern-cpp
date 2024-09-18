#ifndef FOOEXAMPLE_HPP
#define FOOEXAMPLE_HPP

#include <iostream>

class Foo 
{
    public:
        Foo(double value);
    private:
        double m_value { 0 };
};

class MyClassFoo
{
    public:
        MyClassFoo(double value);
    private:
        double m_value { 0 };
        Foo m_foo;
};

class MyClassFooReverse
{
    public:
        MyClassFooReverse(double value);
    private:
        Foo m_foo;
        double m_value { 0 };
};

class MyClassSolve
{
    public:
        MyClassSolve(double value);
    private:
        double m_value { 0 };
        Foo m_foo;
};

#endif // FOOEXAMPLE_HPP