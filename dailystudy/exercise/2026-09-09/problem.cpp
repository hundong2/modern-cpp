// <cstddef>는 페이지 시작과 원소 수를 표현하는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 결과를 쓰는 std::cout/std::ostream과 삽입 연산자를 선언한다.
#include <iostream>
// <ranges>는 반복자 쌍을 읽기 전용 비소유 범위로 노출할 std::ranges::subrange를 선언한다.
#include <ranges>
// <utility>는 소유 vector를 멤버로 넘길 때 쓰는 std::move를 선언한다.
#include <utility>
// <vector>는 점수 Entry를 연속 저장소에 소유하는 std::vector를 선언한다.
#include <vector>

// struct는 기본 public이다. 별도 불변식 없이 값만 전달하는 DTO라 필드를 직접 읽는다.
struct Entry {
    int points{}; // int{}는 빠진 초기값을 0으로 만드는 중괄호 값 초기화다.
};

struct SliceResult {
    std::size_t count{};
    int total{};
};

// class는 기본 private이다. vector 소유권과 clamp 규칙을 감추고 const_iterator 범위만 공개한다.
class ScoreBoard {
public:
    // using은 새 타입이 아니라 별칭이다. vector의 템플릿 인자 Entry가 소유 원소 타입이고,
    // subrange의 반복자/센티널 템플릿 인자는 같은 ConstIterator라 크기를 O(1)에 계산할 수 있다.
    using Storage = std::vector<Entry>;
    using ConstIterator = Storage::const_iterator;
    using Difference = Storage::difference_type;
    using Slice = std::ranges::subrange<ConstIterator>;

    // explicit은 Storage에서 ScoreBoard로의 의도치 않은 암시 변환을 막고, 멤버 초기화 목록은
    // body 전에 owned_를 직접 구성한다.
    // [호출 계약: std::move(entries)와 Storage 이동 생성]
    // (1) 수신 객체 없는 move의 대상 entries는 완성된 Storage 값 매개변수 lvalue이고 owned_는 구성 전이다.
    // (2) move<T>(T&&)에서 T=Storage&가 추론되어 Storage&&를 반환하고 vector(vector&&)가 선택된다.
    // (3) entries는 모든 유효한 vector를 허용하는 lvalue 식이다. move는 빌릴 뿐이고 후속 생성자가
    //     연속 저장소 소유권을 이전한다.
    // (4) 반환 xvalue 참조는 owned_ 구성에 즉시 사용하며 생성자 자체 반환값은 없다.
    // (5) owned_가 Entry를 소유하고 entries는 살아 있지만 값 미지정이다.
    // (6) 기본 allocator에서는 O(1), 무할당·noexcept이고 기존 원소 관찰자는 새 소유자의 원소를 가리킨다.
    //     수명을 연장하거나 스레드를 동기화하지 않으며 유효한 vector를 한 흐름에서 옮겨 UB를 피한다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    // 대표 문서: ../standard-library/containers-and-views.md
    explicit ScoreBoard(Storage entries) noexcept
        : owned_{std::move(entries)} {}

    [[nodiscard]] Slice slice(std::size_t offset, std::size_t requested_count) const {
        // [호출 계약: const Storage::size()]
        // (1) 수신자는 Entry들을 소유하고 수명이 유효한 const Storage lvalue owned_다.
        // (2) size_type size() const noexcept overload가 선택되고 데이터 인자는 없다.
        // (3) 숨은 const this만 빌리며 원소나 소유권을 수정하지 않는다.
        // (4) 반환된 std::size_t와 같은 기본 allocator size_type prvalue를 current_size 구성에 사용한다.
        // (5) owned_의 크기·용량·원소·반복자는 바뀌지 않는다.
        // (6) O(1), 무할당·비무효화·noexcept다. owned_가 살아 있어야 하고 동시 쓰기는 외부에서 막아야 한다.
        // 대표 문서: ../standard-library/containers-and-views.md
        const std::size_t current_size{owned_.size()};
        const std::size_t bounded_offset{offset < current_size ? offset : current_size};
        const std::size_t remaining{current_size - bounded_offset};
        const std::size_t bounded_count{
            requested_count < remaining ? requested_count : remaining};

        // [호출 계약: Storage::cbegin()과 ConstIterator 덧셈]
        // (1) owned_는 구조 변경 없는 const Storage lvalue이고 두 결과 위치는 같은 저장소에 속한다.
        // (2) cbegin() const noexcept와 random-access ConstIterator::operator+(Difference)가 선택된다.
        // (3) cbegin의 데이터 인자는 없다. +에는 각각 [0,size] 안의 non-negative Difference prvalue를 주며
        //     iterator는 저장소를 빌릴 뿐 소유하지 않는다.
        // (4) 첫 + 반환은 first를, 두 번째 + 반환은 last를 구성하는 데 사용한다.
        // (5) owned_는 그대로이고 [first,last)는 정확히 bounded_count개 원소를 나타낸다.
        // (6) 각 연산 O(1), 무할당·비무효화다. clamp가 같은 배열의 허용 위치만 더한다. vector 재할당·
        //     관련 삭제·파괴 후 반복자는 무효이고 동시 변경은 데이터 경쟁이다.
        // 대표 문서: ../standard-library/containers-and-views.md
        const ConstIterator first{
            owned_.cbegin() + static_cast<Difference>(bounded_offset)};
        const ConstIterator last{first + static_cast<Difference>(bounded_count)};

        // [생성 계약: std::ranges::subrange<ConstIterator>(first, last)]
        // (1) 결과 Slice는 아직 없고 first/last는 같은 살아 있는 owned_의 유효한 순서 경계다.
        // (2) I=S=ConstIterator인 sized subrange의 subrange(I,S) 생성자가 선택된다.
        // (3) 두 const iterator lvalue를 값 복사해 보관하고 Entry/vector 소유권은 받지 않는다.
        // (4) 생성자 반환값은 없고 Slice prvalue가 slice() 결과 객체에 직접 구성되어 사용된다.
        // (5) owned_는 그대로이며 결과는 clamp된 반열린 범위다.
        // (6) O(1), 무할당·비무효화다. 생성 시 [first,last)가 유효해야 하며 last는 first에서 도달 가능한
        //     같은 저장소 경계여야 한다. 위반은 생성자 전제조건 위반이며 미정의 동작이다. 여기서는 clamp와
        //     같은 vector가 이를 보장한다. 원본보다 오래 보관하면 댕글링이며 예외·동기화를 추가하지 않는다.
        //     같은 타입 prvalue 반환은 보장 복사 생략이며 이름 있는 지역의 선택적 NRVO와 다르다.
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        return Slice{first, last};
    }

private:
    Storage owned_;
};

[[nodiscard]] SliceResult evaluate(const ScoreBoard::Slice& slice) {
    // [호출 계약: const sized subrange::size()]
    // (1) 수신자는 살아 있는 ScoreBoard 저장소를 빌리는 const Slice lvalue다.
    // (2) sized subrange의 size() const overload가 선택되고 데이터 인자는 없다.
    // (3) iterator 경계가 유효해야 하며 값·소유권 입력은 없다.
    // (4) unsigned 거리 prvalue를 std::size_t count에 저장해 반환 DTO에 사용한다.
    // (5) slice와 원본 Entry는 변하지 않는다.
    // (6) O(1), 무할당·비무효화다. 댕글링 iterator의 차이는 미정의이고 외부 동시 쓰기는 금지한다.
    // 대표 문서: ../standard-library/algorithms-and-ranges.md
    const std::size_t count{slice.size()};
    int total{};

    // [호출 계약: Slice begin/end와 ConstIterator 순회]
    // (1) range-for는 유효한 const Slice lvalue를 수신자로 삼고 같은 저장소의 경계 두 개를 가진다.
    // (2) begin()/end(), iterator !=, 전위 ++, 단항 *가 선택되고 *는 const Entry&를 반환한다.
    // (3) begin/end에는 인자가 없고 비교는 두 iterator를 빌린다. ++는 지역 iterator만 수정하며 *는
    //     end 전 위치만 허용한다. 원소 소유권은 이동하지 않는다.
    // (4) 경계 반환은 숨은 반복 변수, != bool은 조건 분기, * 참조는 entry의 const lvalue 바인딩에 쓴다.
    // (5) 지역 iterator와 total만 바뀌고 slice/owned_ 원소는 유지된다.
    // (6) 각 연산 O(1), 전체 O(count), 무할당·비무효화다. 원본 수명·비변경·end 비역참조가 전제이며
    //     다른 실행 흐름의 쓰기와 겹치면 데이터 경쟁이다.
    // 대표 문서: ../standard-library/algorithms-and-ranges.md
    for (const Entry& entry : slice) {
        total += entry.points;
    }
    return SliceResult{count, total};
}

int main() {
    // [생성 계약: Storage initializer_list와 ScoreBoard 값 매개변수]
    // (1) board와 Storage 목적 객체는 아직 없고 다섯 Entry aggregate prvalue가 준비된다.
    // (2) vector(initializer_list<Entry>, allocator 기본값)과 explicit ScoreBoard(Storage)가 선택된다.
    // (3) 목록의 {10},{20},{30},{40},{50}은 유효한 Entry 값이며 vector가 const 목록 원소를 복사 소유한다.
    // (4) 생성자 반환값은 없고 ScoreBoard prvalue가 board 객체에 직접 구성된다.
    // (5) board가 다섯 독립 원소를 소유하고 임시 목록/값 매개변수는 full-expression 뒤 파괴된다.
    // (6) 시간·공간 O(5), 저장소 할당 실패가 가능하다. 새 객체라 무효화 대상이 없고 목록 수명은 충분하며
    //     단일 흐름에서 구성한다. 같은 타입 prvalue의 직접 구성에는 중간 ScoreBoard 복사/이동이 없다.
    // 대표 문서: ../standard-library/containers-and-views.md
    const ScoreBoard board{ScoreBoard::Storage{{10}, {20}, {30}, {40}, {50}}};

    // board는 이름 있는 const lvalue라 수명이 main 끝까지 유지된다. Slice prvalue는 반복자만 복사하며,
    // offset 1부터 세 원소 20,30,40을 빌린다. const_iterator 역참조는 const Entry&에만 바인딩된다.
    const ScoreBoard::Slice middle{board.slice(1, 3)};
    const SliceResult result{evaluate(middle)};

    // [호출 계약: std::ostream 정수/문자 삽입 연쇄]
    // (1) 수신자는 정상 상태의 std::cout/std::ostream lvalue이고 result는 살아 있다.
    // (2) size_t 기반 unsigned 정수와 int에는 ostream 멤버 overload, 공백/개행에는
    //     operator<<(std::ostream&, char) 비멤버 overload가 선택된다.
    // (3) result.count/result.total의 const lvalue 값과 char prvalue 둘을 읽으며 소유권을 옮기지 않는다.
    // (4) 각 호출이 같은 std::ostream&를 반환해 다음 수신자로 쓰고 마지막 반환만 버린다.
    // (5) 성공하면 `3 90\n`을 버퍼에 더하고 result/board/middle은 그대로다.
    // (6) 복잡도와 비용은 숫자 변환·locale·버퍼·장치에 의존하고 버퍼 할당, 상태 비트, 설정 예외가 가능하다.
    //     수명·반복자 무효화는 없고 단일 실행 흐름이라 cout 동시 기록 문제도 없다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << result.count << ' ' << result.total << '\n';

    // 기계 관점에서는 size/주소 load, clamp 비교·조건 분기, 반복자 덧셈, points load와 total store,
    // 스트림 버퍼 간접 호출이 생길 수 있다. 실제 인라인·분기 제거·할당·복사 생략과 명령 형태는 CPU,
    // ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.
    return 0;
}
