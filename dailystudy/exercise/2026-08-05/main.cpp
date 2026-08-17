// <array>는 크기가 컴파일 시점에 고정된 std::array를 제공한다.
#include <array>
// <cstddef>는 바이트를 표현하는 std::byte를 제공한다.
#include <cstddef>
// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <memory_resource>는 다형적 메모리 자원과 pmr 컨테이너를 제공한다.
#include <memory_resource>
// <numeric>은 구간을 합산하는 std::accumulate를 제공한다.
#include <numeric>
// <vector>는 연속 메모리 동적 배열의 선언을 제공한다.
#include <vector>

// struct는 기본 접근이 public이므로 단순 값 묶음에 알맞다.
struct Sample {
    int value{}; // int 기본 타입 멤버를 중괄호로 0 초기화한다.
};

// class는 기본 접근이 private이며 외부에는 필요한 연산만 공개한다.
class SampleService {
public:
    // 생성자는 반환형이 없다. explicit은 memory_resource*가 서비스로 암시 변환되는 것을 막는다.
    explicit SampleService(std::pmr::memory_resource* resource)
        : values_{resource} {} // 멤버 초기화 목록이 pmr 벡터에 비소유 자원 포인터를 전달한다.

    // const 참조는 Sample을 복사하지 않고 읽으며, 함수 끝까지 대상 수명이 유지되어야 한다.
    void add(const Sample& sample) {
        // pmr::vector::push_back(const int&)은 sample.value 하나를 입력받고 void를 반환한다.
        // 성공하면 size가 1 늘고 할당은 연결된 memory_resource에서 받는다. 재할당 시 기존 관찰자가 무효화된다.
        values_.push_back(sample.value);
    }

    [[nodiscard]] int total() const {
        // begin()/end()는 첫 반복자와 끝 반복자를 반환한다. accumulate(first,last,init)는 세 인자를 선형 순회한다.
        // 반환형은 초기값 0의 int 타입이고 values_는 바뀌지 않는다. 합이 int 범위를 넘지 않아야 한다.
        return std::accumulate(values_.begin(), values_.end(), 0);
    }

private:
    // 템플릿 인자 int는 원소 형식이며, 컨테이너는 자원을 소유하지 않는다.
    std::pmr::vector<int> values_;
};

int main() { // 진입 함수는 성공 여부를 int 종료 코드로 반환한다.
    std::array<std::byte, 1024> storage{}; // 스택 수명의 1 KiB 원시 저장소를 0 초기화한다.
    // array::data()는 첫 byte 포인터를, size()는 1024라는 size_type을 반환하며 storage를 바꾸지 않는다.
    // monotonic_buffer_resource(buffer,size)는 외부 버퍼 주소와 바이트 수를 입력받고 생성자는 반환값이 없다.
    // 공간이 모자라면 기본 upstream 자원에 추가 할당하며 개별 deallocate는 자원 소멸 전까지 회수하지 않는다.
    std::pmr::monotonic_buffer_resource resource{storage.data(), storage.size()};
    SampleService service{&resource}; // &는 lvalue resource의 주소를 얻고 직접 초기화한다.
    service.add(Sample{10}); // Sample{10}은 prvalue이며 const 참조에 호출 동안 바인딩된다.
    service.add(Sample{32});
    const int answer{service.total()}; // 반환 int prvalue로 const 변수를 직접 초기화한다.
    std::cout << answer << '\n'; // << 연산자는 값을 출력 스트림에 삽입한다.
    // 실제 실행은 로드·저장·비교·분기·호출을 포함할 수 있으나 CPU·ABI·컴파일러·옵션에 따라 달라진다.
    return answer == 42 ? 0 : 1; // == 비교와 ?: 조건 연산자로 테스트 종료 코드를 고른다.
}
