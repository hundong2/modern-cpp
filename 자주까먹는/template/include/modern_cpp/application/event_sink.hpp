#pragma once  // 이벤트 출력 인터페이스의 중복 선언을 방지합니다.

#include <string_view>  // 메시지를 복사하지 않고 읽는 std::string_view를 제공합니다.

namespace modern_cpp::application {  // 유스케이스가 사용하는 출력 포트를 application 계층에 둡니다.

class EventSink {  // 콘솔, 파일, 테스트 가짜 객체로 교체할 수 있는 다형적 인터페이스입니다.
public:  // 서비스가 호출할 공개 계약을 시작합니다.
    virtual ~EventSink() = default;  // shared_ptr가 기반 타입을 파괴해도 파생 객체가 올바르게 정리됩니다.
    virtual void publish(std::string_view message) = 0;  // view는 호출 중에만 빌린 문자열이며 구현이 보관하려면 복사해야 합니다.
};  // EventSink는 출력 정책을 핵심 업무 로직에서 분리합니다.

class ConsoleEventSink final : public EventSink {  // 표준 출력으로 보내는 실제 어댑터입니다.
public:  // 기반 인터페이스 구현을 공개합니다.
    void publish(std::string_view message) override;  // override는 서명이 다르면 컴파일 오류를 내어 실수를 막습니다.
};  // final은 이 간단한 어댑터의 추가 상속을 금지합니다.

}  // namespace modern_cpp::application: 이벤트 출력 계약을 닫습니다.
