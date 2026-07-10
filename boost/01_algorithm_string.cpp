#include <boost/algorithm/string.hpp>

#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string raw = "  GET, /api/users, application/json  ";

    const std::string trimmed = boost::algorithm::trim_copy(raw);
    // trim_copy는 원본 raw를 바꾸지 않고 새 string을 반환합니다.
    // 큰 문자열을 반복 처리한다면 복사 비용을 의식해야 합니다.

    std::vector<std::string> parts;
    boost::algorithm::split(parts, trimmed, boost::is_any_of(","));

    for (auto& part : parts) {
        boost::algorithm::trim(part);
        // trim은 in-place 수정입니다. 새 문자열 할당을 줄일 수 있습니다.
    }

    const std::string method = boost::algorithm::to_lower_copy(parts[0]);
    const std::string normalized = boost::algorithm::join(parts, " | ");

    std::cout << "method=" << method << '\n';
    std::cout << normalized << '\n';
}

