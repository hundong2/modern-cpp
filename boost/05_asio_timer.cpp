#include <boost/asio.hpp>

#include <chrono>
#include <iostream>

int main() {
    boost::asio::io_context io;

    boost::asio::steady_timer timer{io, std::chrono::milliseconds{10}};

    timer.async_wait([](const boost::system::error_code& error) {
        if (error) {
            std::cout << "timer cancelled: " << error.message() << '\n';
            return;
        }

        std::cout << "timer fired\n";
    });

    std::cout << "before run\n";
    io.run();
    std::cout << "after run\n";

    // async_wait는 콜백을 등록만 합니다.
    // 실제 콜백 실행은 io_context::run()이 이벤트 루프를 돌 때 일어납니다.
    // 네트워크 코드도 같은 모델로 socket read/write 완료 콜백을 실행합니다.
}

