#include <concepts>
#include <vector>

#include <type_traits>

template<typename T>
concept LessThanComparable1 = requires(T a, T b)
{
    a < b;
};

template<typename T>
concept LessThanComparable2 = requires(T a, T b)
{
    {a < b} -> std::convertible_to<bool>;
};

template<typename T>
concept Equality = requires(T a, T b)
{
    {a == b} -> std::convertible_to<bool>;
    {a != b} -> std::convertible_to<bool>;
};

template<typename T>
concept Container = requires(T c)
{
    c.begin();
    c.end();
};

template<typename T>
concept HasValueType = requires
{
    typename T::value_type;
};

template<typename T> requires Container<T>
void foo(T a){}

template<typename T> requires HasValueType<T>
void goo(T a) {}

int main()
{
    std::vector<int> v = {1,2,3};
    foo(v);
    goo(v);
}