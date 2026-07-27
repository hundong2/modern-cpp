#include <boost/container/flat_map.hpp>  // 연속 저장소 기반의 Boost 정렬 연관 컨테이너를 가져옵니다.

#include <iostream>  // Boost 예제 결과를 표준 출력으로 보여 줍니다.
#include <string>    // flat_map 키가 문자 데이터를 직접 소유하도록 합니다.

int main() {  // MODERN_CPP_ENABLE_BOOST=ON일 때만 만들어지는 실행 파일의 진입점입니다.
    boost::container::flat_map<std::string, int> priority_scores{  // 원소를 연속 메모리에 정렬 보관해 작은 읽기 중심 map에 유리할 수 있습니다.
        {"high", 3},                                                // 중괄호 초기화로 키와 값 pair를 만듭니다.
        {"low", 1},                                                 // 입력 순서와 무관하게 컨테이너가 키 정렬 상태를 유지합니다.
        {"normal", 2},                                              // 삽입 시 원소 이동 비용이 있으므로 쓰기 중심 큰 데이터에는 측정이 필요합니다.
    };  // Boost flat_map 초기화를 닫습니다.

    for (const auto& [name, score] : priority_scores) {  // pair 원소를 구조화된 바인딩으로 읽되 복사하지 않습니다.
        std::cout << name << '=' << score << '\n';       // 키 정렬 순서로 각 점수를 출력합니다.
    }  // 연속 원소 순회를 닫습니다.

    return 0;  // flat_map과 그 string 원소들이 역순으로 파괴된 뒤 성공 종료합니다.
}  // Boost.Container의 이 타입은 헤더 사용 외에 별도 바이너리 링크가 필요하지 않습니다.
