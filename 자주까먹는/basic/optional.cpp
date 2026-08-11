#include <iostream>
#include <string>
#include <optional>

// constexpr: '컴파일 타임'에 값이 완전히 결정되는 상수입니다. 
// 런타임 오버헤드가 전혀 없으며, 실무에서 절대 변하지 않는 고정 기준값에 주로 씁니다.
constexpr int MIN_AGE_LIMIT = 0; 

// 함수 반환형을 std::optional<int>로 지정하여 "값이 없을 수도 있음"을 명시적으로 선언합니다.
// const: userName 원본 문자열이 함수 내부에서 수정되지 않음을 엄격히 보장합니다. (Read-only)
// &: 레퍼런스로 넘겨 불필요한 문자열 복사(Copy) 비용을 없앱니다.
std::optional<int> getUserAge(const std::string& userName) {
    
    if (userName == "고윤정") {
        return 28; // 유효한 값이 있다면 그대로 반환합니다. 자동으로 optional 객체로 감싸집니다.
    }
    
    // 사용자를 찾을 수 없다면 '비어 있음'을 뜻하는 std::nullopt를 반환합니다.
    return std::nullopt; 
}

int main() {
    // auto: 컴파일러가 반환형(std::optional<int>)을 자동으로 추론합니다. 타이핑을 줄이고 리팩토링에 유리합니다.
    const auto ageOpt = getUserAge("고윤정"); 
    
    // has_value()를 통해 안전하게 값이 존재하는지 검사합니다.
    if (ageOpt.has_value()) {
        // value()를 호출하여 실제 값을 꺼냅니다.
        std::cout << "나이: " << ageOpt.value() << "세\n";
    } else {
        std::cout << "사용자를 찾을 수 없습니다.\n";
    }

    // [실무 꿀팁] value_or()를 사용하면 if문 없이도 값이 없을 때의 기본값(Default)을 쉽게 지정할 수 있습니다.
    const auto unknownAge = getUserAge("홍길동");
    std::cout << "홍길동의 나이: " << unknownAge.value_or(MIN_AGE_LIMIT) << "세\n";

    return 0;
}

// age: 26
// 홍길동의 나이: 0세
