#include <future>   // DB 완료 결과의 shared state를 표현한다.
#include <iostream> // 조회 결과를 출력한다.
#include <string>   // username을 소유한다.

struct UserRow {                                    // DB row를 logic 계층의 값 객체로 옮긴다.
    int id;                                         // 정수 primary key 모형이다.
    std::string name;                               // driver buffer와 독립된 소유 문자열이다.
};                                                  // SDK 객체가 사라져도 UserRow는 유효하다.

std::future<UserRow> submit_db_job(int user_id) {   // 실제로는 bounded DB pool에 작업을 제출한다.
    return std::async(std::launch::async, [user_id] { // 교육용으로 별도 비동기 실행을 강제한다.
        const std::string sql = "SELECT id,name FROM users WHERE id=?"; // 값 연결 대신 placeholder를 쓴다.
        (void)sql;                                  // 모형에서 실행하지 않는 문자열 경고를 막는다.
        return UserRow{user_id, "Alice"};           // driver row를 소유 값으로 복사해 반환한다.
    });                                             // future가 완료/예외 shared state를 소유한다.
}                                                   // 호출자는 get을 어느 thread에서 할지 결정해야 한다.

int main() {                                        // DB job 경계의 결과 전달을 시험한다.
    std::future<UserRow> pending = submit_db_job(7); // network loop 밖으로 보낼 작업을 만든다.
    const UserRow row = pending.get();              // 예제 main은 기다리지만 event loop에서는 하면 안 된다.
    std::cout << row.id << ':' << row.name << '\n'; // SDK 수명과 독립된 값을 출력한다.
    return row.id == 7 ? 0 : 1;                     // 자체 검사 결과를 반환한다.
}                                                   // row와 future를 파괴한다.
