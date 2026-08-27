// <cstddef>는 결과 줄 수를 표현하는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 std::ostream 출력 포트와 std::cout을 선언한다.
#include <iostream>
// <sstream>은 검증 가능한 메모리 출력 스트림 std::ostringstream를 선언한다.
#include <sstream>
// <string>은 완성된 로그 스냅샷을 소유한다.
#include <string>
// <string_view>는 호출 동안만 이름 문자를 빌리는 std::string_view를 선언한다.
#include <string_view>
// <syncstream>은 한 레코드를 모아 내보내는 std::osyncstream을 선언한다.
#include <syncstream>
// <thread>는 RAII 스레드 소유자 std::jthread를 선언한다.
#include <thread>
// <vector>는 실행 중 정해지는 스레드 수를 소유한다.
#include <vector>

// class 기본 private 접근으로 출력 구현을 숨기고 public 동작만 노출한다.
class MetricWriter {
public:
    // 생성자에는 반환형이 없다. explicit은 ostream에서 MetricWriter로의 뜻밖의 암시 변환을 차단한다.
    explicit MetricWriter(std::ostream& output) noexcept
        : output_{output} {} // output lvalue를 비소유 참조 멤버에 바인딩한다.

    // name은 비소유 뷰라 호출 동안 원본 문자가 살아 있어야 하고, value는 int 값 복사다.
    void write(std::string_view name, int value) const {
        // osyncstream(ostream&)은 output_의 streambuf를 감싼 동기 출력 버퍼를 소유한다.
        // output_은 ostream lvalue 참조이며 이동·복사되지 않는다. 생성자는 반환값이 없고 할당 실패 예외가 가능하다.
        std::osyncstream line{output_};
        // 각 operator<<는 같은 스트림 참조를 반환한다. name의 문자는 복사 소유하지 않고 즉시 읽고 value는 값으로 형식화한다.
        // 호출 뒤 line 버퍼에 한 줄이 쌓이고 output_은 소멸 시 emit될 때 갱신된다. 시간·공간은 기록 문자 수에 선형이다.
        line << name << '=' << value << '\n';
    }

private:
    // 참조 멤버는 출력 스트림 수명을 소유하지 않는다.
    std::ostream& output_;
};

// 문자열 안의 개행 수를 세며 입력 string은 const&로 빌려 변경하지 않는다.
[[nodiscard]] std::size_t count_lines(const std::string& text) {
    std::size_t count{};
    for (const char character : text) {
        if (character == '\n') {
            ++count;
        }
    }
    return count;
}

int main() {
    // 빈 문자열 버퍼를 소유하는 ostringstream과 그 버퍼를 빌리는 writer를 직접 초기화한다.
    std::ostringstream output{};
    MetricWriter writer{output};
    // 빈 vector는 jthread 객체와 실행 스레드의 수명을 이후 원소로 소유한다.
    std::vector<std::jthread> workers{};

    // reserve(2)의 인자는 size_type으로 변환 가능한 정수 prvalue다. void 반환은 무시한다.
    // 성공 뒤 size는 0, capacity는 2 이상이며 뒤 두 emplace에서 재할당과 참조 무효화를 피한다.
    workers.reserve(2U);
    // emplace_back은 lambda prvalue를 jthread 생성자에 전달하고 jthread& 반환은 무시한다.
    // writer는 참조 캡처라 스레드보다 오래 살아야 한다. "latency"는 정적 수명 문자 배열이고 string_view가 빌려도 안전하다.
    workers.emplace_back([&writer] { writer.write("latency", 12); });
    // 두 번째 호출도 새 실행 스레드를 즉시 시작하며 성공 뒤 workers.size()는 2다.
    workers.emplace_back([&writer] { writer.write("throughput", 34); });

    for (std::jthread& worker : workers) {
        // joinable()은 bool 관찰만 하고, join()은 void를 반환하며 성공 뒤 소유 실행을 끝낸 non-joinable 상태로 만든다.
        if (worker.joinable()) {
            worker.join();
        }
    }

    // str()은 output의 현재 문자를 새 string prvalue로 복사하고 output을 유지한다.
    const std::string text{output.str()};
    // find(const char*)는 각 패턴 첫 위치 또는 npos를 반환하고 text를 바꾸지 않는다. 각 호출은 최악 O(text.size())다.
    const bool complete{text.find("latency=12\n") != std::string::npos &&
                        text.find("throughput=34\n") != std::string::npos};
    // count_lines는 text lvalue를 const&로 빌리며 size_t 값을 반환한다.
    const std::size_t lines{count_lines(text)};
    // 삽입 연산은 결과를 cout에 쓰고 반환 ostream&는 무시한다.
    std::cout << lines << ' ' << complete << '\n';
    return lines == 2U && complete ? 0 : 1;
}
