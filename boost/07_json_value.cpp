#define BOOST_JSON_HEADER_ONLY
#include <boost/json.hpp>

#include <iostream>
#include <string>

namespace json = boost::json;

int main() {
    json::value parsed = json::parse(R"({"name":"kim","score":90})");

    json::object& object = parsed.as_object();
    object["active"] = true;
    object["tags"] = json::array{"cpp", "boost"};

    const std::string serialized = json::serialize(parsed);
    std::cout << serialized << '\n';

    // json::value는 null, bool, number, string, array, object 중 하나를 담는 variant 계열 타입입니다.
    // 큰 JSON을 반복 파싱하면 할당 비용이 커질 수 있어 parser 재사용과 메모리 리소스를 검토합니다.
}
