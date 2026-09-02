#include "ffi_api.h"  /* C compiler로 C++ library의 C ABI header를 실제 소비해 문법 호환성을 검증한다. */

#include <inttypes.h>  /* int64_t를 printf할 이식 가능한 PRId64 format macro를 선언한다. */
#include <stdio.h>     /* printf와 fputs 표준 C 출력 함수를 선언한다. */

int main(void) {                                      /* C 프로그램 진입점은 인자 없음(void)을 명시하고 int 상태를 반환한다. */
    dd_counter* counter = dd_counter_create(10);      /* 초기값 10의 opaque native 객체를 만들고 소유 handle을 받는다. */
    if (counter == NULL) {                            /* create 실패의 null pointer를 역참조 전에 검사한다. */
        fputs("dd_counter_create failed\n", stderr); /* 표준 오류 stream에 고정 문자열을 기록한다. */
        return 1;                                     /* 생성 실패 exit code를 운영체제에 반환한다. */
    }                                                 /* 생성 실패 분기를 닫는다. */

    int64_t value = 0;                                /* native 함수가 결과를 기록할 caller 소유 고정 폭 변수를 만든다. */
    const int status = dd_counter_add(counter, 32, &value); /* handle에 32를 더하고 value 주소로 결과를 받는다. */
    if (status != DD_STATUS_OK) {                     /* 결과 buffer를 사용하기 전에 상태 코드 0인지 확인한다. */
        dd_counter_destroy(counter);                  /* 오류 경로에서도 소유 native 자원을 같은 library로 반환한다. */
        fputs("dd_counter_add failed\n", stderr);    /* 호출 실패 메시지를 표준 오류 stream에 쓴다. */
        return 2;                                     /* add 실패를 구분하는 exit code를 반환한다. */
    }                                                 /* add 실패 분기를 닫는다. */

    int64_t snapshot = 0;                             /* 별도 get API가 결과를 기록할 caller 소유 저장소를 만든다. */
    const int get_status = dd_counter_get(counter, &snapshot); /* 같은 handle의 thread-safe snapshot API를 호출한다. */
    if (get_status != DD_STATUS_OK || snapshot != value) { /* status와 add/get의 값 일치 불변식을 함께 확인한다. */
        dd_counter_destroy(counter);                  /* snapshot 실패 경로에서도 native handle을 먼저 정리한다. */
        fputs("dd_counter_get failed\n", stderr);    /* 어떤 API 검증이 실패했는지 표준 오류 stream에 기록한다. */
        return 3;                                     /* get 또는 snapshot 불일치 exit code를 반환한다. */
    }                                                 /* snapshot 검증 분기를 닫는다. */

    dd_counter* upper = dd_counter_create(INT64_MAX); /* signed 최대값 handle을 만들어 overflow 거절 계약을 검사한다. */
    if (upper == NULL) {                              /* 두 번째 native allocation 실패를 검사한다. */
        dd_counter_destroy(counter);                  /* 이미 소유한 첫 handle을 오류 경로에서 반환한다. */
        fputs("overflow fixture create failed\n", stderr); /* 검증 fixture 생성 실패를 표준 오류에 기록한다. */
        return 4;                                     /* 두 번째 생성 실패 exit code를 반환한다. */
    }                                                 /* 두 번째 create 실패 분기를 닫는다. */
    int64_t ignored = 0;                              /* 실패 시 사용하지 않을 out buffer도 유효한 주소로 제공한다. */
    const int range_status = dd_counter_add(upper, 1, &ignored); /* INT64_MAX + 1을 요청하되 구현이 계산 전에 거절해야 한다. */
    dd_counter_destroy(upper);                        /* 범위 검사 결과와 무관하게 두 번째 handle을 정확히 한 번 해제한다. */

    printf("ffi_value=%" PRId64 "\n", value);        /* int64_t의 플랫폼별 올바른 format을 사용해 42를 출력한다. */
    dd_counter_destroy(counter);                      /* 성공 경로에서 opaque 객체와 내부 mutex를 정확히 한 번 파괴한다. */
    counter = NULL;                                   /* 해제된 dangling pointer를 우연히 재사용하지 않도록 local 값을 비운다. */
    return value == 42 && range_status == DD_STATUS_OUT_OF_RANGE ? 0 : 5; /* 값과 overflow 상태가 맞으면 성공 0을 반환한다. */
}                                                     /* main block과 C 프로그램 실행을 끝낸다. */
