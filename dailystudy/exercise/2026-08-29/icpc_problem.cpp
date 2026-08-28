/*
문제 ID·제목: BOJ 9248 - Suffix Array
출처: Baekjoon Online Judge
출처 URL: https://www.acmicpc.net/problem/9248

문제 요약:
소문자 알파벳 문자열 S의 모든 접미사를 사전순으로 정렬한다. 첫 줄에는 각 접미사의 시작 위치를
1부터 세어 출력한다. 둘째 줄에는 첫 접미사 자리에 문자 x를 쓰고, 이후에는 바로 앞 접미사와
현재 접미사의 최장 공통 접두사(LCP) 길이를 순서대로 출력한다.

입력:
한 줄에 문자열 S 하나가 주어진다. 공백 없는 영문 소문자로만 이루어진다.

출력:
첫 줄에 1-indexed suffix array를 출력한다. 둘째 줄에는 x와 LCP 배열을 출력한다.

제약:
1 <= |S| <= 500,000. 모든 접미사 문자열을 실제로 만들어 비교하면 시간·메모리가 부족하다.

예제:
입력: banana
출력:
6 4 2 1 5 3
x 1 3 0 0 2
*/

#include <cstddef>   // std::size_t로 컨테이너 크기와 인덱스를 표현한다.
#include <iostream>  // 빠른 표준 입력과 suffix array/LCP 출력을 사용한다.
#include <string>    // 입력 문자와 sentinel을 연속 저장하는 std::string을 사용한다.
#include <vector>    // 순위·접미 위치·계수 배열을 동적 연속 메모리에 소유한다.

// 알고리즘 대표 문서: ../algorithm/suffix-array-doubling-kasai.md
// 길이 2^k 접두사의 동치류를 두 정수 쌍으로 확장하고 계수 정렬해 전체 O(n log n)에 만든다.
[[nodiscard]] std::vector<int> build_suffix_array(const std::string& text) {
    // string 복사 생성자는 const lvalue text의 모든 문자를 새 버퍼에 O(n) 복사해 augmented가 독립 소유한다.
    // 할당 실패 시 bad_alloc 가능하며 text는 바뀌지 않고 반환 참조·뷰는 만들지 않는다.
    std::string augmented{text};
    // push_back(char)는 '\0' 값 하나를 끝에 추가하고 void를 반환한다. 크기는 1 늘고 용량 부족 시 재할당되어 기존 포인터·참조가 무효화된다.
    // 여기서는 augmented의 내부 주소를 보관하지 않는다. sentinel은 소문자보다 작아 빈 접미사를 정렬 첫 원소로 만든다.
    augmented.push_back('\0');

    // size()는 인자 없이 size_type을 O(1)에 반환하고 문자열을 바꾸지 않는다. n<=500001이라 int로 명시 변환해 인덱스 계산을 단순화한다.
    const int n{static_cast<int>(augmented.size())};
    // vector<int>(count) 생성자는 n개 int를 0으로 값 초기화해 각각 연속 저장한다. O(n) 시간·공간과 한 번의 동적 할당이 가능하다.
    std::vector<int> suffix_order(static_cast<std::size_t>(n));
    // group[start]는 현재 길이 접두사의 0-based 동치류를 저장한다.
    std::vector<int> group(static_cast<std::size_t>(n));
    // shifted는 이미 정렬된 위치를 half만큼 왼쪽 순환 이동한 임시 순서다.
    std::vector<int> shifted(static_cast<std::size_t>(n));
    // next_group은 이번 단계의 두 그룹 쌍으로 계산한 새 동치류를 저장한다.
    std::vector<int> next_group(static_cast<std::size_t>(n));
    // 문자 값 0..255를 세므로 정확히 256칸을 0으로 초기화한다.
    std::vector<int> character_count(256U);

    // int index는 0부터 n-1까지 증가하고, < 조건이 string::operator[]의 전제조건을 보장한다.
    for (int index{0}; index < n; ++index) {
        // unsigned char 변환은 char의 부호 여부와 무관하게 0..255 배열 인덱스를 만든다.
        const auto symbol{static_cast<unsigned char>(augmented[static_cast<std::size_t>(index)])};
        ++character_count[static_cast<std::size_t>(symbol)];  // 전위 ++로 이 문자 빈도를 1 증가시킨다.
    }
    // 빈도를 누적 끝 위치로 바꾼다. symbol 0은 앞 항이 없어 1부터 시작한다.
    for (int symbol{1}; symbol < 256; ++symbol) {
        // +=는 오른쪽까지의 빈도를 왼쪽 피연산자에 저장해 누적합 불변식을 만든다.
        character_count[static_cast<std::size_t>(symbol)] += character_count[static_cast<std::size_t>(symbol - 1)];
    }
    // 뒤에서 앞으로 배치해야 같은 문자끼리 원래 index 순서를 유지하는 안정 계수 정렬이 된다.
    for (int index{n - 1}; index >= 0; --index) {
        // auto는 static_cast 결과인 unsigned char로 추론되고 symbol은 값 복사된 지역 상수다.
        const auto symbol{static_cast<unsigned char>(augmented[static_cast<std::size_t>(index)])};
        // 전위 --는 누적 끝 위치를 실제 0-based 위치로 바꾸며 안정 정렬을 위해 원문을 뒤에서 훑는다.
        suffix_order[static_cast<std::size_t>(--character_count[static_cast<std::size_t>(symbol)])] = index;
    }

    int group_count{1};  // sentinel 그룹 하나에서 시작하며 int 중괄호 초기화는 축소 변환을 막는다.
    group[static_cast<std::size_t>(suffix_order[0])] = 0;  // 정렬 첫 sentinel 위치의 그룹은 0이다.
    // 정렬된 인접 문자를 비교해 문자 값이 바뀔 때만 새 그룹을 연다.
    for (int order{1}; order < n; ++order) {
        // current/previous는 suffix_order 원소를 값 복사한 int라 vector 재배치와 무관하게 유효하다.
        const int current{suffix_order[static_cast<std::size_t>(order)]};
        const int previous{suffix_order[static_cast<std::size_t>(order - 1)]};
        // !=는 두 char 값을 비교해 bool을 만들며 문자열을 변경하지 않는다.
        if (augmented[static_cast<std::size_t>(current)] != augmented[static_cast<std::size_t>(previous)]) {
            ++group_count;  // 다른 문자라면 다음 연속 그룹 번호를 만든다.
        }
        group[static_cast<std::size_t>(current)] = group_count - 1;  // 현재 시작 위치에 0-based 그룹을 저장한다.
    }

    // 불변식: half 시작 시 group[i]는 i에서 시작하는 길이 half 접두사의 사전순 동치류다.
    for (int half{1}; half < n; half *= 2) {
        // 현재 SA의 두 번째 절반 순서를 첫 절반 시작 위치 순서로 바꾸는 선형 순회다.
        for (int order{0}; order < n; ++order) {
            // - 연산은 두 번째 절반 시작을 이미 정렬된 접미사 위치에 맞춰 왼쪽으로 옮긴다.
            int start{suffix_order[static_cast<std::size_t>(order)] - half};
            // 음수는 sentinel 포함 길이 n의 순환 인덱스 뒤쪽으로 보정한다.
            if (start < 0) {
                start += n;  // sentinel을 포함한 순환 이동으로 두 번째 절반 정렬 결과를 재사용한다.
            }
            shifted[static_cast<std::size_t>(order)] = start;  // 같은 order 칸에 값 복사해 O(1) 저장한다.
        }

        // 새 vector 생성자는 group_count개의 0을 소유한다. 모든 동치류는 0..group_count-1이라 범위가 정확하다.
        std::vector<int> group_size(static_cast<std::size_t>(group_count));
        // 각 shifted 시작 위치의 첫 그룹 빈도를 센다.
        for (int index{0}; index < n; ++index) {
            // 중첩 operator[]는 먼저 start의 그룹을 읽고 그 그룹 빈도를 전위 ++로 증가시킨다.
            ++group_size[static_cast<std::size_t>(group[static_cast<std::size_t>(shifted[static_cast<std::size_t>(index)])])];
        }
        // 그룹 빈도를 각 그룹의 배타적 끝 위치로 바꾸는 누적합이다.
        for (int index{1}; index < group_count; ++index) {
            group_size[static_cast<std::size_t>(index)] += group_size[static_cast<std::size_t>(index - 1)];
        }
        // shifted를 뒤에서 배치해 두 번째 key의 기존 순서를 유지하는 안정 정렬을 만든다.
        for (int index{n - 1}; index >= 0; --index) {
            // start와 key는 vector 원소의 값 복사본이며 이 반복 안에서 바뀌지 않는다.
            const int start{shifted[static_cast<std::size_t>(index)]};
            const int key{group[static_cast<std::size_t>(start)]};
            // key 그룹의 끝 위치를 먼저 1 줄이고 그 칸에 시작 위치를 쓴다.
            suffix_order[static_cast<std::size_t>(--group_size[static_cast<std::size_t>(key)])] = start;
        }

        group_count = 1;  // 대입 연산자로 이번 단계 그룹 수를 첫 그룹 하나로 다시 설정한다.
        next_group[static_cast<std::size_t>(suffix_order[0])] = 0;  // 정렬 첫 쌍의 새 그룹은 0이다.
        // 정렬된 인접 두 그룹 쌍이 달라지는 경계마다 새 번호를 부여한다.
        for (int order{1}; order < n; ++order) {
            // 첫 key를 비교할 두 시작 위치를 정렬 배열에서 읽는다.
            const int current{suffix_order[static_cast<std::size_t>(order)]};
            const int previous{suffix_order[static_cast<std::size_t>(order - 1)]};
            // % n은 길이 half 뒤 위치를 sentinel 포함 순환 인덱스로 정규화한다.
            const int current_second{(current + half) % n};
            const int previous_second{(previous + half) % n};
            // ||는 첫 그룹이 다르면 두 번째 비교를 생략하는 단락 평가 조건 연산자다.
            if (group[static_cast<std::size_t>(current)] != group[static_cast<std::size_t>(previous)] ||
                group[static_cast<std::size_t>(current_second)] != group[static_cast<std::size_t>(previous_second)]) {
                ++group_count;  // 두 key 중 하나가 다를 때만 새 동치류를 연다.
            }
            next_group[static_cast<std::size_t>(current)] = group_count - 1;  // 시작 위치별 새 0-based 그룹을 저장한다.
        }
        // vector::swap(other)는 두 버퍼 소유권을 O(1)에 교환하고 void를 반환한다. 크기는 유지되며 원소 참조는 각 버퍼를 따라간다.
        // 인자는 non-const lvalue next_group이고 복사·할당이 없다. allocator가 같은 기본 vector라 예외를 던지지 않는다.
        group.swap(next_group);
        if (group_count == n) {  // == 비교가 true면 n개 시작 위치가 모두 서로 다른 그룹이다.
            break;  // 모든 접미사가 서로 다른 그룹이면 이후 배수 단계는 순서를 바꾸지 않는다.
        }
    }

    // sentinel의 빈 접미사인 suffix_order[0]을 빼고 원문 접미사 n-1개만 소유하는 결과를 만든다.
    std::vector<int> result(static_cast<std::size_t>(n - 1));
    for (int order{1}; order < n; ++order) {  // order 0 sentinel을 건너뛰고 n-1개를 복사한다.
        result[static_cast<std::size_t>(order - 1)] = suffix_order[static_cast<std::size_t>(order)];
    }
    return result;  // vector prvalue 반환은 결과 객체 직접 초기화 또는 버퍼 이동으로 O(1) 소유권 이전이 가능하다.
}

// Kasai 알고리즘 대표 문서: ../algorithm/suffix-array-doubling-kasai.md
// h가 다음 시작 위치에서 최소 h-1이라는 불변식으로 모든 문자 비교를 합계 O(n)으로 제한한다.
[[nodiscard]] std::vector<int> build_lcp(const std::string& text, const std::vector<int>& suffix_order) {
    // text.size()는 문자열을 바꾸지 않는 O(1) 관찰이고 문제 제약 안에서 int로 안전하게 변환된다.
    const int n{static_cast<int>(text.size())};
    // 두 vector 생성자는 n개 int를 0으로 값 초기화하고 각 버퍼를 독점 소유한다.
    std::vector<int> inverse_rank(static_cast<std::size_t>(n));
    std::vector<int> lcp(static_cast<std::size_t>(n));
    // SA의 역함수: 시작 위치가 사전순 몇 번째인지 O(n)에 기록한다.
    for (int order{0}; order < n; ++order) {
        inverse_rank[static_cast<std::size_t>(suffix_order[static_cast<std::size_t>(order)])] = order;
    }

    int height{0};  // 이전 반복에서 재사용할 일치 길이를 0으로 시작한다.
    // SA 순서가 아니라 원문 시작 위치 순서로 훑어 h-1 하한을 사용할 수 있게 한다.
    for (int start{0}; start < n; ++start) {
        // 역순위 원소를 값 복사하므로 이후 lcp 쓰기가 order를 바꾸지 않는다.
        const int order{inverse_rank[static_cast<std::size_t>(start)]};
        // 사전순 첫 원소에는 이전 이웃이 없으므로 0으로 값 초기화된 lcp를 유지한다.
        if (order == 0) {
            continue;  // 사전순 첫 접미사는 바로 앞 접미사가 없어 LCP가 0이다.
        }
        const int previous{suffix_order[static_cast<std::size_t>(order - 1)]};  // 직전 접미사 시작 위치를 값 복사한다.
        // &&는 왼쪽부터 단락 평가해 두 인덱스 경계를 확인한 뒤에만 문자를 읽는다.
        while (start + height < n && previous + height < n &&
               text[static_cast<std::size_t>(start + height)] == text[static_cast<std::size_t>(previous + height)]) {
            ++height;  // 같은 문자를 하나 더 확인했으므로 공통 접두사 길이를 1 늘린다.
        }
        lcp[static_cast<std::size_t>(order)] = height;  // 현재 SA 인접 쌍의 최종 LCP를 저장한다.
        // 양수일 때만 줄여 음수 길이를 만들지 않는다.
        if (height > 0) {
            --height;  // 시작점을 한 칸 옮기면 이미 일치한 접두사에서 최대 한 글자만 사라진다.
        }
    }
    return lcp;  // 소유 vector를 값 반환하며 복사 생략 또는 이동으로 호출자에게 버퍼를 넘긴다.
}

int main() {
    // sync_with_stdio(false)는 bool false 값을 받아 C stdio 동기화를 끄고 이전 상태 bool을 반환하지만 여기서는 버린다.
    // 이후 cin/cout 혼용 성능은 좋아질 수 있으나 C stdio와 순서를 섞지 않는다. 스트림 객체 수명은 프로그램 전체다.
    std::ios::sync_with_stdio(false);
    // tie(nullptr)는 cin의 연결 출력 스트림 포인터를 null로 바꾸고 이전 포인터를 반환하지만 무시한다. 입력 전 자동 cout flush가 사라진다.
    std::cin.tie(nullptr);

    std::string text{};  // 기본 생성자는 빈 소유 문자열을 만들고 동적 할당 여부는 구현에 따라 다르다.
    // operator>>(istream&, string&)는 cin에서 공백 전 문자를 text 소유 버퍼에 저장하고 cin&를 반환한다. 실패 시 text 사용을 피하려고 조건으로 검사한다.
    if (!(std::cin >> text)) {
        return 0;  // 입력 실패를 정상 무출력 종료로 처리한다.
    }

    // 함수 호출은 text const lvalue를 비소유 참조로 빌리고 새 vector를 값 반환한다. suffix_order가 결과 버퍼를 소유한다.
    const std::vector<int> suffix_order{build_suffix_array(text)};
    // 두 인자는 const lvalue 참조로 빌려 읽기만 하고 반환 vector의 버퍼는 lcp가 독점 소유한다.
    const std::vector<int> lcp{build_lcp(text, suffix_order)};

    // size() 반환값은 O(1)이고 vector를 바꾸지 않는다. 각 operator[]는 범위 검사 없이 O(1) 참조를 돌려주므로 loop 조건이 전제조건을 보장한다.
    for (std::size_t order{0}; order < suffix_order.size(); ++order) {
        // cout operator<<는 1-based 시작 위치와 구분 문자를 버퍼에 기록하고 같은 ostream&를 반환한다. 배열은 바뀌지 않는다.
        std::cout << suffix_order[order] + 1 << (order + 1 == suffix_order.size() ? '\n' : ' ');
    }
    // 문자 'x'를 cout 버퍼에 쓰고 반환 ostream&는 사용하지 않는다. 스트림 상태 외 프로그램 객체는 바뀌지 않는다.
    std::cout << 'x';
    // 첫 LCP는 x로 표시했으므로 인덱스 1부터 끝 전까지 순회한다.
    for (std::size_t order{1}; order < lcp.size(); ++order) {
        // 두 operator<<는 공백과 int LCP를 순서대로 쓰고 같은 cout 참조를 연쇄한다.
        std::cout << ' ' << lcp[order];
    }
    // 마지막 개행을 쓰며 출력 실패 시 cout 상태 비트가 설정될 수 있다.
    std::cout << '\n';
    return 0;  // int 0은 운영체제에 정상 종료 상태를 전달한다.
}
