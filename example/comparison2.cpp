#include <compare>

class Int32
{
private:
    /* data */
    int value;
public:
    explicit Int32(int v = 0) : value(v) {}

    //friend bool operator<(const Int32& obj1, const Int32& obj2) { return obj1.value < obj2.value; };
    //friend bool operator<(const Int32& obj1, int n) { return obj1.value < n; };
    //friend bool operator<(int n, const Int32& obj){ return n < obj.value; };
    auto operator<=>(const Int32& other) const { return value <=> other.value; }
    auto operator==(const Int32& other) const { return value <=> other.value; }

    auto operator<=>(int other) const { return value <=> other; }
    auto operator==(int other) const { return value <=> other; }
};

int main()
{
    Int32 n1{20};
    Int32 n2{10};

    bool b1 = n1 < n2;
    bool b2 = n1 < 10;
    bool b3 = 20 < n2;

}