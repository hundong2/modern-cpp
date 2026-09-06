/*
문제 ID·제목: CSES 2195 - Convex Hull(볼록 껍질)
출처: CSES Problem Set / Geometry
출처 URL: https://cses.fi/problemset/task/2195

문제 요약:
서로 다른 평면 위 점들이 주어질 때, 그 점들을 모두 포함하는 가장 작은 볼록 다각형의 경계에 놓인
점을 전부 구한다. 아래 설명은 저작권이 있는 원문 전체를 복제하지 않고 풀이에 필요한 조건을 한글로
재서술한 것이다.

입력:
첫 줄에 점의 수 n이 주어진다. 다음 n줄에는 한 점의 정수 좌표 x, y가 한 줄에 하나씩 주어진다.

출력:
첫 줄에 볼록 껍질 경계에 놓인 점의 수 k를 출력한다. 이어지는 k줄에 그 점들의 좌표를 출력한다.
허용되는 답이 여러 개라면 순서는 자유지만, 모서리 위에 일직선으로 놓인 점도 빠짐없이 출력해야 한다.

공식 제약:
3 <= n <= 200,000, -1,000,000,000 <= x,y <= 1,000,000,000.
모든 입력 점은 서로 다르고 볼록 껍질의 넓이는 양수다. 시간 제한은 1초, 메모리 제한은 512MB다.

공식 예제:
입력
6
2 1
2 5
3 3
4 3
4 4
6 3

가능한 출력
4
2 1
2 5
4 4
6 3
*/

// <algorithm>은 임의 접근 구간을 비교자에 따라 정렬하는 std::sort를 선언한다.
#include <algorithm>
// <cstddef>는 컨테이너 크기와 인덱스에 쓰는 부호 없는 정수 타입 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 좌표와 외적을 정확히 64비트 부호 있는 정수로 계산할 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 입력·출력 객체와 정수/문자 스트림 연산자를 선언한다.
#include <iostream>
// <vector>는 입력 점과 볼록 껍질 점을 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// using은 새 타입을 만들지 않고 정확한 64비트 부호 있는 정수 타입에 읽기 쉬운 별칭을 붙인다.
using Coordinate = std::int64_t;

// struct는 class와 달리 멤버가 기본 public이다. 좌표 두 개를 함께 운반하는 단순 값 객체에 알맞다.
// 중괄호 기본 멤버 초기화는 Point{}처럼 값 초기화할 때 x와 y를 0으로 만든다.
// 사용자 정의 생성자가 없어 멤버 초기화 목록이나 explicit가 필요 없다. 단일 인자 생성자가 있었다면
// explicit로 의도하지 않은 암시적 Point 변환을 막아야 한다.
struct Point {
    Coordinate x{};
    Coordinate y{};
};

// std::sort에 전달할 비교 함수다. 두 매개변수는 const lvalue reference라 점을 복사하거나 소유하지 않는다.
// 반환형 bool은 left가 right보다 사전순으로 앞서는지를 뜻한다. x가 같을 때 y까지 비교하므로 서로 다른
// 점들에 strict weak ordering을 제공한다.
bool point_less(const Point& left, const Point& right) {
    if (left.x != right.x) {
        return left.x < right.x;
    }
    return left.y < right.y;
}

// (first -> second)와 (first -> third)의 2차원 외적을 반환한다.
// 양수는 반시계 방향, 0은 공선, 음수는 시계 방향 회전을 뜻한다.
Coordinate cross(const Point& first, const Point& second, const Point& third) {
    // 각 좌표 차의 절댓값은 최대 2*10^9, 곱은 최대 4*10^18, 두 곱의 차는 최대 8*10^18이다.
    // 이는 int64_t 최댓값 9,223,372,036,854,775,807보다 작으므로 공식 제약 아래 signed overflow가 없다.
    // 제약을 더 키운 변형에서는 __int128 같은 더 넓은 중간 타입이 필요하다. 부호 있는 overflow는 C++에서
    // 미정의 동작이므로 “우연히 2의 보수로 감긴다”고 기대하면 안 된다.
    // 기계 관점에서는 좌표 로드, 뺄셈, 곱셈, 뺄셈과 부호 비교가 후보지만 실제 명령·레지스터 배치는
    // CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리로 단정하지 않는다.
    return (second.x - first.x) * (third.y - first.y)
         - (second.y - first.y) * (third.x - first.x);
}

// 매개변수는 x,y 사전순으로 정렬된 vector를 const 참조로 빌린다. 함수는 입력을 수정하거나 소유하지 않고,
// 반환 vector가 껍질 점들의 독립적인 소유권을 갖는다. 공식 조건 n>=3과 양의 넓이를 전제로 한다.
std::vector<Point> build_convex_hull(const std::vector<Point>& sorted_points) {
    // [호출 계약: std::vector<Point> 기본 생성자]
    // (1) 아직 존재하지 않는 정확한 타입 std::vector<Point> 객체 hull을 구성한다.
    // (2) 선택 시그니처는 vector()이고 템플릿 인자는 Point, 기본 allocator<Point>다.
    // (3) 명시적 인자는 없고 외부 저장소나 소유권을 받지 않는다.
    // (4) 생성자는 반환값이 없다.
    // (5) 호출 뒤 hull은 size 0인 유효한 소유 컨테이너이며 함수가 끝날 때 원소와 저장소를 정리한다.
    // (6) 정확한 기본 allocator 특수화에서는 상수 시간·무할당·noexcept이고 기존 관찰자의 무효화는
    //     없다. 외부 동시 접근이 없는 지역 객체라 데이터 경쟁도 없다.
    std::vector<Point> hull;

    // [호출 계약: const std::vector<Point>::size]
    // (1) 수신 객체는 정렬 완료·수명 유효·size>=3인 const std::vector<Point> lvalue sorted_points다.
    // (2) 시그니처는 constexpr size_type size() const noexcept다. 매개변수는 없다.
    // (3) 입력 인자나 소유권 이전은 없다.
    // (4) 반환형 size_type(std::size_t)은 현재 점 수이며 point_count에 값으로 저장해 사용한다.
    // (5) 호출 뒤 vector, 원소, 용량, 인자 상태와 수명은 변하지 않는다.
    // (6) O(1), 무할당·비무효화·noexcept이며 동시 변경이 없어 스레드 안전 전제도 만족한다.
    const std::size_t point_count{sorted_points.size()};

    // [호출 계약: std::vector<Point>::reserve]
    // (1) 수신 객체는 size=0, capacity는 구현이 정한 값인 유효한 hull lvalue다.
    // (2) 시그니처는 void reserve(size_type new_capacity)다.
    // (3) 인자는 point_count*2인 std::size_t prvalue이며 공식 상한 400,000이라 max_size 이하다. 소유권은 없다.
    // (4) 반환형 void라 사용할 반환값이 없다.
    // (5) 성공 후 capacity>=2*n이고 size는 0이다. 이후 최대 2*n-1회 push가 재할당 없이 가능하다.
    // (6) 원소 이동 복잡도는 O(size)=O(0)이고 실제 저장소 할당 비용은 allocator/시스템에 의존한다.
    //     length_error/bad_alloc 가능성이 있고 실패 시 이 Point 특수화는 강한 보장을 갖는다. 재할당했다면
    //     기존 관찰자가 모두 무효지만 현재는 원소/관찰자가 없고, 외부 동시 접근은 없다.
    hull.reserve(point_count * std::size_t{2});

    // lower chain 불변식: hull은 지금까지 본 점 중 아래쪽 경계를 사전순으로 담고, 연속한 세 점의 외적은
    // 항상 0 이상이다. 외적 0을 제거하지 않아 경계 선분 위의 모든 공선점을 보존한다.
    for (std::size_t index{}; index < point_count; ++index) {
        // [호출 계약: vector::size/operator[]/back/pop_back]
        // (1) 수신자는 용량을 예약한 hull lvalue와 size=n인 const sorted_points lvalue다. while 본문에서는
        //     hull.size()>=2라 back과 size-2가 유효하고, index는 [0,n)다.
        // (2) size() const noexcept, Point& operator[](size_type), const Point& operator[](size_type) const,
        //     Point& back(), void pop_back() 오버로드를 선택한다.
        // (3) [] 인자는 각각 size-2와 index인 size_t prvalue이며 소유권을 넘기지 않는다. 나머지는 인자가 없다.
        // (4) size는 개수, []/back은 살아 있는 원소 참조를 반환해 cross가 즉시 읽고, pop_back은 void다.
        // (5) 관찰 호출은 상태를 안 바꾸고 pop은 마지막 Point 수명을 끝내 size를 1 줄인다.
        // (6) 모두 O(1)·무할당이다. []/back 범위 위반과 빈 pop은 UB지만 조건이 배제한다. pop은 지운 원소와
        //     과거 end 관찰자를 무효화하며, 저장 참조가 없고 Point 소멸은 예외가 없으며 단일 스레드다.
        while (hull.size() >= std::size_t{2}
               && cross(hull[hull.size() - std::size_t{2}], hull.back(), sorted_points[index]) < Coordinate{0}) {
            hull.pop_back();
        }

        // [호출 계약: std::vector<Point>::push_back(const Point&)]
        // (1) 수신자는 size<2*n, capacity>=2*n인 hull lvalue다.
        // (2) 선택 시그니처는 void push_back(const value_type& value), value_type은 Point다.
        // (3) 인자는 sorted_points[index]라는 const Point lvalue다. 읽기 참조이며 입력 소유권은 유지된다.
        // (4) 반환형 void라 사용할 반환값이 없다.
        // (5) Point 값 하나를 복사 소유하고 size가 1 증가한다. 입력 점과 sorted_points는 변하지 않는다.
        // (6) 예약 용량 안이라 O(1)·무할당이고 재할당하지 않는다. 일반 재할당 시 모든 관찰자가 무효지만
        //     여기서는 과거 end만 바뀐다. Point 복사는 예외가 없고 외부 동시 접근은 없다.
        hull.push_back(sorted_points[index]);
    }

    // lower_size-1 위치의 사전순 최대점은 upper chain의 출발점으로도 사용한다.
    const std::size_t lower_size{hull.size()};

    // `index-- > 0`은 감소 전 값을 0과 비교한 뒤 감소한다. 첫 본문 인덱스는 n-2, 마지막은 0이므로
    // unsigned std::size_t가 0 아래로 내려간 값을 본문에서 쓰지 않는다. 재귀가 없어 호출 스택 깊이는 O(1)다.
    for (std::size_t index{point_count - std::size_t{1}}; index-- > std::size_t{0};) {
        // upper chain에서도 엄격한 시계 방향(cross<0)일 때만 제거한다. hull.size()>lower_size이면
        // 사전순 최대점 뒤에 upper 점이 하나 이상 있어 back과 size-2가 모두 upper 회전의 두 선행점이다.
        // 앞의 vector 관찰/pop 계약이 그대로 적용되며 각 점은 한 번 push되고 최대 한 번 pop된다.
        while (hull.size() > lower_size
               && cross(hull[hull.size() - std::size_t{2}], hull.back(), sorted_points[index]) < Coordinate{0}) {
            hull.pop_back();
        }
        // 앞의 push_back 계약과 같다. 예약 용량 때문에 재할당 없이 입력 점 값을 복사 소유한다.
        hull.push_back(sorted_points[index]);
    }

    // reverse 순회가 마지막에 추가한 사전순 최소점은 lower의 첫 점과 중복된다. 공식 양의 넓이 조건에서는
    // lower와 upper가 공유하는 다른 점이 없으므로 이 한 점만 제거하면 각 경계점이 정확히 한 번 남는다.
    // 앞의 pop_back 계약과 같고 hull은 최소 네 번 push되어 비어 있지 않으므로 전제조건을 만족한다.
    hull.pop_back();

    // [반환 계약: NRVO 또는 std::vector<Point> 이동 생성]
    // (1) 원본은 모든 경계점을 소유하는 유효한 지역 std::vector<Point> lvalue hull이고 별도 수신자는 없다.
    // (2) NRVO가 적용되면 생성자 호출이 생략되고, 아니면 vector(vector&& other) 이동 생성자가 선택된다.
    // (3) fallback 인자는 반환 문맥에서 xvalue로 취급되는 hull이며 저장소 소유권을 결과 객체에 넘긴다.
    // (4) 생성자는 반환값이 없고 함수의 반환형 vector 결과 객체가 완성되어 호출자가 사용한다.
    // (5) NRVO면 결과 위치에 직접 존재한다. fallback이면 원본은 유효하지만 미지정 상태가 된 뒤 파괴되고,
    //     원소 저장소와 관찰자는 결과가 소유한다. sorted_points의 수명과는 독립적이다.
    // (6) NRVO는 작업이 없고 기본 allocator의 이동 생성은 O(1)·무할당·noexcept다. 결과 vector 수명 동안
    //     원소가 유효하며 외부 동시 접근 보장은 호출자가 책임진다.
    return hull;
}

int main() {
    // [호출 계약: std::ios::sync_with_stdio]
    // (1) 정적 함수라 수신 인스턴스는 없고 표준 스트림에서 아직 입출력하지 않은 상태다.
    // (2) 시그니처는 static bool std::ios::sync_with_stdio(bool sync = true)다.
    // (3) 인자는 bool prvalue false이며 소유권 의미가 없고 허용된 두 bool 값 중 하나다.
    // (4) 반환형 bool은 이전 동기화 설정이지만 사용하지 않는다.
    // (5) C stdio와 C++ 표준 스트림의 동기화를 해제하되 스트림 수명과 소유권은 유지한다.
    // (6) 첫 I/O 뒤 효과는 구현별이므로 지금 호출한다. 복잡도·할당·예외는 별도 규정이 없고 C/C++ I/O
    //     혼용 순서나 애플리케이션 수준 스레드 안전을 보장하지 않는다.
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::cin.tie(nullptr)]
    // (1) 수신자는 정상 상태인 std::cin, 정확한 타입은 std::istream인 유효한 lvalue다.
    // (2) 선택 시그니처는 std::ostream* tie(std::ostream* tied_stream) setter 오버로드다.
    // (3) nullptr prvalue가 null std::ostream*로 변환되며 비소유이고 null은 허용된다.
    // (4) 반환형 std::ostream*는 이전 tie 대상의 비소유 포인터지만 사용하지 않는다.
    // (5) 입력 전 자동 flush 연결이 사라지고 두 스트림의 상태·수명·소유권은 유지된다.
    // (6) O(1)·무할당·비무효화다. 오류 보고나 스레드 동기화를 추가하지 않으며 대화형 입력이 아니다.
    std::cin.tie(nullptr);

    int point_count{}; // 기본 타입 int를 0으로 값 초기화하며 공식 최댓값 200,000을 안전하게 담는다.

    // [호출 계약: std::istream의 int 추출]
    // (1) 수신자는 입력 전 정상 상태인 std::cin/std::istream lvalue다.
    // (2) 선택 시그니처는 std::istream& operator>>(int& value) 멤버 오버로드다.
    // (3) 인자는 수정 가능한 int lvalue point_count이며 비소유 참조이고 공식 입력값은 [3,200000]이다.
    // (4) 반환형은 같은 std::istream&지만 연쇄하지 않고 버린다.
    // (5) 성공하면 point_count와 읽기 위치가 갱신되고 소유권/수명은 변하지 않는다.
    // (6) 비용은 소비 문자·locale·버퍼에 의존한다. EOF/형식/범위 실패는 상태 비트를 세우고 예외 마스크에
    //     따라 ios_base::failure를 던질 수 있다. 별도 할당·참조 무효화·스레드 보장은 없다.
    std::cin >> point_count;

    // [호출 계약: std::vector<Point> count 생성자]
    // (1) 아직 존재하지 않는 정확한 타입 std::vector<Point> 객체 points를 구성한다.
    // (2) 선택 시그니처는 explicit vector(size_type count, const Allocator& = Allocator())다.
    // (3) 인자는 point_count를 변환한 size_t prvalue이며 [3,200000], max_size 이하이고 소유권은 없다.
    // (4) 생성자는 반환값이 없다.
    // (5) 성공 후 size=count이고 각 Point는 값 초기화되어 두 좌표가 0이며 vector가 저장소를 소유한다.
    // (6) O(n) 초기화와 저장소 할당이 필요하다. length_error/bad_alloc 가능성이 있고 실패 시 완성 객체가
    //     없으며 무효화할 기존 관찰자도 없다. 원소 수명은 points의 RAII 수명에 묶이고 단일 스레드다.
    std::vector<Point> points(static_cast<std::size_t>(point_count));

    for (std::size_t index{}; index < static_cast<std::size_t>(point_count); ++index) {
        // [호출 계약: vector 비const operator[] + int64_t 입력 추출 체인]
        // (1) vector 수신자는 size=n인 points lvalue, 스트림 수신자는 유효한 std::cin lvalue다.
        // (2) Point& vector<Point>::operator[](size_type)와 std::istream의 대응 signed 64-bit 정수
        //     operator>>(std::int64_t&) 오버로드 두 번이 선택된다.
        // (3) [] 인자 index는 [0,n) size_t prvalue다. 추출 인자는 그 Point의 x/y라는 수정 가능한
        //     int64_t lvalue이며 비소유 참조이고 공식 좌표 범위 안이다.
        // (4) []는 Point&를 멤버 선택에 쓰고, 각 >>는 같은 istream&를 반환해 연쇄하며 마지막은 버린다.
        // (5) 성공 후 x,y와 스트림 위치가 갱신되고 vector 크기·용량·소유권·참조 수명은 변하지 않는다.
        // (6) []는 O(1)·무할당이고 범위 위반은 UB이나 루프가 배제한다. 추출 비용/실패/예외/스레드 계약은
        //     앞과 같고 실패 시 상태 비트가 설정된다. vector 관찰자는 무효화되지 않는다.
        std::cin >> points[index].x >> points[index].y;
    }

    // [호출 계약: vector::begin/end + std::sort]
    // (1) points는 size=n이고 서로 다른 Point를 소유하는 유효한 non-const vector lvalue다.
    // (2) iterator begin() noexcept, iterator end() noexcept와 template<class RandomIt, class Compare>
    //     void std::sort(RandomIt first, RandomIt last, Compare comp)를 RandomIt=vector<Point>::iterator,
    //     Compare=bool(*)(const Point&,const Point&)로 선택한다.
    // (3) begin/end 반환 반복자가 [first,last) 반열린 유효 구간을 만들고 point_less 함수 포인터 prvalue는
    //     비소유 호출 대상을 뜻한다. 비교자는 strict weak ordering이어야 하며 여기서 만족한다.
    // (4) begin/end는 반복자를 반환해 sort 인자로 사용하고 sort 반환형 void에는 사용할 값이 없다.
    // (5) 성공 후 같은 저장소와 size/capacity를 유지하며 원소 값만 (x,y) 오름차순으로 재배열된다.
    // (6) 표준이 요구하는 비교 횟수는 O(n log n)이고 원소 이동·추가 작업은 구현에 의존한다. vector
    //     재할당/반복자 무효화는 없다. 원소 이동·비교가 던지면 전파될 수 있으나 Point 연산은 예외가 없다.
    //     동시 접근 보장은 없고 이 코드는 단일 스레드다.
    std::sort(points.begin(), points.end(), point_less);

    // 함수 호출식은 vector prvalue다. C++17 이후 이 prvalue가 호출자 hull 결과 객체를 직접 초기화하므로
    // 호출 경계에서 별도 이동 생성은 없다. 함수 내부 지역 객체에 NRVO가 적용되지 않은 경우에만 앞서 설명한
    // 한 번의 fallback 이동이 결과 객체를 만든다. build 함수가 빌린 const 참조는 호출 동안 유효하다.
    // 공용 알고리즘 문서: ../algorithm/convex-hull-monotone-chain.md
    std::vector<Point> hull{build_convex_hull(points)};

    // [호출 계약: std::ostream의 long long 삽입]
    // (1) 수신자는 출력 가능한 std::cout/std::ostream lvalue이고 hull은 수명 유효하다.
    // (2) 선택 시그니처는 std::ostream& operator<<(long long value) 멤버 오버로드다.
    // (3) 인자는 hull.size()를 long long으로 명시 변환한 prvalue이며 [3,n], 소유권 의미가 없다.
    // (4) 반환형은 같은 std::ostream&지만 여기서는 사용하지 않는다.
    // (5) 숫자의 문자 표현이 버퍼에 기록되고 hull과 인자는 변하지 않는다.
    // (6) 비용은 자릿수·locale·버퍼/장치에 의존한다. 실패는 상태 비트에 남고 예외 마스크에 따라
    //     ios_base::failure가 가능하다. 별도 관찰자 무효화나 논리적 다중 스레드 원자 출력 보장은 없다.
    // 감사 요약: cout 수신자와 크기 정수 인자를 받고 ostream& 반환은 버린다. 출력 복잡도는 자릿수·버퍼에
    // 의존하고 상태 오류·예외 가능성이 있으며 hull 수명과 관찰자를 무효화하지 않는다.
    std::cout << static_cast<long long>(hull.size());

    // [호출 계약: operator<<(std::ostream&, char)]
    // (1) 수신자는 앞 삽입 뒤에도 유효한 std::cout/std::ostream lvalue다.
    // (2) 선택 시그니처는 std::ostream& operator<<(std::ostream&, char) 비멤버 오버로드다.
    // (3) 첫 인자는 비소유 ostream lvalue, 둘째는 문자 '\n' char prvalue이며 모두 유효하다.
    // (4) 반환형 std::ostream&는 같은 cout을 뜻하지만 사용하지 않는다.
    // (5) 개행 문자를 버퍼에 추가하되 endl 조작자와 달리 강제 flush하지 않고 다른 객체는 변하지 않는다.
    // (6) O(1) 문자 처리 뒤 버퍼/장치 비용이 들며 할당은 버퍼 구현에 의존한다. 실패/예외/스레드 보장은
    //     앞 삽입과 같고 vector 참조·수명에는 영향이 없다.
    std::cout << '\n';

    for (std::size_t index{}; index < hull.size(); ++index) {
        // 앞서 명시한 vector::operator[] 계약과 같다. index<hull.size()라 범위 안이며 반환 Point&를
        // const 참조에 바인딩해 읽기만 한다. 이 참조는 루프 본문 동안 hull이 재할당되지 않아 유효하다.
        const Point& point{hull[index]};

        // [호출 계약: std::ostream의 signed 64-bit 정수 삽입]
        // (1) 수신자는 유효한 std::cout/std::ostream lvalue이고 point는 살아 있는 const Point lvalue다.
        // (2) std::int64_t가 별칭인 구현의 대응 signed integer 멤버 operator<< 오버로드가 선택된다.
        // (3) 인자는 point.x라는 int64_t lvalue이며 값만 읽고 소유권을 전달하지 않는다.
        // (4) 반환형은 같은 std::ostream&이고 사용하지 않는다.
        // (5) 좌표의 문자 표현이 버퍼에 추가되고 point/hull/수명은 변하지 않는다.
        // (6) 비용은 자릿수·locale·버퍼에 의존한다. 상태 비트와 ios_base::failure 가능성, 무효화 없음,
        //     애플리케이션 수준 스레드 원자성 없음은 앞의 정수 삽입 계약과 같다.
        std::cout << point.x;
        // 앞의 char 삽입 계약과 같으며 공백 하나를 추가하고 반환 ostream&는 버린다.
        std::cout << ' ';
        // 앞의 signed 64-bit 삽입 계약과 같으며 y를 읽어 출력하고 Point는 변하지 않는다.
        std::cout << point.y;
        // 앞의 char 삽입 계약과 같으며 줄바꿈만 추가하고 강제 flush하지 않는다.
        std::cout << '\n';
    }

    return 0;
}
