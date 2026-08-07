// <memory>는 std::unique_ptr와 std::make_unique를 제공한다.
#include <memory>
// <string>은 소유 문자열 타입을 제공한다.
#include <string>
// <utility>는 std::move를 제공한다.
#include <utility>
// <vector>는 연속 저장 동적 배열을 제공한다.
#include <vector>

class Sink {
public:
    virtual ~Sink() = default; // 생성자와 달리 소멸자는 ~가 붙고, 가상 소멸로 파생 객체를 안전히 정리한다.
    virtual void write(std::string text) = 0; // 값 매개변수는 호출에서 문자열 소유권을 함수로 옮길 수 있다.
};

class MemorySink final : public Sink {
public:
    void write(std::string text) override {
        lines_.push_back(std::move(text)); // push_back이 xvalue 문자열 자원을 벡터 원소로 이동한다.
    }
    [[nodiscard]] const std::vector<std::string>& lines() const {
        return lines_; // const 참조 반환은 복사 없이 읽는 비소유 별칭이며 객체보다 오래 쓰면 안 된다.
    }
private:
    std::vector<std::string> lines_{}; // 템플릿 인자 string인 벡터가 여러 문자열을 소유한다.
};

class Logger {
public:
    explicit Logger(std::unique_ptr<Sink> sink) : sink_{std::move(sink)} {} // 복사는 금지되고 소유권만 이동한다.
    void log(std::string text) { sink_->write(std::move(text)); } // 매 단계에서 문자열 소유권을 전달한다.
private:
    std::unique_ptr<Sink> sink_{};
};

int main() {
    auto concrete{std::make_unique<MemorySink>()}; // prvalue unique_ptr로 파생 객체 수명을 시작한다.
    MemorySink* observer{concrete.get()}; // 원시 포인터는 소유하지 않고 잠시 관찰만 한다.
    Logger logger{std::move(concrete)}; // 직접 초기화로 Logger에 유일한 소유권을 넘긴다.
    logger.log(std::string{"first"}); // 임시 문자열 prvalue는 매개변수를 거쳐 저장소로 이동한다.
    const bool ok{observer->lines().size() == 1U}; // size_t 값과 unsigned 리터럴을 ==로 비교한다.
    // observer의 유효성은 logger가 Sink를 소유하는 동안뿐이다. 실제 호출/메모리 동작은 구현과 최적화에 따라 달라진다.
    return ok ? 0 : 1;
}
