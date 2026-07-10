#include <boost/beast/http.hpp>

#include <iostream>
#include <string>

namespace http = boost::beast::http;

int main() {
    http::request<http::string_body> request{http::verb::get, "/api/users", 11};
    request.set(http::field::host, "example.com");
    request.set(http::field::user_agent, "boost-beast-example");

    std::cout << request << '\n';

    http::response<http::string_body> response{http::status::ok, request.version()};
    response.set(http::field::content_type, "text/plain");
    response.body() = "hello";
    response.prepare_payload();

    std::cout << response << '\n';

    // Beast는 Asio 위에서 HTTP/WebSocket을 다룹니다.
    // request/response는 헤더 필드와 body 타입을 템플릿으로 조합합니다.
}

