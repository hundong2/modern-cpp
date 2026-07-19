#include <iomanip>
#include <iostream>
#include <string>

int main()
{
    const std::string item = "notebook";
    const int quantity = 3;
    const double unit_price = 12500.0;
    const double discount_rate = 0.10;

    const double subtotal = quantity * unit_price;
    const double discount = subtotal * discount_rate;
    const double total = subtotal - discount;

    std::cout << std::fixed << std::setprecision(0);
    std::cout << "item      : " << item << '\n';
    std::cout << "quantity  : " << quantity << '\n';
    std::cout << "subtotal  : " << subtotal << '\n';
    std::cout << "discount  : " << discount << '\n';
    std::cout << "total     : " << total << '\n';

    return 0;
}
