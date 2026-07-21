#include <cstdio> // C 파일 핸들 획득과 반환을 커스텀 deleter로 감싸기 위해 포함한다.
#include <iostream> // 생성과 소멸 순서를 표준 출력에서 관찰하기 위해 포함한다.
#include <memory> // std::unique_ptr에 FILE 핸들의 유일 소유권을 맡기기 위해 포함한다.
#include <mutex> // std::lock_guard가 관리할 상호 배제 자원을 제공한다.
#include <stdexcept> // 실패 경로에서 스택 해제 동작을 확인할 예외 타입을 제공한다.
#include <string_view> // 수명 비소유 문자열을 복사 없이 Trace에 전달하기 위해 포함한다.
#include <thread> // 같은 카운터를 갱신하는 여러 실행 흐름을 만들기 위해 포함한다.
#include <utility> // std::move로 유일 소유 핸들을 이전하기 위해 포함한다.
#include <vector> // join할 스레드 객체를 연속 컨테이너에서 소유하기 위해 포함한다.

class Trace final { // 자동 객체의 생성과 역순 소멸을 눈으로 확인하는 진단 타입을 정의한다.
public: // 생성과 소멸 로그에 필요한 인터페이스를 공개한다.
    explicit Trace(const std::string_view name) noexcept : name_{name} { // 문자열 리터럴의 수명이 충분하다는 전제로 비소유 뷰를 저장한다.
        std::cout << "+ construct " << name_ << '\n'; // 객체 수명이 시작된 시점을 출력한다.
    } // 생성자 본문을 끝내며 이미 획득한 별도 자원은 없다.
    ~Trace() noexcept { std::cout << "- destroy " << name_ << '\n'; } // 정상 종료와 스택 해제 모두에서 수명 종료를 기록한다.
    Trace(const Trace&) = delete; // 로그 객체 복제로 소멸 순서가 흐려지지 않도록 복사를 금지한다.
    Trace& operator=(const Trace&) = delete; // 같은 이유로 복사 대입도 금지한다.

private: // 관찰 이름이 외부에서 바뀌지 않도록 표현을 숨긴다.
    std::string_view name_; // 문자열을 소유하지 않으므로 가리키는 대상이 Trace보다 오래 살아야 한다.
}; // Trace 타입 정의를 끝낸다.

struct FileCloser final { // std::FILE*에 맞는 반환 정책을 함수 객체로 정의한다.
    void operator()(std::FILE* const file) const noexcept { // unique_ptr 소멸자가 호출할 예외 없는 정리 함수를 제공한다.
        if (file != nullptr) { // null 핸들에는 fclose를 호출할 수 없으므로 먼저 검사한다.
            std::cout << "- close FILE handle\n"; // 실제 반환 직전에 관찰 메시지를 출력한다.
            static_cast<void>(std::fclose(file)); // C 런타임에 핸들을 반환하며 소멸자에서는 오류를 던지지 않는다.
        } // 유효 핸들 정리 분기를 끝낸다.
    } // 커스텀 deleter 호출을 끝낸다.
}; // FileCloser 정책 타입 정의를 끝낸다.

using FileHandle = std::unique_ptr<std::FILE, FileCloser>; // FILE* 주소와 fclose 정책을 하나의 유일 소유 타입으로 묶는다.

[[nodiscard]] FileHandle open_temporary_file() { // 성공한 핸들만 호출자에게 반환하는 팩토리 함수를 정의한다.
    std::FILE* const raw = std::tmpfile(); // C API로 운영체제 파일 자원을 획득하고 raw 핸들을 잠시 받는다.
    if (raw == nullptr) { // 자원 획득 실패를 유효 객체 생성과 구분한다.
        throw std::runtime_error{"failed to open a temporary file"}; // 소유 객체를 만들기 전에 실패를 값이 아닌 예외로 전달한다.
    } // 획득 실패 분기를 끝낸다.
    std::cout << "+ open FILE handle\n"; // 핸들 수명이 시작됐음을 출력한다.
    return FileHandle{raw}; // 즉시 RAII 소유자에 넣어 이후 모든 정상 해제 경로를 단일화한다.
} // 반환값 최적화 또는 unique_ptr 이동으로 소유권만 호출자에게 이전한다.

void write_record(const bool fail_after_write) { // 정상 반환과 예외 반환에서 동일한 정리가 일어나는지 보여 준다.
    Trace function_scope{"write_record scope"}; // 가장 먼저 만든 자동 객체는 이 함수에서 가장 나중에 파괴된다.
    auto file = open_temporary_file(); // 파일 핸들을 획득하자마자 FileHandle의 유일 소유권으로 감싼다.
    Trace inner_scope{"inner object"}; // 파일보다 나중에 생성되어 스코프를 떠날 때 파일보다 먼저 파괴된다.
    if (std::fputs("RAII\n", file.get()) < 0) { // 소유권을 유지한 채 raw 핸들을 잠시 빌려 쓰기 결과를 검사한다.
        throw std::runtime_error{"failed to write"}; // 실패해도 스택 해제가 inner, file, function 순으로 정리한다.
    } // 쓰기 실패 검사를 끝낸다.
    if (fail_after_write) { // 교육용 예외 경로를 호출자가 선택할 수 있게 한다.
        throw std::runtime_error{"simulated processing failure"}; // throw가 현재 스코프의 자동 객체 소멸을 시작하게 한다.
    } // 모의 실패 분기를 끝낸다.
    std::cout << "write completed normally\n"; // 정상 경로도 함수 끝에서 같은 역순 정리를 수행함을 보여 준다.
} // return을 명시하지 않아도 모든 지역 RAII 객체를 파괴한 뒤 호출자에게 돌아간다.

class SafeCounter final { // mutex와 보호 대상 데이터를 한 객체에 묶어 잠금 규칙을 캡슐화한다.
public: // 데이터 레이스 없이 값을 더하고 읽는 연산만 공개한다.
    void add(const int delta) { // 여러 스레드가 동시에 호출할 수 있는 갱신 연산을 정의한다.
        const std::lock_guard<std::mutex> lock{mutex_}; // 생성자가 lock하고 소멸자가 unlock하는 범위 잠금을 얻는다.
        if (delta == 0) { // 조기 반환도 lock_guard의 소멸자를 반드시 거친다는 것을 보여 준다.
            return; // 현재 블록을 떠나기 전에 lock이 파괴되어 mutex가 자동으로 풀린다.
        } // 조기 반환 분기를 끝낸다.
        value_ += delta; // 잠금이 유지된 임계 구역에서 공유 정수를 안전하게 갱신한다.
    } // lock이 역순으로 파괴되며 mutex의 unlock을 호출한다.
    [[nodiscard]] int value() const { // 현재 카운터를 데이터 레이스 없이 읽는다.
        const std::lock_guard<std::mutex> lock{mutex_}; // const 함수에서도 동기화하도록 mutable mutex를 잠근다.
        return value_; // 잠금 중 읽은 작은 정수 값을 호출 규약의 반환 레지스터로 전달할 수 있다.
    } // 반환값을 보존한 채 lock_guard가 파괴되어 mutex를 푼다.

private: // mutex를 거치지 않은 데이터 접근을 외부 코드에서 차단한다.
    mutable std::mutex mutex_; // 논리적 값이 아니라 동기화 상태이므로 const 관찰에서도 변경 가능하게 둔다.
    int value_{0}; // 반드시 mutex_를 보유한 상태에서만 읽고 쓰는 공유 상태다.
}; // SafeCounter 타입 정의를 끝낸다.

int main() { // 운영체제가 호출하는 프로그램 진입점에서 RAII 시나리오를 순서대로 실행한다.
    std::cout << "[normal path]\n"; // 정상 함수 종료 관찰 구역을 표시한다.
    write_record(false); // 명시적 close 없이도 함수 끝에서 파일 핸들이 반환된다.
    std::cout << "[exception path]\n"; // 예외 스택 해제 관찰 구역을 표시한다.
    try { // 모의 오류를 처리해 프로그램이 계속 진행하도록 예외 경계를 연다.
        write_record(true); // 파일 획득 뒤 throw되어도 완성된 지역 객체들이 역순 파괴된다.
    } catch (const std::exception& error) { // 표준 예외 계층을 비소유 const 참조로 받아 slicing과 복사를 피한다.
        std::cout << "caught: " << error.what() << '\n'; // 스택 해제가 끝난 뒤 catch가 실행됨을 출력한다.
    } // 예외 처리 구역을 끝낸다.
    SafeCounter counter; // mutex와 정수를 함께 소유하는 자동 객체를 생성한다.
    std::vector<std::thread> workers; // join 가능한 OS 스레드 핸들을 컨테이너가 소유한다.
    workers.reserve(4U); // 스레드 객체 삽입 중 컨테이너 재할당을 피한다.
    for (int index{0}; index < 4; ++index) { // 같은 카운터를 갱신할 네 작업자를 만든다.
        workers.emplace_back([&counter]() { // counter를 비소유 참조로 캡처하고 main이 join까지 수명을 보장한다.
            for (int iteration{0}; iteration < 1000; ++iteration) { // 각 스레드가 천 번 임계 구역에 진입한다.
                counter.add(1); // lock_guard가 매 호출에서 mutex 획득과 반환을 예외 안전하게 묶는다.
            } // 현재 작업자의 반복을 끝낸다.
        }); // 람다를 새 OS 스레드의 진입 함수로 전달한다.
    } // 네 스레드 생성을 끝낸다.
    for (auto& worker : workers) { // 소유 컨테이너의 모든 joinable 스레드를 순회한다.
        worker.join(); // std::thread는 C++17에서 자동 join하지 않으므로 명시적으로 완료를 기다린다.
    } // 모든 스레드가 끝났으므로 counter 참조 캡처의 수명이 안전함을 확정한다.
    std::cout << "counter=" << counter.value() << '\n'; // lock으로 보호된 결정적 결과 4000을 출력한다.
    return 0; // 정상 종료하며 workers, counter가 선언의 역순으로 파괴된다.
} // main의 자동 저장 기간 객체 수명과 프로그램 실행을 끝낸다.
