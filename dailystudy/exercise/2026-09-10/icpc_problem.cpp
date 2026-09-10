/*
문제: CSES 1111 - Longest Palindrome
출처: https://cses.fi/problemset/task/1111/

문제 요약
- 소문자 영문자로 이루어진 문자열에서 앞뒤로 읽은 결과가 같은 연속 부분 문자열 중 가장 긴 하나를 찾는다.
- 최장 답이 여러 개면 어느 하나를 출력해도 된다.

입력
- 한 줄에 길이 n인 문자열 하나가 주어진다. 모든 문자는 a부터 z까지의 소문자다.

출력
- 가장 긴 팰린드롬 부분 문자열 하나를 출력한다.

공식 제약
- 1 <= n <= 1,000,000
- 시간 제한 1초, 메모리 제한 512MB

예제
- 입력: aybabtu
- 출력: bab

핵심
- 각 위치를 중심으로 하는 홀수 길이 반지름 odd와, 두 문자 사이를 중심으로 하는 짝수 길이 반지름 even을
  Manacher 알고리즘으로 구한다. 지금까지 가장 오른쪽까지 닿은 팰린드롬 left..right 안의 새 중심은
  대칭 중심의 이미 계산한 반지름을 안전한 범위까지만 재사용한다.
- right 바깥에서 성공하는 문자 비교는 전체 실행 동안 right를 최대 n번만 전진시킨다. 따라서 모든 중심에서
  단순 확장하는 O(n^2) 풀이와 달리 전체 O(n) 시간, 두 반지름 배열 O(n) 공간이다.
- 원문 전체를 복제하지 않고 학습에 필요한 조건과 예제를 한국어로 다시 설명했다.
*/

// <algorithm>은 mirror 반지름과 현재 오른쪽 경계까지의 거리 중 작은 값을 고르는 std::min을 선언한다.
#include <algorithm>
// <iostream>은 빠른 입력 설정, std::cin/std::cout, std::istream/std::ostream 연산을 선언한다.
#include <iostream>
// <string>은 입력과 반환 부분 문자열을 소유하는 std::string 및 substr를 선언한다.
#include <string>
// <vector>는 각 중심의 정수 반지름을 연속 저장하는 std::vector를 선언한다.
#include <vector>

// 공용 알고리즘 문서: ../algorithm/manacher-longest-palindromic-substring.md
[[nodiscard]] std::string longest_palindrome(const std::string& text) {
    // [호출 계약: const string::length()]
    // (1) 수신자는 공식 제약상 1개 이상 완성된 소문자를 소유하는 const std::string lvalue text다.
    // (2) 선택 멤버는 size_type length() const noexcept이며 데이터 인자는 없다.
    // (3) 숨은 const this만 빌리고 문자나 버퍼 소유권을 바꾸지 않는다.
    // (4) 문자 수를 나타내는 size_type prvalue를 n_size에 저장하고, 범위가 int 안임을 이용해 n에도 변환한다.
    // (5) text의 크기·용량·문자·참조는 그대로다.
    // (6) O(1), 무할당·비무효화·noexcept다. text가 살아 있어야 하고 동시 변경은 없어야 한다. 공식
    //     n<=1,000,000이므로 int 변환은 표현 가능하며, 이 전제 밖의 거대 문자열에는 별도 검사가 필요하다.
    // 대표 문서: ../standard-library/containers-and-views.md
    const auto n_size{text.length()};
    const int n{static_cast<int>(n_size)};

    // [생성 계약: vector fill 생성자 두 번]
    // (1) 목적 odd/even은 아직 없고 n_size는 표현 가능한 vector 크기이며 값 0은 유효한 int다.
    // (2) 각 호출은 vector(size_type count, const int& value, const Allocator& = Allocator())를 선택한다.
    //     템플릿 원소 타입은 int이고 allocator 인자는 기본값을 쓴다.
    // (3) 첫 인자 n_size는 size_type lvalue의 값을 복사하고, 둘째 0은 int prvalue가 const int&에 잠시
    //     바인딩된다. 반지름 원소 n개를 값 0으로 복사 구성하며 외부 소유권을 빌리지 않는다.
    // (4) 생성자 반환값은 없다. 성공하면 각 vector가 n개의 int와 연속 저장소를 독점 소유한다.
    // (5) n_size/text는 변하지 않고 odd/even의 size는 n, capacity는 최소 n이며 모든 원소는 0이다.
    // (6) 각각 시간·공간 O(n), 한 번의 연속 저장소 할당이 가능하다. 길이 초과 또는 할당 실패 예외가
    //     날 수 있고 그 목적 객체는 완성되지 않는다. 새 객체라 기존 반복자 무효화는 없고 동기화도 없다.
    // 대표 문서: ../standard-library/containers-and-views.md
    std::vector<int> odd(n_size, 0);
    std::vector<int> even(n_size, 0);

    int best_start{0};
    int best_length{1};

    // 홀수 팰린드롬 불변식: 루프 시작 때 [left,right]는 지금까지 찾은 홀수 팰린드롬 중 right가 가장 크다.
    // i가 그 안이면 mirror=left+right-i는 이미 처리됐고, mirror 반지름과 right-i+1 중 작은 부분은
    // 문자 비교 없이 팰린드롬임을 안다. 그 경계를 넘어서만 직접 확장한다.
    int left{0};
    int right{-1};
    for (int i{0}; i < n; ++i) {
        int radius{1};
        if (i <= right) {
            const int mirror{left + right - i};

            // [호출 계약: vector::operator[], std::min(const int&, const int&), string::operator[]]
            // (1) odd는 n개의 초기화된 int를 소유하고 mirror는 이미 계산된 [0,i) 인덱스다. text는 길이 n의
            //     const string이며 아래 while의 두 인덱스는 단락 평가로 [0,n)일 때만 접근한다.
            // (2) odd[mirror]는 int& operator[](size_type), text[index]는 const char& operator[](size_type) const,
            //     min은 template<class T> const T& min(const T&, const T&)에서 T=int를 선택한다.
            // (3) operator[] 인덱스는 non-negative int prvalue가 size_type으로 변환된다. min 첫 인자는
            //     odd 원소 int lvalue, 둘째는 right-i+1 int prvalue이며 둘 다 읽기 참조로 빌린다.
            // (4) vector []의 int& 반환과 min의 const int& 반환은 radius 값 복사에 쓰인다. string []의
            //     const char& 반환 두
            //     개는 == 비교의 char 값으로 쓰인다. 참조를 저장하거나 소유권을 옮기지 않는다.
            // (5) 이 읽기 호출들 뒤 컨테이너·문자열·원소·용량은 그대로이며 radius만 안전한 초기값을 얻는다.
            // (6) 각 연산 O(1), 무할당·비무효화다. []는 범위 검사나 예외를 제공하지 않아 인덱스가 범위를
            //     벗어나면 미정의 동작이다. 불변식과 while 조건이 범위를 보장하고 동시 쓰기는 없다.
            // 대표 문서: ../standard-library/containers-and-views.md
            // 대표 문서: ../standard-library/algorithms-and-ranges.md
            radius = std::min(odd[mirror], right - i + 1);
        }

        while (i - radius >= 0 && i + radius < n &&
               text[i - radius] == text[i + radius]) {
            ++radius;
        }
        odd[i] = radius;

        const int length{2 * radius - 1};
        const int start{i - radius + 1};
        if (length > best_length) {
            best_start = start;
            best_length = length;
        }

        if (i + radius - 1 > right) {
            left = i - radius + 1;
            right = i + radius - 1;
        }
    }

    // 짝수 정의: even[i]=k이면 [i-k, i+k) 즉 중심이 i-1과 i 사이인 길이 2k 구간이 팰린드롬이다.
    // mirror 식에 +1이 붙는 점과 기본 반지름이 0인 점이 홀수 버전과 다르다.
    left = 0;
    right = -1;
    for (int i{0}; i < n; ++i) {
        int radius{0};
        if (i <= right) {
            const int mirror{left + right - i + 1};
            // 위 vector []/min 계약을 그대로 재사용한다. mirror는 이미 계산된 유효 인덱스다.
            radius = std::min(even[mirror], right - i + 1);
        }

        while (i - radius - 1 >= 0 && i + radius < n &&
               text[i - radius - 1] == text[i + radius]) {
            ++radius;
        }
        even[i] = radius;

        const int length{2 * radius};
        const int start{i - radius};
        if (length > best_length) {
            best_start = start;
            best_length = length;
        }

        if (i + radius - 1 > right) {
            left = i - radius;
            right = i + radius - 1;
        }
    }

    // [호출 계약: const string::substr(pos, count)]
    // (1) 수신자는 길이 n인 const string lvalue text이고 best_start/best_length는 찾은 답의 유효한 경계다.
    // (2) 선택 멤버는 string substr(size_type pos=0, size_type count=npos) const &이고 두 인자를 모두 준다.
    // (3) best_start와 best_length는 non-negative int lvalue의 값을 size_type으로 변환해 복사한다.
    //     pos<=size이고 count는 size-pos 이하라 clamp에 의존하지 않으며 원본을 읽기만 한다.
    // (4) 지정 구간 문자를 독립 소유하는 std::string prvalue를 반환해 함수 결과 객체에 직접 구성한다.
    // (5) text와 반지름 배열은 변하지 않고 결과 문자열만 별도 버퍼/문자 수명을 소유한다.
    // (6) 시간·추가 공간 O(best_length), 저장소 할당 실패 예외가 가능하다. pos>size이면 범위 예외지만
    //     불변식상 일어나지 않는다. 원본 참조는 무효화되지 않고 결과는 text보다 오래 살아도 안전하다.
    // 대표 문서: ../standard-library/containers-and-views.md
    return text.substr(
        static_cast<decltype(text.length())>(best_start),
        static_cast<decltype(text.length())>(best_length));
}

int main() {
    // [호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 수신 객체 없는 static 함수이며 표준 C/C++ 스트림에 아직 입출력을 수행하지 않은 시작 상태다.
    // (2) 선택 시그니처는 static bool sync_with_stdio(bool sync=true)이고 유일한 인자는 false bool prvalue다.
    // (3) false는 C 스트림과 C++ 표준 스트림의 동기화를 끄라는 설정값이며 데이터나 소유권이 아니다.
    // (4) 이전 동기화 상태 bool을 반환하지만 대회 코드는 의도적으로 버린다.
    // (5) 이후 표준 C++ 스트림은 C stdio와 버퍼 순서를 자동 맞출 의무가 없어질 수 있다.
    // (6) 표준이 점근 복잡도를 정하지 않으며 구현 버퍼 작업이 가능하다. 첫 I/O 뒤 호출 효과는 구현에 따라
    //     달라질 수 있어 시작 때 한 번만 부른다. 스트림 객체 수명·스레드 안전 보장을 새로 만들지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::cin.tie(nullptr)]
    // (1) 수신자는 살아 있는 입력 스트림 std::cin lvalue이고 기본적으로 출력 스트림에 tie됐을 수 있다.
    // (2) 선택 멤버는 std::ostream* tie(std::ostream*)이며 nullptr 하나를 준다.
    // (3) nullptr은 새 연결 출력 스트림이 없다는 포인터 값이다. 객체 소유권이나 수명을 넘기지 않는다.
    // (4) 이전 tied std::ostream*를 반환하지만 다시 복구하지 않으므로 버린다.
    // (5) 이후 cin 입력 직전 자동 flush 대상이 없고 cout 수명/버퍼는 변하지 않는다.
    // (6) 표준은 점근 복잡도를 정하지 않는다. 무효화·할당을 요구하지 않으며 nullptr은 허용값이다.
    //     여러 흐름이 같은 스트림 설정을 동시에 바꾸지 않고, 프롬프트 수동 flush가 필요한 프로그램은 주의한다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cin.tie(nullptr);

    // [생성 계약: std::string 기본 생성]
    // (1) 목적 text는 아직 없고 allocator는 기본 구성 가능하다.
    // (2) basic_string() 기본 생성자가 선택되며 데이터 인자는 없다.
    // (3) 외부 문자/소유권 입력이 없고 기본 allocator를 사용한다.
    // (4) 생성자 반환값은 없으며 빈 문자열 객체가 생긴다.
    // (5) text의 길이는 0이고 유효한 null 종료 문자열 표현을 소유한다.
    // (6) 상수 시간으로 요구되며 구현상 빈 문자열 할당은 필수가 아니다. 예외·무효화 대상이 없고 한 흐름만 쓴다.
    // 대표 문서: ../standard-library/containers-and-views.md
    std::string text{};

    // [호출 계약: std::istream의 string 추출 operator>>]
    // (1) 수신/왼쪽 피연산자는 입력 가능한 std::cin의 std::istream lvalue, text는 빈 string lvalue다.
    // (2) operator>>(std::istream&, std::string&) overload가 선택된다.
    // (3) 첫 인자는 스트림을 non-const 참조로, 둘째는 쓸 문자열을 non-const 참조로 빌린다. 기본 skipws가
    //     선행 공백을 건너뛰고 다음 공백 전까지 문자를 허용하며 소유권을 넘기지 않는다.
    // (4) 같은 std::istream&를 반환하지만 연쇄/검사하지 않아 버린다.
    // (5) 성공하면 text가 입력 단어를 소유하고 위치가 전진한다. 실패하면 상태 비트가 서고 text는 비거나
    //     읽은 접두 상태일 수 있으나 공식 입력은 항상 성공한다.
    // (6) 소비 문자 수에 선형이고 문자열 재할당/할당 실패, 스트림 오류와 설정 예외가 가능하다. 재할당 시
    //     기존 문자 관찰자는 무효지만 아직 없다. 같은 스트림 동시 추출을 하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cin >> text;

    // [호출 계약: std::ostream의 string/char 삽입]
    // (1) 수신자는 출력 가능한 std::cout의 std::ostream lvalue다. longest_palindrome 반환 임시 string은
    //     첫 삽입을 마칠 때까지 살아 있고 개행은 char prvalue다.
    // (2) 문자열에는 operator<<(std::ostream&, const std::string&), 개행에는
    //     operator<<(std::ostream&, char) overload가 차례로 선택된다.
    // (3) 첫 인자는 반환 string을 const 참조로 빌리고, 둘째 삽입은 '\n' 값을 복사한다. 소유권 이동은 없다.
    // (4) 각 삽입은 같은 std::ostream&를 반환해 다음 수신자로 쓰며 마지막 반환은 버린다.
    // (5) 답 문자와 개행이 버퍼에 추가되고 입력 text는 그대로다. 임시 답은 전체 표현식 끝에 파괴된다.
    // (6) 답 길이에 선형이며 버퍼 할당, 출력 실패 상태와 설정 예외가 가능하다. 참조는 호출 중 유효하고
    //     컨테이너 관찰자를 무효화하지 않는다. 단일 실행 흐름에서만 cout을 쓴다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << longest_palindrome(text) << '\n';
    return 0;
}
