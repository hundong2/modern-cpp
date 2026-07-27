// <algorithm>은 조건에 맞는 원소를 지우는 C++20 std::erase_if를 제공한다.
#include <algorithm>
// <iostream>은 결과 출력용 std::cout과 << 연산자를 제공한다.
#include <iostream>
// <memory>는 공유 소유권 shared_ptr와 비소유 관찰자 weak_ptr를 제공한다.
#include <memory>
// <string>은 문자 시퀀스를 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 이동 요청에 쓰는 std::move를 제공한다.
#include <utility>
// <vector>는 여러 콜백 객체를 저장할 컨테이너를 제공한다.
#include <vector>

// struct는 기본 public이다. 알림을 받는 아키텍처 포트를 순수 가상 함수로 정의한다.
struct IObserver {
public:
    virtual ~IObserver() = default; // 기반 포인터를 통한 안전한 다형 파괴를 보장한다.
    virtual void update(int value) const = 0; // void 반환형, int 값 매개변수, 읽기 전용 const 멤버 함수다.
};

// class는 기본 private다. public/private 지정자로 외부 계약과 내부 상태를 나눈다.
class NamedObserver final : public IObserver {
public:
    // 생성자는 반환형이 없고 name은 매개변수다. explicit은 암시적 문자열 변환을 막는다.
    explicit NamedObserver(std::string name)
        : name_{std::move(name)} { // 멤버 초기화 목록에서 xvalue를 이용해 문자열 소유권 이동을 요청한다.
    }

    void update(int value) const override {
        std::cout << name_ << '=' << value << '\n'; // << 연산자와 표준 출력 함수를 실제 호출한다.
    }

private:
    std::string name_{}; // 구독자 이름을 소유하는 멤버 변수다.
};

// <IObserver>는 shared_ptr 템플릿 인자이고 using은 해당 긴 타입의 별칭을 만든다.
using ObserverPtr = std::shared_ptr<IObserver>;

class Subject {
public:
    void attach(const ObserverPtr& observer) {
        // const 참조는 lvalue shared_ptr를 복사하지 않고 읽으며 weak_ptr 저장은 강한 소유권을 늘리지 않는다.
        observers_.push_back(observer);
    }

    [[nodiscard]] int notify(int value) {
        int count{0}; // 기본 타입 int 변수의 안전한 중괄호 초기화다.

        // TODO 연습: 각 weak를 lock하고, 성공한 경우 update(value)를 호출한 뒤 count를 증가시켜 보자.
        for (const std::weak_ptr<IObserver>& weak : observers_) {
            // lock() 결과 prvalue를 직접 초기화하고, bool 변환으로 객체가 살아 있는지 조건 분기한다.
            if (ObserverPtr observer{weak.lock()}) {
                observer->update(value); // -> 연산자를 통한 가상 간접 호출이다.
                ++count; // 전위 증가 연산자가 호출 횟수를 갱신한다.
            }
        }

        // 만료된 관찰자를 지우면 다음 반복의 불필요한 비교와 분기를 줄일 수 있다.
        std::erase_if(observers_, [](const std::weak_ptr<IObserver>& weak) {
            return weak.expired();
        });
        return count;
    }

private:
    std::vector<std::weak_ptr<IObserver>> observers_{}; // Subject는 Observer 객체를 소유하지 않는다.
};

int main() {
    Subject subject{}; // 이름 있는 subject는 lvalue이고 객체 수명은 main 블록 끝까지다.
    // make_shared의 prvalue가 shared_ptr 목적 객체를 직접 만들며 NamedObserver{"sensor"}가 올바른 직접 초기화다.
    ObserverPtr observer{std::make_shared<NamedObserver>("sensor")};
    subject.attach(observer); // const lvalue 참조 매개변수에 기존 shared_ptr가 바인딩된다.

    const int before{subject.notify(7)};
    observer.reset(); // 마지막 강한 소유권을 해제하며 weak_ptr만으로는 객체 수명을 연장하지 못한다.
    const int after{subject.notify(9)};

    // 비교 결과 bool에 따라 성공/실패 반환 경로로 조건 분기한다.
    if (before == 1 && after == 0) {
        return 0;
    }
    return 1;
}
