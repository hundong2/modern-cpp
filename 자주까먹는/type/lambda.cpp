#include <iostream>

namespace Console
{
    template <typename... Args>
    void WriteLine(Args&&... args) {
        // 1. 들어온 인자들을 띄어쓰기(혹은 그냥)와 함께 쭉 출력합니다.
        (std::cout << ... << std::forward<Args>(args)) << std::endl;
        // 2. 구분선을 출력합니다.
        std::cout << "--------------------" << std::endl;
}
}


int main()
{

    auto my_lambda = [](int x) { return x = 2; };
    auto result = my_lambda(5);
    Console::WriteLine("람다 호출 결과: ", result);
}