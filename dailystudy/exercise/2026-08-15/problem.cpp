// <cstdint>는 카운터 값에 사용할 std::uint64_t를 제공한다.
#include <cstdint>
// <iostream>은 결과를 출력하는 std::cout을 제공한다.
#include <iostream>
// <mutex>는 상호 배제 뮤텍스와 RAII 잠금 std::scoped_lock을 제공한다.
#include <mutex>
// <string>은 키 문자열의 메모리를 소유하는 std::string을 제공한다.
#include <string>
// <unordered_map>은 키별 카운터를 평균 O(1)에 찾는 해시 맵을 제공한다.
#include <unordered_map>

// class는 기본 private이며 메트릭 데이터와 동기화 규칙을 함께 캡슐화한다.
class MetricRegistry final {
public:
    // void 반환형은 결과값이 없음을 뜻하고 const 참조는 key를 복사 없이 읽는다.
    void increment(const std::string& key) {
        // scoped_lock은 생성 시 잠그고 함수가 끝날 때 소멸하며 자동으로 잠금을 해제한다.
        std::scoped_lock lock{mutex_};
        ++counts_[key]; // []는 키가 없으면 0 값을 만들고 전위 ++가 값을 1 증가시킨다.
    }

    // const 멤버 함수는 관찰 연산이며 기본 정수 카운터의 값 복사본을 반환한다.
    [[nodiscard]] std::uint64_t value(const std::string& key) const {
        std::scoped_lock lock{mutex_}; // mutable 뮤텍스만 바꾸고 논리적인 카운터 값은 바꾸지 않는다.
        const auto iterator{counts_.find(key)}; // find 표준 함수가 키 위치 또는 end를 돌려준다.
        if (iterator == counts_.end()) { // 조건문이 존재하지 않는 키를 검사한다.
            return 0; // 없는 메트릭은 0이라는 값 정책을 반환한다.
        }
        return iterator->second; // ->로 반복자가 가리키는 pair의 두 번째 값을 읽는다.
    }

private:
    mutable std::mutex mutex_{}; // const 조회도 잠금 상태를 바꿔야 하므로 mutable이다.
    std::unordered_map<std::string, std::uint64_t> counts_{}; // 템플릿 인자는 키와 값 타입이다.
};

// main은 작은 실무형 요청/오류 메트릭 시나리오를 실행한다.
int main() {
    MetricRegistry metrics{}; // 중괄호로 객체를 직접 초기화하며 객체 수명은 main 끝까지다.
    // 이름 있는 metrics는 lvalue이고 점 연산자로 반환값 없는 멤버 함수를 호출한다.
    metrics.increment(std::string{"requests"});
    metrics.increment(std::string{"requests"});
    metrics.increment(std::string{"errors"});
    metrics.increment(std::string{"requests"});

    const std::uint64_t requests{metrics.value(std::string{"requests"})}; // prvalue 문자열은 호출 끝에 파괴된다.
    const std::uint64_t errors{metrics.value(std::string{"errors"})}; // 기본 타입을 중괄호로 직접 초기화한다.
    std::cout << "requests=" << requests << " errors=" << errors << '\n'; // <<가 값을 순서대로 출력한다.
    // 잠금은 로드·저장 순서를 동기화하지만 실제 명령은 CPU·ABI·라이브러리·컴파일러 최적화에 따라 달라진다.
    return requests == 3 && errors == 1 ? 0 : 1; // 비교, 논리곱, 조건 연산자로 종료 코드를 고른다.
}
