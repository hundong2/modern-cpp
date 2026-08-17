// <cstdint>는 버전 번호에 쓸 고정 폭 부호 없는 정수 std::uint64_t를 제공한다.
#include <cstdint>
// <iostream>은 표준 출력 스트림 std::cout을 제공한다.
#include <iostream>
// <mutex>는 쓰기 잠금을 소유하는 std::unique_lock을 제공한다.
#include <mutex>
// <optional>은 조회 결과가 없을 수 있음을 나타내는 std::optional을 제공한다.
#include <optional>
// <shared_mutex>는 읽기-쓰기 뮤텍스와 읽기 잠금 std::shared_lock을 제공한다.
#include <shared_mutex>
// <string>은 문자열을 소유하는 std::string을 제공한다.
#include <string>
// <unordered_map>은 평균 O(1) 키 조회를 제공하는 해시 맵이다.
#include <unordered_map>
// <utility>는 이름 있는 객체를 이동 후보인 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>

// struct는 기본 접근이 public이며 구성 값과 버전을 단순한 값 객체로 묶는다.
struct ConfigEntry {
    std::string value{};     // std::string 멤버가 문자 메모리를 직접 소유한다.
    std::uint64_t version{}; // 중괄호 초기화는 기본값을 0으로 만든다.
};

// using은 긴 템플릿 타입에 읽기 쉬운 별칭을 붙이고 ConfigEntry는 템플릿 인자다.
using ConfigMap = std::unordered_map<std::string, ConfigEntry>;

// class는 기본 접근이 private이며 public 영역에는 저장소 계약만 노출한다.
class IConfigRepository {
public:
    // 생성자·소멸자에는 반환형이 없다. 가상 소멸자는 기반 포인터 삭제도 안전하게 한다.
    virtual ~IConfigRepository() = default;
    // 매개변수를 값으로 받아 구현이 문자열 소유권을 이동해 보관할 수 있게 한다.
    virtual std::uint64_t put(std::string key, std::string value) = 0;
    // const 함수는 논리적으로 저장 값을 바꾸지 않으며, 없음을 optional로 표현한다.
    [[nodiscard]] virtual std::optional<ConfigEntry> find(const std::string& key) const = 0;
};

// final은 더 파생하지 않는 구현 경계를 나타내고 public 상속은 인터페이스 계약을 구현한다.
class ThreadSafeConfigRepository final : public IConfigRepository {
public:
    // explicit은 ConfigMap 하나가 저장소로 암시 변환되는 것을 막는다. 생성자에는 반환형이 없다.
    explicit ThreadSafeConfigRepository(ConfigMap initial)
        // 멤버 초기화 목록은 본문 전에 initial의 자원을 entries_로 이동한다.
        : entries_{std::move(initial)} {}

    // override는 기반 가상 함수와 서명이 맞는지 컴파일러가 검사하게 한다.
    std::uint64_t put(std::string key, std::string value) override {
        // unique_lock lock{mutex_} 생성자는 shared_mutex& 하나를 입력받아 독점 잠금을 소유하며 생성자는 반환값이 없다.
        // 잠금 획득은 대기할 수 있고, lock 소멸자가 예외·정상 반환 모두에서 자동 unlock한다.
        std::unique_lock lock{mutex_};
        ++version_; // 전위 ++ 연산자로 잠금 안에서 버전을 1 증가시킨다.
        // insert_or_assign(key,value)는 이동할 키와 ConfigEntry 두 입력을 받고 pair<iterator,bool>을 반환한다(여기서는 버림).
        // bool은 새 삽입 여부이고, 성공하면 해당 키 값이 version_으로 갱신된다. 평균 O(1), 재해시 시 반복자가 무효화된다.
        entries_.insert_or_assign(std::move(key), ConfigEntry{std::move(value), version_});
        return version_; // 기본 정수 값을 호출자에게 복사해 반환한다.
    }

    // optional 값 반환형과 const 참조 매개변수로 조회 결과의 존재 여부와 비소유 입력을 표현한다.
    [[nodiscard]] std::optional<ConfigEntry> find(const std::string& key) const override {
        // shared_lock 생성자는 shared_mutex&를 입력받아 공유 잠금을 소유하고 반환값은 없다. 소멸 시 자동 해제한다.
        std::shared_lock lock{mutex_};
        // unordered_map::find(key)는 키 const& 하나를 입력받아 원소 반복자 또는 end()를 반환하고 map은 바뀌지 않는다.
        const auto iterator{entries_.find(key)};
        // end()는 인자 없이 one-past-end 반복자를 반환한다. 두 반복자 비교로 부재를 판정하며 평균 조회는 O(1)이다.
        if (iterator == entries_.end()) {
            return std::nullopt; // 값 없음 상태인 prvalue를 반환한다.
        }
        // 내부 참조를 노출하지 않고 ConfigEntry를 복사한 스냅샷으로 반환해 잠금 밖 수명을 분리한다.
        return iterator->second;
    }

private:
    // const find에서도 동기화 상태는 바꿔야 하므로 mutable을 붙인다. 데이터의 논리 값은 바꾸지 않는다.
    mutable std::shared_mutex mutex_{};
    ConfigMap entries_{};       // private 해시 맵이 구성 문자열들의 수명을 소유한다.
    std::uint64_t version_{1};  // 초기 데이터의 버전을 1부터 시작한다.
};

// 서비스 계층은 동시성 세부 구현 대신 저장소 인터페이스에만 의존한다.
class ConfigService final {
public:
    // 참조 매개변수는 소유하지 않는 lvalue 별명이다. explicit은 의도치 않은 변환을 막는다.
    explicit ConfigService(IConfigRepository& repository) : repository_{repository} {}

    // enable은 기능 이름을 값으로 받아 저장소에 넘기고 새 버전을 반환한다.
    [[nodiscard]] std::uint64_t enable(std::string feature) {
        // feature는 이름 있는 lvalue이고 std::move(feature)는 이동 가능한 xvalue다.
        return repository_.put(std::move(feature), std::string{"on"});
    }

    // bool 반환형은 기능 활성 여부이며 const 참조 매개변수는 호출자 문자열을 빌린다.
    [[nodiscard]] bool enabled(const std::string& feature) const {
        const auto entry{repository_.find(feature)}; // optional 반환값으로 지역 결과를 직접 초기화한다.
        // &&는 왼쪽이 거짓이면 오른쪽을 호출하지 않는 단락 평가 조건 연산자다.
        // has_value()는 인자 없이 값 존재 여부 bool을 반환하며 entry를 바꾸지 않는다.
        return entry.has_value() && entry->value == "on";
    }

    // get은 없을 수도 있는 독립 값 스냅샷을 반환하고 서비스 상태는 바꾸지 않는다.
    [[nodiscard]] std::optional<ConfigEntry> get(const std::string& feature) const {
        return repository_.find(feature); // 값 반환은 결과 객체에 직접 생성되는 복사 생략 대상이다.
    }

private:
    IConfigRepository& repository_; // 참조 멤버는 소유하지 않으므로 저장소가 서비스보다 오래 살아야 한다.
};

// int 반환형 main은 프로그램 진입점이며 0은 성공, 그 밖의 값은 실패 종료를 뜻한다.
int main() {
    // ConfigMap{...}은 prvalue이고 직접 초기화된 저장소가 문자열과 항목을 소유한다.
    ThreadSafeConfigRepository repository{ConfigMap{{"checkout", ConfigEntry{"off", 1}}}};
    ConfigService service{repository}; // repository lvalue에 비소유 참조를 바인딩한다.
    const std::uint64_t version{service.enable(std::string{"checkout"})}; // prvalue 문자열의 소유권이 연쇄 이동된다.
    const auto entry{service.get(std::string{"checkout"})}; // const 지역 optional이 안전한 값 스냅샷을 소유한다.

    if (!entry.has_value()) { // ! 연산자로 값 없음 조건을 검사하고 조기 반환한다.
        // 반환값 1은 기대한 항목이 없어서 검증에 실패했음을 나타낸다.
        return 1;
    }
    // << 연산자가 문자열과 기본 정수를 출력 스트림에 차례로 삽입한다.
    std::cout << "checkout=" << entry->value << " version=" << entry->version << '\n';
    // 가상 호출은 간접 호출일 수 있고 잠금은 대기할 수 있으나 구체 동작은 CPU·ABI·구현·최적화에 따라 달라진다.
    return service.enabled(std::string{"checkout"}) && version == 2 ? 0 : 2;
}
