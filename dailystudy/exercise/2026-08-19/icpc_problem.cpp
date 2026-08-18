/*
문제 ID/제목: BOJ 1786 - 찾기
출처: https://www.acmicpc.net/problem/1786 (Baekjoon Online Judge)

문제 요약:
한 줄짜리 긴 텍스트 T와 찾을 패턴 P가 주어진다. T 안에서 P와 완전히 같은 부분 문자열이
시작하는 모든 위치를 찾아야 한다. 일치 구간은 서로 겹쳐도 각각 세며 위치는 1부터 센다.

입력:
첫 줄에 텍스트 T, 둘째 줄에 패턴 P가 주어진다. 두 문자열에는 영문자와 공백이 들어갈 수 있으므로
공백에서 끊는 추출 연산자가 아니라 줄 전체를 읽어야 한다.

출력:
첫 줄에 일치 횟수를 출력한다. 둘째 줄에는 각 일치 시작 위치를 오름차순으로 공백 구분해 출력한다.

제약:
T와 P의 길이는 각각 최대 1,000,000이다. 모든 시작점에서 패턴을 처음부터 비교하는 O(|T||P|)
방식은 너무 느리다. KMP 접두 함수로 이미 맞은 접두사 정보를 재사용해 O(|T|+|P|) 시간,
O(|T|+|P|) 출력 포함 공간에 해결한다. 작업용 접두 함수 공간만 보면 O(|P|)이다.

설명용 예제:
입력
ABC ABCDAB ABCDABCDABDE
ABCDABD

출력
1
16
*/

// <iostream>은 줄 입력에 연결할 std::cin, 정답을 쓸 std::cout과 스트림 상태를 선언한다.
#include <iostream>
// <string>은 공백을 포함한 텍스트를 소유하는 std::string과 std::getline을 선언한다.
#include <string>
// <vector>는 접두 함수와 발견 위치를 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// 알고리즘 문서: ../algorithm/knuth-morris-pratt.md
// prefix[i]는 pattern[0..i]의 접두사이면서 접미사인 가장 긴 진부분 문자열의 길이다.
// 반환형 vector<int>가 표를 소유하고, const string& 매개변수는 호출자 패턴을 복사 없이 빌린다.
[[nodiscard]] std::vector<int> build_prefix(const std::string& pattern) {
    // string::size()는 인자 없이 문자 수를 size_type으로 O(1)에 반환하고 pattern을 바꾸지 않는다.
    // vector<int>(count, value)는 size()개 int를 0으로 복사 초기화해 소유하며 O(|P|) 시간·공간과
    // 동적 할당이 필요하다. 실패하면 bad_alloc이 발생하고 pattern의 값·수명은 유지된다.
    std::vector<int> prefix(pattern.size(), 0);

    // matched는 pattern[0..index-1]의 경계 길이이며 반복 시작마다 0<=matched<=index를 유지한다.
    int matched{};
    for (int index{1}; index < static_cast<int>(pattern.size()); ++index) {
        // string::operator[](size_type)는 pattern 수신 객체와 index/matched 인덱스 값을 받아 문자 참조를
        // O(1)에 반환하며 문자열은 바꾸지 않는다. 두 인덱스가 [0,size)라는 전제는 반복 범위와 matched
        // 불변식이 보장한다. 범위 검사는 없고 위반 시 미정의 동작이며, 재할당이 없어 참조는 유지된다.
        // 불일치하면 prefix[matched-1]로 더 짧은 경계를 따라간다. matched는 매 단계 감소하므로
        // 전체 전처리에서 되돌아가는 횟수를 포함해 O(|P|)이다.
        while (matched > 0 && pattern[index] != pattern[matched]) {
            // vector::operator[](size_type)는 범위 검사 없이 const int&를 O(1)에 반환한다.
            // matched>0과 prefix 크기 |P|가 인덱스 전제조건을 보장하며 컨테이너는 바뀌지 않는다.
            matched = prefix[matched - 1];
        }
        if (pattern[index] == pattern[matched]) {
            ++matched; // 새 문자도 같으므로 가장 긴 경계 길이를 한 칸 늘린다.
        }
        prefix[index] = matched; // 현재 위치의 정답을 O(1) 대입한다.
    }
    return prefix; // vector 소유권은 이동 또는 NRVO로 호출자 결과에 전달된다.
}

// 알고리즘 문서: ../algorithm/knuth-morris-pratt.md
// 두 const string& 매개변수는 입력을 빌리고 반환 vector<int>가 1-based 일치 위치를 소유한다.
[[nodiscard]] std::vector<int> find_matches(const std::string& text, const std::string& pattern) {
    std::vector<int> positions{}; // 기본 생성자는 크기와 용량이 0인 빈 vector를 만든다.
    // pattern의 정확한 타입은 const string&다. empty()는 인자 없이 size()==0 결과 bool을 O(1)에
    // 반환하고 수신 문자열의 값·용량·소유권을 바꾸지 않는다. 반환값을 if 분기에 사용하며 예외는 없다.
    if (pattern.empty()) {
        // 공식 입력은 빈 패턴이 아니지만 함수 계약을 완전하게 하기 위해 빈 결과를 반환한다.
        return positions;
    }

    // pattern lvalue가 const 참조에 바인딩되고 반환 vector prvalue가 prefix를 직접 초기화한다.
    const std::vector<int> prefix{build_prefix(pattern)};
    // reserve(count)는 positions의 크기 0을 유지하면서 용량을 최대 가능한 일치 수만큼 확보한다.
    // count는 text.size() size_type prvalue이고 반환형은 void라 버릴 값이 없다. 재할당 시 기존 관찰자가
    // 무효화되지만 지금은 원소가 없고 관찰자도 없다. O(|T|) 할당 가능, 실패 시 bad_alloc이 난다.
    positions.reserve(text.size());

    int matched{}; // 현재 text 접미사와 같은 pattern 접두사의 길이다.
    for (int index{}; index < static_cast<int>(text.size()); ++index) {
        // text/pattern의 operator[]는 각 const string 수신 객체에서 index/matched 위치의 const char&를
        // O(1)에 반환한다. 두 인덱스는 루프와 KMP 불변식으로 범위 안이고 문자열·관찰자는 바뀌지 않는다.
        // 불일치 전 matched 글자는 이미 같다는 불변식을 유지하면서 가능한 다음 경계로 이동한다.
        while (matched > 0 && text[index] != pattern[matched]) {
            matched = prefix[matched - 1];
        }
        if (text[index] == pattern[matched]) {
            ++matched;
        }

        if (matched == static_cast<int>(pattern.size())) {
            // 현재 0-based 끝 index에서 길이 |P|를 빼고 1-based로 바꾸면 index-|P|+2다.
            const int one_based_start{index - matched + 2};
            // vector<int>::push_back(const int&)는 one_based_start lvalue 값을 끝에 복사하고 void를 반환한다.
            // 성공 뒤 size가 1 증가한다. reserve가 충분해 이 코드에서는 재할당되지 않으므로 기존 원소 참조는
            // 유지된다. 일반 계약은 상각 O(1)이고 할당 실패 시 강한 예외 보장을 제공한다.
            positions.push_back(one_based_start);
            // 겹치는 일치를 놓치지 않도록 완전 일치 패턴의 가장 긴 경계에서 다음 비교를 계속한다.
            matched = prefix[matched - 1];
        }
    }
    return positions;
}

int main() {
    // sync_with_stdio(false)는 bool 인자 하나로 C/C++ 표준 스트림 동기화를 끄고 이전 bool을 반환하지만
    // 여기서는 버린다. 호출 뒤 C stdio와 순서를 섞지 않는다. 전역 스트림 설정만 바뀌고 문자열은 없다.
    std::ios::sync_with_stdio(false);
    // cin은 std::istream 수신 객체다. tie(nullptr)는 ostream* null 인자를 받아 이전 연결 포인터를
    // 반환하지만 버리고, 호출 뒤 입력 전 cout 자동 flush를 해제한다. 소유권 이전이나 할당은 없다.
    std::cin.tie(nullptr);

    std::string text{};    // 기본 생성자는 할당을 요구하지 않는 빈 소유 문자열을 만든다.
    std::string pattern{};
    // std::getline<char>(istream&, string&)은 cin lvalue와 text lvalue 참조 두 인자를 받는 함수 템플릿이다.
    // 구분자 기본값 '\n'까지 문자를 text에 저장하고 istream&를 반환한다. 반환 참조를 bool 문맥에서 검사한다.
    // 성공 뒤 입력 위치와 text 크기/내용이 바뀌며 재할당 시 이전 text 관찰자는 무효화된다. I/O/할당 실패는
    // 스트림 상태 또는 예외로 표현되고 시간·추가 공간은 읽은 줄 길이에 선형이다.
    if (!std::getline(std::cin, text)) {
        return 0;
    }
    // 같은 getline 오버로드의 두 번째 string& 인자만 pattern으로 달라진다. 반환 참조는 검사하고,
    // 성공하면 공백을 포함한 둘째 줄 전체를 pattern이 소유한다. text는 바뀌지 않는다.
    if (!std::getline(std::cin, pattern)) {
        return 0;
    }

    // text와 pattern lvalue 두 개를 const 참조로 빌려 주고 반환 vector prvalue로 positions를 직접 초기화한다.
    const std::vector<int> positions{find_matches(text, pattern)};
    // vector::size()는 인자 없이 원소 수를 size_type 값으로 O(1)에 반환하고 vector를 바꾸지 않는다.
    // operator<<가 그 값을 출력하며 반환 ostream&는 개행 삽입에만 쓰고 최종적으로 저장하지 않는다.
    std::cout << positions.size() << '\n';
    // 범위 for는 positions의 begin/end 반복자로 모든 int를 const 참조로 읽는다. 순회 중 vector를
    // 바꾸지 않아 반복자와 reference가 유효하고, 빈 vector면 본문을 실행하지 않는다.
    for (const int position : positions) {
        std::cout << position << ' '; // 두 삽입은 같은 ostream&를 반환하며 출력 상태만 바꾼다.
    }
    std::cout << '\n';

    // 문자 접근과 접두 함수 갱신은 load/store/비교/조건 분기를 만들 수 있지만 실제 명령은 CPU, ABI,
    // 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.
    return 0;
}
