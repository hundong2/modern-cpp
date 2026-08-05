#include <iostream>  // 결과 확인용 표준 출력을 제공한다.
#include <memory>    // std::unique_ptr와 std::make_unique를 제공한다.
#include <string>    // std::string을 제공한다.
#include <vector>    // 연속 컨테이너 std::vector를 제공한다.

struct Job {                                         // 서버 작업 하나를 표현하는 단순 구조체다.
    int id;                                          // 작업을 구별하는 값 타입 식별자다.
    std::string payload;                             // 작업이 직접 소유하는 문자열 데이터다.
};                                                   // 구조체 정의를 끝낸다.

int main() {                                         // 실습 프로그램의 시작점이다.
    std::vector<std::unique_ptr<Job>> queue;         // 각 Job을 단독 소유하는 큐 모형을 만든다.
    auto job = std::make_unique<Job>(Job{1, "login"}); // Job을 동적 생성하고 job이 소유한다.

    // TODO 1: job의 소유권을 queue 뒤로 이동하라. 복사는 허용되지 않는다.
    queue.push_back(std::move(job));                 // std::move가 rvalue로 바꿔 vector의 이동 삽입을 선택한다.

    // TODO 2: 이동 후 job이 비었는지 확인하고, queue의 payload를 출력하라.
    std::cout << std::boolalpha << (job == nullptr) << '\n'; // 이동 후 unique_ptr가 빈 상태인지 출력한다.
    std::cout << queue.at(0)->payload << '\n';       // 범위를 검사해 첫 Job의 문자열을 읽는다.

    // 예상 출력: true 다음 줄에 login. queue가 파괴되며 Job도 자동 해제된다.
    return 0;                                        // 정상 종료를 운영체제에 알린다.
}                                                    // vector→unique_ptr→Job 순으로 소유 자원이 정리된다.
