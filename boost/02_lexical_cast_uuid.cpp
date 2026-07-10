#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iostream>
#include <string>

int main() {
    try {
        const int port = boost::lexical_cast<int>("8080");
        const double ratio = boost::lexical_cast<double>("0.75");

        boost::uuids::random_generator generator;
        const boost::uuids::uuid request_id = generator();

        std::cout << "port=" << port << '\n';
        std::cout << "ratio=" << ratio << '\n';
        std::cout << "request_id=" << request_id << '\n';
    } catch (const boost::bad_lexical_cast& error) {
        std::cerr << "conversion failed: " << error.what() << '\n';
    }

    // lexical_cast는 문자열 변환 코드가 짧아지지만 실패 시 예외를 던집니다.
    // 고성능 hot path에서는 std::from_chars 같은 무할당 변환도 비교해야 합니다.
}

