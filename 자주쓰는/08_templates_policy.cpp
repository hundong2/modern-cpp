#include <iostream>
#include <string_view>

struct ConsoleLogger {
    void Log(std::string_view message) const {
        std::cout << "[console] " << message << '\n';
    }
};

struct SilentLogger {
    void Log(std::string_view) const {
        // 아무것도 하지 않는 정책입니다.
        // 최적화 빌드에서는 이 호출이 제거될 가능성이 큽니다.
    }
};

template <typename LoggerPolicy>
class Repository {
public:
    explicit Repository(LoggerPolicy logger = {}) : logger_{logger} {}

    void Save(std::string_view key) {
        logger_.Log(key);
        std::cout << "save: " << key << '\n';
    }

private:
    // 템플릿 정책은 멤버로 직접 들어갑니다.
    // virtual 함수 호출이 없고, 컴파일러가 실제 타입을 알기 때문에 인라이닝이 쉽습니다.
    LoggerPolicy logger_;
};

int main() {
    Repository<ConsoleLogger> with_log;
    with_log.Save("user:1");

    Repository<SilentLogger> without_log;
    without_log.Save("user:2");

    // 실무적 의미:
    // 런타임에 정책을 자주 바꿔야 하면 인터페이스와 virtual도 좋은 선택입니다.
    // 타입별로 고정된 정책이면 템플릿이 분기 비용과 간접 호출 비용을 줄일 수 있습니다.
}

