#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <algorithm> // C++20 std::ranges 를 사용하기 위함
#include <stdexcept>

// [실무 테크닉] 단순 int가 아닌 의미를 부여하기 위해 구조체를 사용하는 것이 일반적입니다.
// 여기서는 데이터 패킷의 우선순위를 처리하는 상황을 가정합니다.
struct Packet {
    int payload;
};

class OptimizedBulkScheduler {
private:
    std::vector<Packet> buffer;

    // 1. [기본 문법 설명] static constexpr auto
    // - static: 클래스의 인스턴스(객체) 없이도 존재하게 하여 메모리와 생성 비용을 줄입니다.
    // - constexpr: C++11 도입. 이 함수가 '컴파일 타임'에 미리 계산되고 결정될 수 있음을 지시합니다. 
    //   실행 시간(Run-time) 오버헤드를 0으로 만들어버리는 궁극의 최적화 키워드입니다.
    // - auto: C++11 도입. 컴파일러가 람다의 복잡한 타입을 알아서 추론하게 만듭니다.
    static constexpr auto abs_cmp = [](const Packet& left, const Packet& right) constexpr noexcept -> bool {
        // 2. [기본 문법 설명] const Packet&
        // - const: 이 변수(객체)의 상태를 절대 변경(Write)하지 않겠다는 읽기 전용(Read-Only) 약속입니다.
        // - & (참조): 객체를 통째로 복사(Deep Copy)하는 비용을 없애기 위해 객체의 주소 개념으로 접근합니다.
        
        // 3. [기본 문법 설명] noexcept
        // - C++11 도입. 이 함수 내에서는 절대 예외(Exception)가 던져지지 않음을 컴파일러에게 맹세합니다.
        //   이 키워드가 있으면 컴파일러는 예외 처리를 대비한 잉여 코드를 생성하지 않아 속도가 비약적으로 빨라집니다.
        
        const int left_abs = std::abs(left.payload);
        const int right_abs = std::abs(right.payload);

        // 절댓값이 다르면 절댓값이 큰 순서대로, 같으면 실제 값이 작은 순서대로 배치 (Max-Heap 기준)
        if (left_abs != right_abs) {
            return left_abs < right_abs; 
        }
        return left.payload > right.payload; 
    };

public:
    // 4. [기본 문법 설명] explicit
    // - 단일 인자를 받는 생성자가 의도치 않게 암시적 형변환(Implicit Conversion)을 일으키는 것을 막습니다.
    //   예: OptimizedBulkScheduler scheduler = 100; (이런 말도 안 되는 코드를 컴파일 에러로 잡아냅니다.)
    explicit OptimizedBulkScheduler(std::vector<Packet> initial_data) noexcept {
        // 5. [메모리 최적화 테크닉] std::move
        // - C++11 도입. initial_data가 복사되어 넘어온 임시 객체이므로, 
        //   내부 buffer로 요소들을 하나씩 복사(Copy)하는 대신, 메모리 소유권만 쓱싹 이전(Move)합니다. 비용은 O(1)입니다.
        buffer = std::move(initial_data);

        // 6. [모던 C++ 패러다임] C++20 Ranges 기반 벌크 힙 구축
        // - 요소를 하나씩 push 하는 O(N log N) 방식이 아닌, 한 번에 힙 트리를 구성하는 O(N) 최적화 알고리즘입니다.
        std::ranges::make_heap(buffer, abs_cmp);
    }

    // 7. [기본 문법 설명] [[nodiscard]]
    // - C++17 도입. 이 함수의 반환값을 호출자가 무시(사용하지 않음)하면 컴파일러가 경고를 뱉습니다. 버그 방지용입니다.
    [[nodiscard]] bool empty() const noexcept {
        return buffer.empty();
    }

    // 단일 데이터 스트리밍 삽입
    void push(int val) {
        // 새로운 데이터를 벡터 맨 뒤에 넣고, 힙 트리의 위로 올려보내는(Sift-up) 과정입니다.
        buffer.push_back(Packet{val});
        std::ranges::push_heap(buffer, abs_cmp);
    }

    // 최상위 우선순위 데이터 추출
    int pop_highest() {
        if (empty()) {
            throw std::out_of_range("Scheduler is empty!");
        }

        // 최상단 노드(루트)를 벡터의 맨 마지막으로 보내고 힙 속성을 복구합니다.
        std::ranges::pop_heap(buffer, abs_cmp);
        
        // 맨 뒤로 밀려난 최상단 데이터를 꺼냅니다.
        const int highest_val = buffer.back().payload;
        buffer.pop_back(); // 실제 메모리 반환
        
        return highest_val;
    }
};

int main() {
    // 실무 시나리오: 외부에서 10만 개의 데이터가 한 번에 들어온다고 가정합니다. (여기선 5개로 축약)
    // 중괄호 초기화(Brace Initialization)를 사용하여 직관적으로 벡터를 생성합니다.
    std::vector<Packet> bulk_data = { {10}, {-50}, {30}, {-10}, {50} };

    // 벡터 전체를 move로 넘겨서 생성 즉시 O(N)으로 힙 구축 완료 (Reallocation 0회, Copy 0회)
    OptimizedBulkScheduler scheduler(std::move(bulk_data));

    // 나중에 들어오는 개별 패킷 스트리밍 처리
    scheduler.push(-100);
    scheduler.push(20);

    std::cout << "추출 결과 (절댓값 큰 순, 동률 시 원본 작은 순):" << std::endl;
    while (!scheduler.empty()) {
        std::cout << scheduler.pop_highest() << " ";
        // 예상 출력: -100 -50 50 30 20 -10 10
    }
    std::cout << std::endl;

    return 0;
}