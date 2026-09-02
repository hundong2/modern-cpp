#include "ffi_api.h"  // C ABI 공개 선언과 정의의 signature가 일치하는지 compiler가 검사하게 한다.

#include <cstdint>     // 구현에서 std::int64_t 고정 폭 정수 타입을 사용한다.
#include <limits>      // signed overflow 전에 최솟값과 최댓값을 검사할 std::numeric_limits를 선언한다.
#include <mutex>       // opaque 객체 내부 값을 보호할 std::mutex와 std::lock_guard를 선언한다.
#include <new>         // std::bad_alloc과 nothrow가 아닌 일반 new의 실패 예외를 선언한다.

struct dd_counter final {                 // header에서는 숨긴 opaque C tag의 실제 C++ layout을 이 파일에서 완성한다.
    explicit dd_counter(std::int64_t initial_value) // 초기 counter 값을 받는 변환 방지 생성자다.
        : value{initial_value} {}          // value를 직접 초기화하고 mutex는 기본 생성한다.

    std::mutex mutex;                      // value의 모든 C ABI 접근을 직렬화하는 동기화 객체다.
    std::int64_t value;                    // lock 아래에서만 읽고 쓰는 고정 폭 signed counter다.
};                                        // opaque 구현 타입 정의를 끝낸다.

extern "C" dd_counter* dd_counter_create(std::int64_t initial_value) { // C linkage symbol로 native 객체를 만들고 pointer를 반환한다.
    try {                                  // C++ allocation exception이 C ABI를 넘지 않도록 경계를 연다.
        return new dd_counter{initial_value}; // heap에 객체를 생성하고 소유권을 caller에게 handle 형태로 넘긴다.
    } catch (const std::bad_alloc&) {       // 메모리 부족 예외만 구체적으로 잡는다.
        return nullptr;                    // C caller가 확인할 수 있는 null 실패 값으로 번역한다.
    } catch (...) {                        // 생성 중 발생할 수 있는 그 밖의 C++ 예외를 모두 ABI 안에서 막는다.
        return nullptr;                    // 단순 create 계약에서는 모든 실패를 null로 반환한다.
    }                                      // 예외 번역 구역을 닫는다.
}                                          // create 함수 정의를 끝낸다.

extern "C" void dd_counter_destroy(dd_counter* handle) { // create 소유권을 회수하며 null도 허용하는 C linkage 함수다.
    delete handle;                         // C++ delete는 null에 안전하고 같은 module의 allocator로 객체와 mutex를 파괴한다.
}                                          // destroy 뒤 caller의 기존 pointer는 dangling이므로 다시 쓰면 안 된다.

extern "C" int dd_counter_add(            // 상태 코드를 int로 반환하는 thread-safe C linkage 함수를 정의한다.
    dd_counter* handle,                    // 수정할 opaque 객체 pointer이며 null은 잘못된 입력이다.
    std::int64_t delta,                    // counter에 더할 고정 폭 signed 값이다; overflow 정책은 아래에서 제한한다.
    std::int64_t* out_value) {             // 성공 결과를 기록할 caller 소유 pointer이며 null이면 쓸 수 없다.
    if (handle == nullptr || out_value == nullptr) { // 두 필수 pointer를 역참조하기 전에 검증한다.
        return DD_STATUS_INVALID_ARGUMENT; // shared state에 접근하지 않고 구체적인 오류 코드를 반환한다.
    }                                      // 입력 검증 분기를 닫는다.
    try {                                  // mutex/runtime 예외를 C ABI 밖으로 내보내지 않는 방어 경계다.
        std::lock_guard<std::mutex> lock{handle->mutex}; // opaque 객체의 mutex를 scope 동안 획득한다.
        const auto maximum = std::numeric_limits<std::int64_t>::max(); // int64_t가 표현할 수 있는 최댓값을 compile-time 상수로 얻는다.
        const auto minimum = std::numeric_limits<std::int64_t>::min(); // int64_t가 표현할 수 있는 최솟값을 compile-time 상수로 얻는다.
        const bool positive_overflow = delta > 0 && handle->value > maximum - delta; // 실제 덧셈 전에 양수 방향 범위 초과를 검사한다.
        const bool negative_overflow = delta < 0 && handle->value < minimum - delta; // 실제 덧셈 전에 음수 방향 범위 초과를 검사한다.
        if (positive_overflow || negative_overflow) { // 둘 중 하나라도 true면 signed overflow/UB가 될 요청이다.
            return DD_STATUS_OUT_OF_RANGE;            // 값을 바꾸지 않고 C caller가 처리할 수 있는 오류 코드를 반환한다.
        }                                      // 안전 범위 검사 분기를 닫는다.
        handle->value += delta;             // 위 검사로 표현 가능성이 증명된 경우에만 signed 덧셈을 수행한다.
        *out_value = handle->value;         // lock 보유 중 일관된 새 값을 caller buffer에 복사한다.
        return DD_STATUS_OK;                // 성공 상태 0을 반환하고 lock 소멸자가 mutex를 해제한다.
    } catch (...) {                        // 어떠한 C++ 예외도 C 또는 다른 runtime frame으로 전파하지 않는다.
        return DD_STATUS_INTERNAL_ERROR;   // 내부 오류 코드로 변환하며 out_value 내용은 성공으로 간주하면 안 된다.
    }                                      // 예외 경계를 닫는다.
}                                          // add 함수 정의를 끝낸다.

extern "C" int dd_counter_get(dd_counter* handle, std::int64_t* out_value) { // 현재 snapshot을 반환하는 C linkage 함수다.
    if (handle == nullptr || out_value == nullptr) { // handle과 출력 저장소가 모두 유효한지 먼저 확인한다.
        return DD_STATUS_INVALID_ARGUMENT; // null 입력이면 역참조 없이 오류 코드를 반환한다.
    }                                      // 입력 검증 분기를 닫는다.
    try {                                  // C ABI 경계를 넘는 C++ 예외를 차단한다.
        std::lock_guard<std::mutex> lock{handle->mutex}; // add와 같은 mutex 규약으로 읽기를 동기화한다.
        *out_value = handle->value;         // 보호된 값을 caller 소유 메모리에 복사한다.
        return DD_STATUS_OK;                // snapshot 기록 성공을 반환한다.
    } catch (...) {                        // 구현/runtime에서 나온 예외를 모두 잡는다.
        return DD_STATUS_INTERNAL_ERROR;   // C가 처리할 수 있는 정수 오류 코드로 바꾼다.
    }                                      // 예외 경계를 닫는다.
}                                          // get 함수 정의를 끝낸다.
