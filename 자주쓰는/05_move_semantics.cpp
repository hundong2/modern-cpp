#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Buffer {
public:
    explicit Buffer(std::size_t size) : data_(size, 0) {
        std::cout << "construct size=" << data_.size() << '\n';
    }

    Buffer(const Buffer& other) : data_{other.data_} {
        // 복사는 other.data_의 원소들을 새 힙 메모리에 복제합니다.
        std::cout << "copy\n";
    }

    Buffer& operator=(const Buffer& other) {
        std::cout << "copy assign\n";
        data_ = other.data_;
        return *this;
    }

    Buffer(Buffer&& other) noexcept : data_{std::move(other.data_)} {
        // 이동은 vector 내부의 힙 버퍼 소유권을 가져옵니다.
        // 원소 하나하나를 복사하지 않고 포인터/크기/용량 같은 관리 정보가 이동됩니다.
        std::cout << "move\n";
    }

    Buffer& operator=(Buffer&& other) noexcept {
        std::cout << "move assign\n";
        data_ = std::move(other.data_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const {
        return data_.size();
    }

private:
    std::vector<int> data_;
};

Buffer MakeBuffer() {
    Buffer buffer{1000};
    return buffer;
    // C++17 이후 많은 경우 복사 생략이 적용됩니다.
    // 컴파일러가 가능하면 buffer를 호출자 위치에 직접 만들 수 있습니다.
}

int main() {
    Buffer a = MakeBuffer();
    std::cout << "a.size=" << a.size() << '\n';

    Buffer b{10};
    b = std::move(a);
    // std::move는 실제 이동을 수행하는 함수가 아니라
    // "이 객체에서 자원을 가져가도 된다"는 우측값 캐스팅입니다.
    // 이동 후 a는 유효하지만 값은 보장하지 않는 상태입니다.

    std::cout << "b.size=" << b.size() << '\n';
}

