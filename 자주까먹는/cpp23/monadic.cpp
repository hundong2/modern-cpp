#include <iostream>
#include <string>
#include <expected>
#include <charconv>

// 1. 에러 타입 정의
enum class ErrorCode {
    InvalidInputFormat, // 입력값이 숫자가 아님
    UserNotFound        // DB에 유저가 없음
};

// 2. 데이터 구조체
struct User {
    int id;
    std::string name;
};

// ---------------------------------------------------------
// 파이프라인을 구성할 개별 함수들 (작은 블록들)
// ---------------------------------------------------------

// [단계 1] 문자열을 정수로 파싱 (실패 가능성 있음 -> expected 반환)
std::expected<int, ErrorCode> parse_id(const std::string& input) {
    if (input.empty() || !std::isdigit(input[0])) {
        return std::unexpected(ErrorCode::InvalidInputFormat);
    }
    return std::stoi(input);
}

// [단계 2] DB에서 유저 조회 (실패 가능성 있음 -> expected 반환)
std::expected<User, ErrorCode> fetch_user_from_db(int id) {
    if (id == 42) return User{42, "Alice"};
    if (id == 77) return User{77, "Bob"};
    
    return std::unexpected(ErrorCode::UserNotFound); // 그 외는 없음
}

// [단계 3] 유저 정보를 바탕으로 메시지 생성 (무조건 성공 -> 일반 값 반환)
std::string create_greeting(const User& user) {
    return "환영합니다, " + user.name + "님! (ID: " + std::to_string(user.id) + ")";
}

// ---------------------------------------------------------
// 모나드 연산을 이용한 체이닝 (핵심)
// ---------------------------------------------------------
void process_user_request(const std::string& input) {
    
    // 복잡한 if문 없이 물 흐르듯 데이터가 전달됩니다.
    auto result = parse_id(input)
                    .and_then(fetch_user_from_db)
                    .transform(create_greeting);

    // 최종 결과 확인
    if (result.has_value()) {
        std::cout << "✅ 성공: " << result.value() << '\n';
    } else {
        std::cout << "❌ 실패: ";
        switch (result.error()) {
            case ErrorCode::InvalidInputFormat: 
                std::cout << "숫자 형식의 ID가 아닙니다.\n"; break;
            case ErrorCode::UserNotFound:       
                std::cout << "해당 ID의 유저를 찾을 수 없습니다.\n"; break;
        }
    }
}

int main() {
    std::cout << "--- 모나드 체이닝 테스트 ---\n";
    
    // 1. 완벽한 성공 케이스
    process_user_request("42"); 
    
    // 2. 단계 2에서 실패하는 케이스 (UserNotFound)
    process_user_request("99"); 
    
    // 3. 단계 1에서 실패하는 케이스 (InvalidInputFormat)
    process_user_request("abc");

    return 0;
}


// --- 모나드 체이닝 테스트 ---
// ✅ 성공: 환영합니다, Alice님! (ID: 42)
// ❌ 실패: 해당 ID의 유저를 찾을 수 없습니다.
// ❌ 실패: 숫자 형식의 ID가 아닙니다.