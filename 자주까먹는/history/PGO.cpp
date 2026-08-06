#include <iostream>
#include <vector>
#include <functional>
#include <string_view>
#include <expected>

// ==========================================
// [C++11] std::function과 간접 호출 (Indirect Call)
// ==========================================
// std::function은 내부적으로 함수 포인터나 작은 vtable을 씁니다.
// CPU 입장에서는 포인터를 타고 가야 하므로 분기 예측이 매우 어렵습니다.
using PacketHandler = std::function<void(int)>;

// ==========================================
// [C++17] std::string_view
// ==========================================
// 문자열 복사(Heap 할당) 오버헤드를 없애고 메모리 뷰만 전달합니다.
// PGO가 캐시 라인(Cache Line)을 덜 오염시키도록 돕는 훌륭한 도구입니다.
void log_packet(std::string_view msg) {
    // 로깅 로직 (생략)
}

// ==========================================
// [C++20] [[likely]] / [[unlikely]] (수동 PGO 힌트)
// ==========================================
// PGO 파이프라인을 구축하기 힘들 때, 개발자가 직접 컴파일러와 CPU의
// 분기 예측기에게 힌트를 줄 수 있게 되었습니다.
bool is_valid_packet(int packet_id) {
    if (packet_id > 0) [[likely]] {
        return true; // 99% 정상 패킷
    } else [[unlikely]] {
        return false; // 1% 확률의 악성 패킷 또는 에러
    }
}

// ==========================================
// [C++23] std::expected (핫 패스와 콜드 패스의 분리)
// ==========================================
// 예외(throw/catch)는 콜드 패스(Cold Path)를 만들어 I-Cache를 망가뜨립니다.
// std::expected는 정상 값과 에러 값을 조합하여 분기 예측을 최적화합니다.
std::expected<int, std::string> parse_packet(int raw_data) {
    if (is_valid_packet(raw_data)) {
        return raw_data * 2; // 정상 처리
    } else {
        return std::unexpected("Invalid Packet Format"); // 에러 처리
    }
}

int main() {
    // 기본 컨테이너 복습: 벡터에 더미 네트워크 패킷 데이터를 넣습니다.
    std::vector<int> network_traffic = { 10, 20, -1, 30, 40 }; 
    
    // [PGO와 Devirtualization(가상화 해제)의 마법]
    // std::function은 어떤 함수가 들어올지 런타임에 결정되지만,
    // PGO를 켜고 트레이닝(2단계)을 해보면 "99% 확률로 람다가 호출되더라"는 통계가 나옵니다.
    // LTO+PGO는 이 간접 호출을 '직접 호출(Direct Call)' 또는 아예 '인라인(Inline)'으로 어셈블리를 뜯어고칩니다.
    PacketHandler handler = [](int data) {
        auto result = parse_packet(data);
        
        // C++23: std::expected의 has_value()로 분기 처리
        if (result.has_value()) {
            // [어셈블리 관점] PGO를 거치면 이 "정상 처리" 블록의 기계어가 
            // if문 바로 뒤에 일직선(Straight-line)으로 배치되어 명령어 캐시 히트율이 100%에 수렴합니다.
            log_packet("Success");
        } else {
            // "에러 처리" 블록의 기계어는 메모리 저 멀리(Cold Section)로 쫓겨납니다.
            // 덕분에 L1 캐시에 쓸데없는 코드가 올라오지 않습니다.
            log_packet(result.error());
        }
    };

    // 데이터 처리 루프
    for (int packet : network_traffic) {
        handler(packet);
    }

    std::cout << "패킷 처리 완료\n";
    return 0; // OS에 정상 종료 알림
}
