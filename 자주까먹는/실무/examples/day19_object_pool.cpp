#include <iostream>  // 대여/반납 결과를 출력한다.
#include <memory>    // unique_ptr와 shared pool state를 제공한다.
#include <mutex>     // free list를 여러 thread에서 보호한다.
#include <string>    // pooled 객체의 payload를 소유한다.
#include <vector>    // free object 저장소를 제공한다.

struct Session {                                    // 재사용할 객체 타입이다.
    std::string name;                               // 대여자가 설정하는 상태다.
    void reset() { name.clear(); }                  // 반납 전 다음 사용자를 위해 상태를 지운다.
};                                                  // 실제 객체는 더 강한 불변식 검사가 필요하다.

class SessionPool {                                 // 안전한 수명 모형의 고정 상태 pool이다.
private:                                            // handle보다 오래 살아야 할 공유 state를 먼저 정의한다.
    struct State {                                  // custom deleter가 공동 소유할 내부 상태다.
        std::mutex mutex;                           // free vector 접근을 보호한다.
        std::vector<std::unique_ptr<Session>> free; // 현재 대여되지 않은 객체들을 단독 소유한다.
    };                                              // State는 마지막 handle까지 살아 있을 수 있다.

public:                                             // pool 생성과 acquire API를 공개한다.
    SessionPool() : state_(std::make_shared<State>()) {} // shared state를 먼저 생성한다.

    std::shared_ptr<Session> acquire() {            // 대여 handle이 반납 함수를 소유하게 한다.
        std::unique_ptr<Session> owned;             // 잠금 아래 free object 소유권을 받을 지역 변수다.
        {                                           // free list 임계 구역을 시작한다.
            std::lock_guard<std::mutex> lock(state_->mutex); // 다른 acquire/반납과 직렬화한다.
            if (!state_->free.empty()) {            // 재사용 가능한 객체가 있는지 본다.
                owned = std::move(state_->free.back()); // 마지막 unique_ptr를 지역으로 이동한다.
                state_->free.pop_back();            // 비어 있는 vector 원소를 제거한다.
            }                                       // 없으면 잠금 밖에서 새 객체를 만들 예정이다.
        }                                           // allocation 전에 mutex를 푼다.
        if (!owned) owned = std::make_unique<Session>(); // free object가 없을 때 새로 할당한다.
        Session* raw = owned.release();             // custom deleter가 책임질 raw pointer로 넘긴다.
        std::shared_ptr<State> state = state_;      // pool 객체보다 handle이 오래 살 수 있게 state를 소유한다.
        return std::shared_ptr<Session>(raw, [state](Session* session) { // 마지막 대여 참조 때 반납한다.
            session->reset();                       // 이전 사용자의 상태를 먼저 지운다.
            std::lock_guard<std::mutex> lock(state->mutex); // free list 갱신을 보호한다.
            state->free.emplace_back(session);      // raw pointer를 unique ownership으로 되돌린다.
        });                                         // handle과 deleter가 완성된다.
    }                                               // 반환 shared_ptr가 Session 대여권을 표현한다.

private:                                            // state 직접 교체를 막는다.
    std::shared_ptr<State> state_;                  // 모든 대여 handle과 pool이 공동 소유한다.
};                                                  // SessionPool 객체 자체는 먼저 파괴될 수도 있다.

int main() {                                        // pool보다 handle이 오래 사는 경우를 시험한다.
    std::shared_ptr<Session> lease;                 // pool 밖에서도 존재할 대여 handle이다.
    {                                               // pool의 짧은 수명 블록이다.
        SessionPool pool;                           // shared state를 만든다.
        lease = pool.acquire();                     // Session과 state를 소유하는 handle을 얻는다.
        lease->name = "Alice";                     // 대여 기간 중 객체 상태를 사용한다.
    }                                               // pool 객체는 사라져도 state는 lease가 보존한다.
    std::cout << lease->name << '\n';              // dangling 없이 Alice를 읽는다.
    lease.reset();                                  // custom deleter가 reset 후 state에 반납한다.
    return 0;                                       // 마지막 state와 free Session이 자동 해제된다.
}                                                   // main을 끝낸다.
