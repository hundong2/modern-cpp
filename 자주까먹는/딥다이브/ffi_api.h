#ifndef DD_FFI_API_H                       /* 이 헤더를 한 translation unit에서 중복 포함하지 않게 하는 guard를 시작한다. */
#define DD_FFI_API_H                       /* include guard 이름을 정의해 아래 선언을 한 번만 보이게 한다. */

#include <stdint.h>                        /* C와 C++ 모두에서 폭이 명시된 int64_t를 선언한다. */

#ifdef __cplusplus                         /* 현재 파일을 C++ compiler가 처리하는지 확인한다. */
extern "C" {                              /* 아래 함수 이름/타입 선언에 C language linkage를 적용한다. */
#endif                                     /* C compiler에는 이해할 수 없는 extern "C" 문법을 노출하지 않는다. */

typedef struct dd_counter dd_counter;      /* 내부 layout을 숨기는 불완전 C struct 타입의 별칭을 선언한다. */

enum dd_status {                           /* 함수 상태 코드를 C와 다른 FFI caller가 공유할 상수 집합으로 선언한다. */
    DD_STATUS_OK = 0,                      /* 호출 성공을 값 0으로 고정한다. */
    DD_STATUS_INVALID_ARGUMENT = 1,        /* null pointer 같은 잘못된 caller 입력을 나타낸다. */
    DD_STATUS_OUT_OF_MEMORY = 2,           /* native 객체 할당 실패를 나타낸다. */
    DD_STATUS_INTERNAL_ERROR = 3,          /* 알려지지 않은 C++ 예외를 ABI 밖으로 내보내지 않고 번역한다. */
    DD_STATUS_OUT_OF_RANGE = 4             /* signed 덧셈이 표현 범위를 벗어나는 요청을 안전하게 거절한다. */
};                                        /* enum 선언을 세미콜론으로 끝낸다. */

dd_counter* dd_counter_create(int64_t initial_value); /* 초기값을 받아 opaque handle을 반환하며 실패하면 null을 반환한다. */
void dd_counter_destroy(dd_counter* handle);          /* create가 반환한 handle 또는 null을 받아 자원을 해제하며 반환값은 없다. */
int dd_counter_add(dd_counter* handle, int64_t delta, int64_t* out_value); /* delta를 더하고 결과를 out pointer에 쓰며 dd_status를 반환한다. */
int dd_counter_get(dd_counter* handle, int64_t* out_value); /* 현재 snapshot을 out pointer에 쓰고 dd_status를 반환한다. */

#ifdef __cplusplus                         /* C++ compiler일 때만 linkage specification의 닫는 중괄호가 필요하다. */
}                                          /* extern "C" 선언 묶음을 닫는다. */
#endif                                     /* C/C++ 조건부 구역을 끝낸다. */

#endif                                     /* DD_FFI_API_H include guard를 끝낸다. */
