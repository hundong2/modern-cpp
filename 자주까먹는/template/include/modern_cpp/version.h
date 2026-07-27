#pragma once  // 같은 번역 단위에서 이 헤더가 여러 번 포함되어도 선언을 한 번만 보이게 합니다.

#include <string_view>  // 문자열을 소유하지 않는 읽기 전용 뷰 std::string_view를 제공합니다.

namespace modern_cpp {  // 프로젝트의 공개 이름을 전역 이름 공간과 분리합니다.

inline constexpr std::string_view project_name{"modern-cpp23-template"};  // inline 변수라 여러 번역 단위에 정의되어도 ODR 위반이 없습니다.
inline constexpr int project_version_major{1};  // constexpr 값은 컴파일 시간 상수로 사용할 수 있으며 별도 저장 공간이 생략될 수 있습니다.
inline constexpr int project_version_minor{0};  // 중괄호 초기화는 정수 축소 변환을 컴파일 단계에서 막아 줍니다.

}  // namespace modern_cpp: 공개 버전 상수의 이름 공간을 닫습니다.
