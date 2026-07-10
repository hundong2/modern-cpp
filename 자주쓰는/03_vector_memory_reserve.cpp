#include <iostream>
#include <string>
#include <vector>

struct LogLine {
    std::string text;

    explicit LogLine(std::string line) : text{std::move(line)} {
        std::cout << "construct: " << text << '\n';
    }
};

int main() {
    std::vector<LogLine> logs;

    std::cout << "initial capacity=" << logs.capacity() << '\n';

    logs.reserve(4);
    // reserve는 size를 늘리지 않습니다. capacity만 미리 확보합니다.
    // capacity는 재할당 없이 담을 수 있는 원소 개수입니다.
    // 많은 원소를 push할 예정이면 reserve가 힙 재할당 횟수를 줄입니다.
    std::cout << "after reserve capacity=" << logs.capacity() << '\n';

    for (int i = 0; i < 4; ++i) {
        logs.emplace_back("line-" + std::to_string(i));
        // emplace_back은 vector의 내부 메모리 위치에 객체를 직접 생성합니다.
        // push_back(LogLine{...})보다 임시 객체 이동을 줄일 수 있습니다.
    }

    const LogLine* first = &logs[0];
    std::cout << "first address before growth=" << first << '\n';

    logs.emplace_back("line-4");
    // capacity를 초과하면 vector는 더 큰 힙 메모리를 새로 잡고 원소를 이동합니다.
    // 이때 기존 원소 주소, 포인터, 참조, iterator가 무효화됩니다.

    std::cout << "first address after growth =" << &logs[0] << '\n';
    std::cout << "old pointer is now invalid if address changed\n";

    for (const auto& log : logs) {
        std::cout << log.text << '\n';
    }
}

