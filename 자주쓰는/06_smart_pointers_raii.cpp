#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

class Connection {
public:
    explicit Connection(std::string endpoint) : endpoint_{std::move(endpoint)} {
        std::cout << "connect " << endpoint_ << '\n';
    }

    ~Connection() {
        std::cout << "disconnect " << endpoint_ << '\n';
    }

    void Send(std::string_view message) {
        std::cout << endpoint_ << " <- " << message << '\n';
    }

private:
    std::string endpoint_;
};

std::unique_ptr<Connection> MakeConnection() {
    // unique_ptr는 단독 소유권을 표현합니다.
    // new/delete를 직접 쓰지 않고 make_unique를 사용하면 예외 안전성이 좋아집니다.
    return std::make_unique<Connection>("cache-server");
}

int main() {
    auto connection = MakeConnection();
    connection->Send("ping");

    auto shared = std::make_shared<Connection>("shared-service");
    std::weak_ptr<Connection> weak = shared;

    {
        std::shared_ptr<Connection> another_owner = shared;
        // shared_ptr는 참조 카운트를 힙의 제어 블록에 저장합니다.
        // 복사할 때마다 카운트가 증가하고, 파괴될 때 감소합니다.
        // 편하지만 atomic 카운트 갱신 비용과 순환 참조 위험이 있습니다.
        another_owner->Send("hello");
    }

    if (auto locked = weak.lock()) {
        // weak_ptr는 소유하지 않고 관찰만 합니다.
        // lock이 성공하면 아직 객체가 살아 있다는 뜻입니다.
        locked->Send("still alive");
    }

    shared.reset();

    if (weak.expired()) {
        std::cout << "shared-service is gone\n";
    }
}
