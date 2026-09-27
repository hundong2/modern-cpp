/*
문제 ID·제목: CSES 1077 - Sliding Window Cost
출처: https://cses.fi/problemset/task/1077/

문제 요약:
정수 배열에서 길이가 k인 연속 구간을 왼쪽부터 한 칸씩 옮긴다. 각 구간마다 모든 원소를 하나의 같은
정수로 바꾸기 위해 필요한 변화량의 합을 최소화하고, 그 최소 비용을 차례대로 출력한다. 한 원소 x를
목표값 m으로 바꾸는 비용은 |x-m|이다. 절댓값 합은 중앙값에서 최소가 된다.

입력:
첫 줄에 배열 길이 n과 창 길이 k가 주어진다. 둘째 줄에 n개의 정수 x_1, ..., x_n이 주어진다.

출력:
n-k+1개 창의 최소 비용을 왼쪽에서 오른쪽 순서로 한 줄에 출력한다.

제약:
1 <= k <= n <= 200000, 1 <= x_i <= 10^9.
한 창의 값 합과 비용은 약 2*10^14까지 커질 수 있으므로 값·부분합·답 계산은 64비트 long long을 쓴다.
위치 인덱스는 0..n-1이므로 int로 안전하게 표현된다.

예제:
입력:
8 3
2 4 3 5 8 1 2 1
출력:
2 2 5 7 7 1

핵심:
현재 창을 작은 절반 lower와 큰 절반 upper로 나눈다. lower가 원소를 하나 더 갖도록 하면
max(lower)가 아래쪽 중앙값이다. 각 절반의 64비트 합도 함께 유지하면 절댓값 합을 O(1)에 계산한다.
삽입·정확한 원소 삭제·재균형은 균형 트리에서 O(log k)이므로 전체 시간은 O(n log k)다.
*/

// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout과 형식 스트림 연산을 선언한다.
#include <iostream>
// <set>은 정렬된 노드 기반 연관 컨테이너 std::multiset을 선언한다.
#include <set>
// <vector>는 입력 배열을 연속 저장하는 동적 배열 std::vector를 선언한다.
#include <vector>

// 같은 값이 여러 번 나와도 원래 위치 index가 다르면 서로 다른 창 원소다.
// 중괄호 초기화 Entry{value, index}는 두 기본 타입 멤버를 선언 순서대로 직접 초기화한다.
struct Entry {
    long long value{};  // long long은 공식 최댓값과 합 계산을 안전하게 담는 signed 64비트 이상 정수형이다.
    int index{};        // int는 최대 199999인 0-based 위치를 충분히 표현한다.
};

// 반환형 bool은 left가 엄격히 앞서는지를 뜻한다. 두 매개변수는 const 참조라 복사·수정하지 않는다.
// value를 먼저, 같은 값이면 고유 index를 비교해 엄격 약순서를 만든다. noexcept는 기본 정수 비교가
// 예외를 던지지 않으며 이 함수도 예외를 내보내지 않겠다는 계약이다.
[[nodiscard]] bool operator<(const Entry& left, const Entry& right) noexcept {
    if (left.value != right.value) {
        return left.value < right.value;
    }
    return left.index < right.index;
}

// 공용 알고리즘 설명: ../algorithm/sliding-window-median-cost.md
// class는 struct와 달리 접근 지정자를 쓰지 않으면 멤버가 private이다. public은 호출자에게 제공할 연산,
// private은 두 파티션과 불변식을 복구하는 구현 세부 사항이다.
class SlidingWindowCost {
public:
    // [첫 생성 계약: 두 std::multiset<Entry> 기본 생성자]
    // (1) 수신 lower_와 upper_는 아직 수명이 시작되지 않은 정확한 타입
    //     std::multiset<Entry, std::less<Entry>, std::allocator<Entry>> 하위 객체다.
    // (2) 각 `multiset() : multiset(Compare())`가 선택되고 숨은 기본 인자로
    //     std::less<Entry>::less() noexcept와 std::allocator<Entry>::allocator() noexcept가 선택된다.
    // (3) 명시 인자는 없다. 두 숨은 생성자도 무인자이며 무상태 비교자/비소유 allocator prvalue를 만들 뿐,
    //     외부 원소나 저장소의 소유권을 받지 않는다.
    // (4) 생성자는 반환값이 없고 성공하면 서로 독립적인 빈 정렬 컨테이너 두 개를 만든다.
    // (5) 성공 뒤 두 컨테이너의 size는 0이고 합 멤버도 중괄호로 0 초기화된다.
    // (6) less/allocator 임시의 생성·전체 식 끝 소멸은 O(1)·무할당·비투척이다. 빈 multiset 구성도 원소 수
    //     기준 O(1)이지만 multiset() 자체는 noexcept가 아니어서 구현 내부 설정/할당 실패 예외를 보존한다.
    //     실패 시 완성 객체가 없고, 객체는 자체 동기화를 제공하지 않으며 원소 수명은 컨테이너에 묶인다.
    SlidingWindowCost()
        : lower_{}, upper_{}, lower_sum_{}, upper_sum_{} {}

    // 매개변수 entry는 값으로 받아 호출자의 원본과 독립인 작은 복사본이다. 반환형 void는 결과 대신
    // 객체 상태를 바꾼다는 뜻이다. 성공 뒤 entry는 정확히 한 파티션에 있고 네 불변식이 모두 복구된다.
    void add(const Entry entry) {
        // [첫 호출 계약: std::multiset<Entry>::size]
        // (1) 수신 lower_와 upper_는 이 객체가 소유하며 각자 유효한 정렬 multiset이다.
        // (2) 두 호출 모두 size_type size() const noexcept 관찰자 overload를 선택한다.
        // (3) 명시 인자와 소유권 이전은 없다. 수신 객체는 읽기만 한다.
        // (4) 각 호출은 현재 원소 수를 부호 없는 size_type 값으로 반환하며 조건/산술에 사용한다.
        // (5) 두 컨테이너, 합, entry는 변하지 않는다.
        // (6) 각 호출은 O(1)·무할당·noexcept이고 iterator/참조를 무효화하지 않는다. 객체 수명이 유효해야
        //     하며 같은 컨테이너를 다른 스레드가 동시에 수정하면 데이터 경쟁이다.
        if (lower_.size() == 0U) {
            // [첫 호출 계약: std::multiset<Entry>::insert(const value_type&)]
            // (1) 수신 lower_는 유효하며 현재 비어 있고, 이후 같은 형태의 upper_ 호출도 유효한 상태다.
            // (2) iterator insert(const value_type&) overload가 value_type=Entry로 선택된다.
            // (3) entry는 const Entry lvalue라 읽어 복사하며 소유권을 넘기지 않는다. 비교자는 유효한 모든
            //     Entry를 허용하고 고유 index 때문에 현재 창 안에는 동등 키가 없다.
            // (4) 새 노드를 가리키는 iterator를 반환하지만 여기서는 필요 없어 버린다.
            // (5) 성공 뒤 수신 size가 1 늘고 정렬 위치에 entry 복사본이 생기며 entry와 다른 컨테이너는
            //     그대로다. 이어 합 변수에 value를 더한다.
            // (6) O(log k) 비교와 노드 할당이 가능하다. bad_alloc 또는 비교/복사 예외 시 삽입 효과가 없고
            //     예외가 전파된다. 성공해도 기존 iterator/참조는 무효화되지 않으며 새 노드는 erase 전까지
            //     컨테이너가 소유한다. 동일 컨테이너 동시 수정은 동기화되지 않는다.
            lower_.insert(entry);
            lower_sum_ += entry.value;
        } else {
            // [첫 호출 계약: multiset::end와 양방향 iterator 감소·역참조]
            // (1) 수신 lower_는 size()>0인 std::multiset<Entry>이고 모든 iterator는 아직 유효하다.
            // (2) `end() noexcept -> std::multiset<Entry>::iterator` 뒤 Cpp17BidirectionalIterator의 전위
            //     감소 식 `--pivot`과 constant iterator 역참조 식 `*pivot`을 차례로 평가한다. 구현의 물리적
            //     연산자 선언과 무관하게 식 결과는 각각 iterator&, const Entry&다.
            // (3) end에는 인자가 없다. --의 수신 pivot은 lower_의 past-the-end iterator이며 비어 있지 않아
            //     감소 가능하다. *의 수신은 감소 후 마지막 원소를 가리키는 iterator다. 소유권 이전은 없다.
            // (4) end는 끝 iterator를 반환해 pivot을 초기화하고, --는 갱신된 iterator 참조를 반환해 버리며,
            //     *는 최대 Entry의 참조를 반환해 비교에 사용한다.
            // (5) pivot만 마지막 원소 위치로 바뀌고 컨테이너·원소·합·entry는 바뀌지 않는다.
            // (6) 각 연산은 양방향 iterator에서 O(1)·무할당이다. end는 noexcept다. 빈 컨테이너의 end를
            //     감소하거나 end/무효 iterator를 역참조하면 미정의 동작이다. 참조 수명은 해당 원소가
            //     지워지기 전까지이며, 동시 수정과의 경쟁을 허용하지 않는다.
            auto pivot{lower_.end()};
            --pivot;

            // pivot이 가리키는 Entry는 컨테이너가 소유하는 lvalue다. 사용자 정의 operator<는 두 값을
            // 읽기만 한다. !(pivot < entry)는 entry가 경계보다 작거나 같은 사전식 키라는 뜻이다.
            if (!(*pivot < entry)) {
                // 위 insert 계약을 재사용한다. entry는 const lvalue이므로 같은 const& overload가 선택된다.
                lower_.insert(entry);
                lower_sum_ += entry.value;
            } else {
                // 위 insert 계약을 upper_에 동일하게 적용한다. 기존 iterator는 유지된다.
                upper_.insert(entry);
                upper_sum_ += entry.value;
            }
        }

        rebalance();
    }

    // 전제조건: entry는 현재 창에 정확히 한 번 존재한다. 고유 index 때문에 값이 중복되어도 키는 유일하다.
    // 성공 뒤 그 원소 하나만 제거되고 분할·순서·크기·합 불변식이 다시 성립한다.
    void remove(const Entry entry) {
        // [첫 호출 계약: multiset::find, end와 iterator 동등성 비교]
        // (1) 수신 lower_는 정렬 불변식을 만족하는 유효한 std::multiset<Entry>이고 entry는 유효한 키다.
        // (2) `iterator find(const key_type&)`와 `end() noexcept -> iterator`를 선택한다. 소스의 !=는 같은
        //     컨테이너 유효 iterator 두 개의 동등 비교 식이며 직접 != 또는 == rewritten candidate일 수 있다.
        // (3) find의 entry는 const Entry lvalue로 빌려 읽으며 소유권 이전이 없다. 비교 피연산자는 같은
        //     lower_에서 나온 유효한 iterator 두 개다.
        // (4) find는 동등 키가 있으면 그 원소 iterator, 없으면 end를 반환해 lower_position을 초기화한다.
        //     end는 비교용 끝 iterator, 동등 비교 식은 bool-convertible 존재 여부를 돌려 분기에 사용한다.
        // (5) 탐색·비교는 컨테이너, entry, 합과 iterator가 가리키는 원소를 바꾸지 않는다.
        // (6) find는 O(log k), end/비교는 O(1)이고 모두 무할당이다. 사용자 비교 예외는 find에서 전파될 수
        //     있으나 이 Entry 비교는 noexcept다. iterator는 수신 객체 수명과 erase까지 유효하며 서로 다른
        //     컨테이너 iterator 비교는 정의되지 않는다. 동시 수정은 데이터 경쟁이다.
        const auto lower_position{lower_.find(entry)};
        if (lower_position != lower_.end()) {
            lower_sum_ -= entry.value;

            // [첫 호출 계약: std::multiset<Entry>::erase(iterator 위치)]
            // (1) 수신 lower_는 유효하고 lower_position은 lower_의 실제 원소 하나를 가리킨다. 이후 upper_의
            //     위치 삭제도 같은 전제조건을 만족한다.
            // (2) iterator가 const_iterator와 같은 구현에서는 iterator erase(const_iterator position),
            //     다르면 대응하는 위치 erase overload가 선택된다.
            // (3) 위치 iterator는 비소유 핸들이며 end가 아니고 반드시 이 수신 컨테이너에서 왔다.
            // (4) 삭제 다음 원소 iterator를 반환하지만 여기서는 사용하지 않고 버린다.
            // (5) 지정한 Entry 한 개가 파괴·제거되고 size가 1 줄며 다른 원소 순서는 유지된다.
            // (6) amortized O(1) 위치 삭제이고 추가 할당은 없다. 지운 원소의 iterator/참조만 무효화되고
            //     나머지는 유지된다. Entry 소멸은 예외를 던지지 않는다. end·외부·이미 무효인 iterator를
            //     넘기면 미정의 동작이며 동일 컨테이너 동시 접근은 동기화되지 않는다.
            lower_.erase(lower_position);
        } else {
            // 앞선 find 계약을 upper_에 적용한다. remove의 전제조건과 분할 완전성 때문에 반환값은 end가
            // 아닌 유효한 위치다. 이를 어기고 end를 erase에 넘기면 미정의 동작이므로 호출자가 보장한다.
            const auto upper_position{upper_.find(entry)};
            upper_sum_ -= entry.value;
            upper_.erase(upper_position);
        }

        rebalance();
    }

    // 반환형 long long은 현재 창의 최소 절댓값 합이다. 전제조건은 창이 비어 있지 않다는 것이다.
    [[nodiscard]] long long cost() const {
        // 앞서 설명한 end/--/* 계약을 재사용한다. 크기 불변식상 전체가 비어 있지 않으면 lower_도
        // 비어 있지 않다. median은 Entry 안의 기본 정수 값을 복사하므로 컨테이너 참조 수명과 독립이다.
        auto median_position{lower_.end()};
        --median_position;
        const long long median{(*median_position).value};

        // 앞서 설명한 size() 계약을 재사용한다. 공식 제약상 두 size는 최대 200000이라 long long 변환이
        // 정확하다. median이 long long이므로 아래 곱셈도 64비트에서 이루어지고 최댓값 약 2*10^14는
        // 표현 범위 안이다.
        const long long lower_count{static_cast<long long>(lower_.size())};
        const long long upper_count{static_cast<long long>(upper_.size())};

        // lower의 모든 값은 median 이하, upper의 모든 값은 median 이상이다. 따라서 절댓값을 두 방향의
        // 선형식으로 풀 수 있다. 함수는 컨테이너와 합을 바꾸지 않고 계산한 prvalue를 값으로 반환한다.
        return median * lower_count - lower_sum_ + upper_sum_ - median * upper_count;
    }

private:
    // using은 긴 표준 타입에 짧은 별칭을 붙일 뿐 새 타입을 만들지 않는다. Tree::iterator는 정확히
    // std::multiset<Entry>::iterator와 같은 타입이다.
    using Tree = std::multiset<Entry>;

    void move_largest_lower_to_upper() {
        // end/--/* 계약을 재사용한다. 호출 전 크기 불변식 계산이 lower_가 비어 있지 않음을 보장한다.
        auto source{lower_.end()};
        --source;
        const Entry moving{*source};

        // insert 계약을 재사용한다. 먼저 복사 삽입하므로 노드 할당이 실패하면 원래 lower_와 합은 그대로다.
        upper_.insert(moving);
        upper_sum_ += moving.value;
        lower_sum_ -= moving.value;
        // erase 계약을 재사용하며 source는 아직 lower_의 유효한 실제 원소 위치다.
        lower_.erase(source);
    }

    void move_smallest_upper_to_lower() {
        // [첫 호출 계약: std::multiset<Entry>::begin과 iterator 역참조]
        // (1) 수신 upper_는 재균형 조건상 size()>0인 유효한 정렬 std::multiset<Entry>다.
        // (2) `begin() noexcept -> std::multiset<Entry>::iterator`와 constant iterator 역참조 식 `*source`를
        //     평가한다. 구현의 물리적 연산자 선언과 무관하게 역참조 식 결과는 const Entry&다.
        // (3) begin에는 인자가 없고, *의 수신 source는 begin이 돌려준 첫 원소 iterator다. 소유권 이전은 없다.
        // (4) begin은 최소 Entry 위치를 반환해 source를 초기화하고, *는 const Entry&를 반환해 moving에
        //     값 복사한다. 반환값은 모두 사용된다.
        // (5) source와 moving만 생기며 upper_, 원소, 합은 아직 바뀌지 않는다.
        // (6) 두 연산은 O(1)·무할당이고 begin은 noexcept다. 빈 컨테이너에서는 begin==end이므로 이를
        //     역참조하면 미정의 동작이다. 참조는 원소 erase 전까지만 유효하고 동시 수정은 데이터 경쟁이다.
        auto source{upper_.begin()};
        const Entry moving{*source};

        // 앞선 insert/erase 계약을 각 반대 파티션에 그대로 적용한다.
        lower_.insert(moving);
        lower_sum_ += moving.value;
        upper_sum_ -= moving.value;
        upper_.erase(source);
    }

    void rebalance() {
        // size() 계약을 재사용한다. auto는 두 size_type 합과 같은 부호 없는 타입을 정확히 보존한다.
        const auto total_size{lower_.size() + upper_.size()};
        const auto desired_lower_size{(total_size + 1U) / 2U};

        // 우승 풀이의 핵심 불변식: lower는 항상 ceil(total/2)개를 가진다. 공개 연산 하나가 원소 한 개만
        // 바꾸므로 실제 이동은 상수 번이지만 while로 쓰면 가변 길이 사용에도 안전하다.
        while (lower_.size() > desired_lower_size) {
            move_largest_lower_to_upper();
        }
        while (lower_.size() < desired_lower_size) {
            // 이 조건이면 전체 원소 중 lower에 부족한 것이므로 upper_에는 옮길 원소가 반드시 있다.
            move_smallest_upper_to_lower();
        }
    }

    // 두 Tree는 현재 창 원소의 소유자다. 노드 컨테이너이므로 삽입은 기존 iterator를 무효화하지 않고,
    // erase는 지운 원소의 iterator만 무효화한다.
    Tree lower_;
    Tree upper_;
    long long lower_sum_{};
    long long upper_sum_{};
};

int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio와 std::cin.tie]
    // (1) 첫 호출은 ios_base가 선언하고 basic_ios<char>의 상속 이름으로 부르는 정적 설정 함수다. tie의 수신자는
    //     입력 전 상태인 정확한 타입 std::istream의 전역 객체 std::cin이다.
    // (2) `static bool std::ios_base::sync_with_stdio(bool sync = true)`와
    //     `std::basic_ostream<char, std::char_traits<char>>*
    //     std::basic_ios<char, std::char_traits<char>>::tie(
    //     std::basic_ostream<char, std::char_traits<char>>* tiestr)`가 선택된다.
    // (3) false는 C 스트림과 동기화를 끄는 bool prvalue, nullptr는 자동 flush 대상을 없애는 null
    //     std::ostream* prvalue다. default sync=true는 쓰지 않고 두 값을 모두 명시 전달하며 소유권 이전이 없다.
    // (4) 첫 호출은 이전 동기화 설정 bool, 둘째는 이전 tie 비소유 포인터를 반환하지만 둘 다 버린다.
    // (5) 이후 C/C++ 혼합 I/O 순서 보장과 입력 전 자동 flush 정책이 바뀐다. nullptr는 tie cycle을 만들지
    //     않으므로 tie 전제조건을 만족하고, 두 인자 자체는 변하지 않는다.
    // (6) 두 설정의 복잡도는 표준이 별도 상한을 정하지 않는다. I/O 뒤 sync 호출 효과는 구현에 따라 달라져
    //     첫 I/O 전에 수행한다. 해제 뒤 C I/O와 섞지 않으며 같은 표준 스트림의 동시 접근은 더는 안전을
    //     보장받지 못해 data race와 UB가 될 수 있다. 표준 스트림 수명은 프로그램 종료까지다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int number_count{};
    int window_size{};

    // [첫 호출 계약: std::istream의 int 형식 추출 operator>>]
    // (1) 수신 std::cin은 입력 가능한 std::istream이고 두 int 객체는 0으로 초기화됐다.
    // (2) `std::basic_istream<char, std::char_traits<char>>&
    //     std::basic_istream<char, std::char_traits<char>>::operator>>(int&)`가 왼쪽부터 두 번 선택된다.
    // (3) number_count와 window_size는 수정 가능한 int lvalue 참조로 전달되며 소유권 이전은 없다.
    //     공식 입력은 두 값이 각 타입 범위와 1<=k<=n을 만족한다.
    // (4) 각 호출은 같은 std::istream&를 반환해 다음 추출에 사용하고 마지막 반환 참조는 버린다.
    // (5) 성공하면 두 변수와 스트림 입력 위치가 갱신된다. 형식 실패 시 상태 비트가 설정되고 대상 값은
    //     표준 수치 추출 규칙에 따라 유지되거나 0/경곗값으로 설정될 수 있다.
    // (6) locale·버퍼·소비 문자에 따른 비용이 들며 단순 O(1)을 보장하지 않는다. eofbit/failbit/badbit와
    //     설정된 예외가 가능하다. 위에서 동기화를 껐으므로 같은 스트림의 동시 접근은 data race와 UB가 될 수 있다.
    std::cin >> number_count >> window_size;

    // [첫 생성 계약: std::vector<long long> count 생성자]
    // (1) 수신 values는 아직 생성되지 않았고 number_count는 공식 제약의 양수다.
    // (2) explicit vector(size_type count, const allocator_type& alloc = allocator_type())와 생략 인자용
    //     std::allocator<long long>::allocator() noexcept가 선택된다. 소괄호라 initializer_list가 아니다.
    // (3) number_count int lvalue의 값이 size_type으로 정확히 변환된다. 생략한 allocator 인자는
    //     std::allocator<long long>{} prvalue로 평가되어 const allocator_type&에 바인딩되며 외부 저장소 owner는 없다.
    // (4) 두 생성자는 반환값이 없다. allocator 임시가 vector 생성 인자로 쓰이고 count개 원소가 완성된다.
    // (5) 성공 뒤 size=number_count, 모든 원소는 0이다. allocator 임시는 전체 식 끝에 소멸하지만 values의
    //     저장소 수명은 독립이고 number_count는 변하지 않는다.
    // (6) O(n) 초기화·할당에서 bad_alloc/length_error가 전파될 수 있다. allocator 임시 생성·소멸은
    //     O(1)·무할당·비투척이다. 실패 시 완성 vector가 없고 values는 자체 동기화를 제공하지 않는다.
    std::vector<long long> values(number_count);

    for (int index{}; index < number_count; ++index) {
        // [첫 호출 계약: vector::operator[]와 long long 형식 추출 operator>>]
        // (1) 수신 values는 size=number_count인 mutable std::vector<long long>, std::cin은 배열 입력
        //     위치이며 0<=index<number_count다.
        // (2) reference vector::operator[](size_type)와 `std::basic_istream<char, std::char_traits<char>>&
        //     std::basic_istream<char, std::char_traits<char>>::operator>>(long long&)`가 선택된다.
        // (3) index 값은 size_type으로 정확히 변환되고 소유권 의미가 없다. 반환된 long long lvalue는
        //     추출의 수정 가능한 인자로 빌려 주며 공식 값은 1..10^9다.
        // (4) []는 해당 원소 long long&를 반환해 추출에 사용하고, >>는 std::istream&를 반환해 버린다.
        // (5) 성공하면 해당 원소와 입력 위치만 갱신되고 vector 크기·용량·소유권은 유지된다.
        // (6) []는 O(1)·무할당이고 범위를 검사하지 않아 index가 범위 밖이면 미정의 동작이다. 추출은
        //     상태 비트/설정 예외를 낼 수 있다. 구조 변경이 없어 참조 무효화는 없고 동시 쓰기는 데이터
        //     경쟁이다. 전체 입력은 소비 문자 수와 n에 선형이다.
        std::cin >> values[index];
    }

    // SlidingWindowCost의 사용자 정의 기본 생성자가 내부 두 multiset을 만들고 합을 0으로 초기화한다.
    SlidingWindowCost window{};

    for (int index{}; index < window_size; ++index) {
        // 앞서 설명한 vector::operator[] 계약을 재사용한다. index는 항상 유효하고 값은 복사된다.
        window.add(Entry{values[index], index});
    }

    // [첫 호출 계약: std::ostream의 long long 삽입 operator<<]
    // (1) 수신 std::cout은 출력 가능한 std::ostream이고 window는 비어 있지 않은 완성 창이다.
    // (2) `std::basic_ostream<char, std::char_traits<char>>&
    //     std::basic_ostream<char, std::char_traits<char>>::operator<<(long long)` 멤버가 선택된다.
    // (3) window.cost()는 최소 비용 long long prvalue이며 출력 형식화에 쓰이고 소유권 의미가 없다.
    // (4) std::ostream&를 반환하지만 이 문장에서는 연쇄하지 않아 버린다.
    // (5) 출력 버퍼와 stream 상태가 갱신되고 window와 비용 값은 변하지 않는다.
    // (6) locale·숫자 변환·버퍼 처리 비용이 들며 표준은 단순 O(1)을 보장하지 않는다. 실패는 상태 비트
    //     또는 설정된 예외로 나타나고 외부 참조를 무효화하지 않는다. 위에서 동기화를 껐으므로 같은
    //     std::cout의 동시 접근은 data race와 UB가 될 수 있고, 수명은 프로그램 종료까지다.
    std::cout << window.cost();

    for (int right{window_size}; right < number_count; ++right) {
        const int leaving_index{right - window_size};
        // 삭제 뒤 크기 k-1 불변식을 먼저 복구한 다음 새 원소를 넣는다. 고유 index 덕분에 중복값도
        // 정확히 현재 창에서 나가는 한 노드만 찾는다.
        window.remove(Entry{values[leaving_index], leaving_index});
        window.add(Entry{values[right], right});

        // [첫 호출 계약: std::ostream의 char와 long long 연쇄 삽입 operator<<]
        // (1) 수신 std::cout은 앞선 비용 출력 뒤 유효하고 window는 다음 완성 창을 나타낸다.
        // (2) 첫 호출은 Traits=std::char_traits<char>인 비멤버 function template
        //     `template<class Traits> std::basic_ostream<char, Traits>& operator<<(
        //     std::basic_ostream<char, Traits>&, char)`, 둘째는 위 long long 멤버 overload를 선택한다.
        // (3) ' '은 char prvalue, cost() 결과는 long long prvalue이며 둘 다 읽기 전용·비소유 값이다.
        // (4) 첫 호출이 같은 ostream&를 반환해 둘째 수신자로 사용하고 최종 ostream&는 버린다.
        // (5) 공백과 비용이 순서대로 버퍼에 추가되고 stream 상태만 바뀌며 window는 유지된다.
        // (6) 각 형식 출력의 복잡도는 locale·숫자 변환·버퍼 구현에 따르며 단순 O(1)을 보장하지 않는다.
        //     상태 비트/설정 예외가 가능하고 참조·iterator 무효화는 없다. sync=false인 같은 std::cout의
        //     동시 접근은 data race와 UB가 될 수 있다. 공백을 먼저 써 마지막 뒤 여분 공백은 없다.
        std::cout << ' ' << window.cost();
    }

    // 위 char 삽입 계약을 재사용한다. '\n'은 줄을 끝내지만 endl 조작자처럼 명시적 flush를 강제하지 않는다.
    std::cout << '\n';

    // [첫 숨은 소멸 계약: multiset iterator, std::vector와 두 std::multiset]
    // (1) 각 블록 끝의 multiset iterator는 유효하거나 더는 사용하지 않을 비소유 값이고, main 끝의 values는
    //     유효한 std::vector<long long>, window의 lower_/upper_는 유효한 std::multiset<Entry>다.
    // (2) 구현 iterator 소멸자, `~vector()`와 `~multiset()`이 선택되며 모두 인자와 반환값이 없다.
    // (3) 소멸에 데이터 인자나 새 소유권 입력은 없다. iterator는 노드를 소유하지 않는다.
    // (4) 소멸자는 값을 반환하지 않고 호출부가 사용하는 결과도 없다.
    // (5) iterator 소멸은 컨테이너에 영향이 없다. vector는 long long 원소와 연속 저장소를, 각 multiset은
    //     Entry 원소와 노드를 파괴·반환하며 관련 iterator·참조·포인터의 수명을 모두 끝낸다.
    // (6) iterator 소멸은 O(1)·무할당, 컨테이너 소멸은 원소 수에 선형이고 저장소/노드를 해제한다. 이 기본
    //     정수·Entry·allocator 경로는 비투척이며, 소멸과 같은 객체의 동시 접근은 동기화 없이는 안전하지 않다.
}
