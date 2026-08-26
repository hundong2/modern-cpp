/*
문제 ID·제목: BOJ 6549 - 히스토그램에서 가장 큰 직사각형
출처: Baekjoon Online Judge
출처 URL: https://www.acmicpc.net/problem/6549

문제 요약:
너비가 모두 1이고 바닥선이 같은 직사각형 막대들이 왼쪽부터 나열된 히스토그램이 주어진다. 막대의
순서를 바꾸거나 일부만 잘라 옮길 수는 없다. 연속한 막대 구간을 골라 그 구간의 최소 높이만큼 세운
직사각형 가운데 가능한 최대 넓이를 구한다. 이 설명은 저작권이 있는 원문 전체를 복제하지 않고 핵심
조건을 한글로 다시 쓴 것이다.

입력:
여러 테스트 케이스가 이어진다. 각 케이스는 막대 수 n 다음에 n개의 높이 h1..hn이 주어진다. n만 0인
줄이 전체 입력의 끝이며 그 줄은 답을 출력하지 않는다. 줄바꿈 위치와 관계없이 공백으로 구분된 정수를
읽으면 된다.

출력:
각 테스트 케이스의 가장 큰 직사각형 넓이를 한 줄에 하나씩 출력한다.

제약:
1 <= n <= 100,000, 0 <= hi <= 1,000,000,000이다. 최대 넓이는 32비트 int를 넘을 수 있으므로
long long을 사용한다. 각 막대는 단조 스택에 최대 한 번 들어가고 한 번 나오므로 테스트 케이스 하나당
O(n) 시간, O(n) 추가 공간에 해결한다.

예제:
입력
7 2 1 4 5 1 3 3
4 1000 1000 1000 1000
0

출력
8
4000
*/

// <algorithm>은 두 넓이 중 큰 값을 고르는 std::max 함수 템플릿을 선언한다.
#include <algorithm>
// <cstddef>는 vector 크기와 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력·출력과 고속 입출력 설정을 선언한다.
#include <iostream>
// <vector>는 높이 배열과 단조 스택을 동적으로 소유하는 std::vector를 선언한다.
#include <vector>

// 공용 알고리즘 문서: ../algorithm/monotonic-stack-largest-rectangle.md
// struct는 기본 접근이 public이다. 아직 오른쪽 경계가 정해지지 않은 막대의 높이와 가장 왼쪽 시작을 묶는다.
struct OpenBar {
    long long height{};     // 최대 1e9인 높이를 저장하고 넓이 곱셈도 long long으로 유지한다.
    std::size_t start{};    // 이 높이 이상이 연속된 가장 왼쪽 0기반 위치다.
};

// heights는 호출자가 소유한 연속 높이 배열을 const 참조로 빌리며 복사하거나 수명을 연장하지 않는다.
[[nodiscard]] long long largest_rectangle(const std::vector<long long>& heights) {
    std::vector<OpenBar> pending{}; // 높이가 엄격히 증가하는 미완료 막대들을 vector로 스택처럼 소유한다.
    // vector<OpenBar>::reserve(size_type)는 heights.size() 값을 복사해 최소 용량을 요청하고 void를 반환한다.
    // 호출 전 pending은 비어 있고 성공 뒤 size는 0인 채 capacity만 늘 수 있다. O(N) 할당과 bad_alloc 가능성이 있다.
    // 이후 최대 N개 push_back에서 재할당을 피하며 현재 원소가 없어 무효화할 관찰자는 없다.
    pending.reserve(heights.size());

    long long best{}; // long long{}는 0으로 값 초기화되며 지금까지 확정한 최대 넓이를 저장한다.

    // index==heights.size()인 마지막 반복은 높이 0 센티널로 남은 모든 막대의 오른쪽 경계를 확정한다.
    for (std::size_t index{}; index <= heights.size(); ++index) {
        // vector::size() const noexcept는 인자 없이 원소 수 size_type을 반환하고 heights를 바꾸지 않는다.
        // vector::operator[](size_type)은 index 값을 받고 범위 검사 없이 const long long&를 O(1)에 반환한다.
        // index<size()인 분기에서만 호출하고 즉시 값을 복사하므로 heights의 상태·소유권·참조 수명은 유지된다.
        // 조건 연산자는 끝 위치면 0, 아니면 operator[]로 읽은 long long 값을 선택해 current_height를 초기화한다.
        const long long current_height{index == heights.size() ? 0 : heights[index]};
        std::size_t start{index}; // 새 높이가 왼쪽으로 확장할 수 있는 시작을 현재 위치로 초기화한다.

        // 불변식: pending의 높이는 아래에서 위로 엄격히 증가하고 각 start는 그 높이가 이어질 수 있는 최좌측이다.
        // empty() const noexcept는 데이터 인자 없이 bool을 O(1)에 반환하고 pending을 유지해 back() 전제를 검사한다.
        while (!pending.empty() && pending.back().height > current_height) {
            // back()은 비어 있지 않은 vector의 마지막 OpenBar&를 O(1)에 반환한다. 값을 복사해 pop 뒤에도 보존한다.
            // 반환 참조 자체는 pop_back()에서 무효화되므로 참조가 아닌 독립 객체 closing에 저장한다.
            const OpenBar closing{pending.back()};
            // pop_back()은 마지막 원소를 파괴해 size를 1 줄이고 void를 반환한다. capacity와 앞 원소는 유지된다.
            // O(1)이고 제거 원소의 포인터·참조·반복자는 무효화되며 비어 있지 않다는 전제를 위에서 만족한다.
            pending.pop_back();

            // index-closing.start는 closing.height 이상인 연속 구간 너비다. n<=100000이라 long long 변환이 안전하다.
            const long long width{static_cast<long long>(index - closing.start)};
            const long long area{closing.height * width}; // 곱셈은 두 long long 피연산자로 최대 1e14를 안전하게 담는다.
            // std::max<long long>(const T&,const T&)는 best와 area lvalue를 const 참조로 빌리고 더 큰 값의 const T&를 반환한다.
            // 반환값을 best에 복사 대입해 최대값만 갱신한다. 입력 두 변수는 유지되고 O(1), 할당·예외가 없다.
            best = std::max(best, area);
            start = closing.start; // 더 낮은 현재 막대는 닫힌 막대의 왼쪽 시작까지 확장할 수 있다.
        }

        // 같은 높이는 더 이른 start를 이미 가진 기존 원소 하나로 대표한다. 0 센티널은 스택에 넣지 않는다.
        if (current_height > 0 &&
            (pending.empty() || pending.back().height < current_height)) {
            // vector::push_back(OpenBar&&)은 OpenBar prvalue의 두 값을 새 끝 원소로 이동/복사 구성하고 void를 반환한다.
            // 성공 뒤 size가 1 늘고 높이 증가 불변식이 유지된다. 예약 안에서는 재할당이 없고 상각 O(1)이다.
            // 예약을 넘는 일반 경우에는 재할당으로 기존 포인터·참조·반복자가 모두 무효화되고 bad_alloc이 가능하다.
            pending.push_back(OpenBar{current_height, start});
        }
    }

    // 센티널 반복 뒤 pending은 비고 모든 후보 면적이 best에 반영되었다. long long 값을 복사 반환한다.
    return best;
}

int main() {
    // sync_with_stdio(false)는 bool false를 받아 C/C++ 스트림 동기화를 끄고 이전 bool 반환은 버린다.
    // 호출 뒤 전역 스트림 설정이 바뀌며 C stdio와 임의 순서로 섞지 않는다. 스레드 동기화를 제공하지 않는다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 널 ostream*을 받아 입력 전 cout 자동 flush 연결을 해제하고 이전 포인터 반환은 버린다.
    // cin/cout 수명과 소유권은 유지되며 대화형 문제가 아니므로 수동 프롬프트 flush가 필요 없다.
    std::cin.tie(nullptr);

    while (true) {
        std::size_t count{}; // size_t{}는 0이며 한 테스트 케이스의 막대 수를 음수 없이 저장한다.
        // operator>>(size_t&)는 count lvalue를 갱신하고 istream&를 반환한다. bool 문맥은 입력 상태를 검사한다.
        // EOF·형식 실패면 상태 비트를 남기고 false가 되어 더 읽지 않고 정상 종료한다.
        if (!(std::cin >> count)) {
            break;
        }
        if (count == 0U) { // 종료 센티널은 높이 배열을 만들거나 답을 출력하지 않는다.
            break;
        }

        // vector<long long>(count)는 count개의 0을 값 초기화해 연속 저장소에 소유한다.
        // O(N) 시간·공간이며 할당 실패 시 bad_alloc이 가능하다. 이후 크기를 바꾸지 않아 원소 참조가 안정적이다.
        std::vector<long long> heights(count);
        // 범위 for의 long long&는 각 vector 원소 lvalue를 차례로 빌리며 복사하지 않는다.
        for (long long& height : heights) {
            // operator>>(long long&)은 입력 값을 height에 저장하고 스트림 위치·상태를 갱신한다.
            // 반환 istream&는 버리며 문제 입력 계약상 모든 높이가 존재하고 허용 범위 안이다.
            std::cin >> height;
        }

        // largest_rectangle은 heights를 const 참조로 빌리고 long long을 반환한다. vector 소유권·크기는 유지된다.
        // operator<<는 반환값과 개행을 cout 버퍼에 쓰고 ostream&를 연쇄하며 최종 참조는 버린다.
        std::cout << largest_rectangle(heights) << '\n';
    }

    // 각 막대는 스택에 최대 한 번 push/pop되므로 케이스당 O(n) 시간, 높이와 스택을 합쳐 O(n) 공간이다.
    return 0;
}
