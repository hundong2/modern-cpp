#include <iostream>
#include <string>
#include <vector>
#include <utility> // std::move, std::forward
#include <memory>  // std::unique_ptr, std::make_unique

// constexpr: 런타임 연산 없이, '컴파일하는 순간'에 메모리에 박아버리는 상수입니다.
// 변하지 않는 매직 넘버(매개변수 기본값, 버퍼 크기 등)에 사용하여 성능을 극대화합니다.
constexpr size_t DEFAULT_BUFFER_SIZE = 1000;

class GameResource {
private:
    std::string name_;
    std::vector<int> heavy_data_;

public:
    // [생성자]
    // 1. name은 const std::string&로 받아 원본 수정을 막고(읽기 전용), 내부에서 복사합니다.
    // 2. data는 std::vector<int>&& (Rvalue 참조)로 받아, 힙 메모리의 소유권을 훔쳐옵니다(이동).
    explicit GameResource(const std::string& name, std::vector<int>&& data)
        : name_{name}, heavy_data_{std::move(data)} 
    {
        std::cout << "[Resource] " << name_ << " 생성 완료. (이동 최적화 적용)\n";
    }
};

// ==============================================================================
// [완벽한 전달(Perfect Forwarding)의 핵심 파트]
// ==============================================================================

// 템플릿 타입 T 뒤에 붙은 &&는 'Rvalue 참조'가 아닙니다.
// 타입 추론이 발생하는 템플릿에서는 이를 "보편 참조(Universal Reference)"라고 부릅니다.
// 어떤 값이든(Lvalue, Rvalue, const 등) 튕겨내지 않고 전부 다 흡수하는 블랙홀 같은 녀석입니다.
template <typename... Args>
std::unique_ptr<GameResource> resourceFactory(Args&&... args) {
    
    // std::forward<Args>(args)... 의 마법
    // 1. args가 외부에서 Lvalue로 들어왔다면 -> 아무것도 안 하고 참조(&)만 넘깁니다. (복사 발생 X, 강탈 발생 X)
    // 2. args가 외부에서 Rvalue(임시 객체)로 들어왔다면 -> std::move처럼 Rvalue로 캐스팅하여 넘깁니다. (강탈 O)
    return std::make_unique<GameResource>(std::forward<Args>(args)...);
}

int main() {
    std::vector<int> buffer(DEFAULT_BUFFER_SIZE, 1);
    const std::string resource_name = "Boss_Texture";

    // 1. resource_name은 변수(Lvalue)이므로 Lvalue 참조로 안전하게 복사되어 전달됩니다.
    // 2. std::move(buffer)는 Rvalue이므로, factory를 거쳐 GameResource 생성자까지 
    //    그 어떠한 추가 복사도 없이 힙 메모리 소유권만 정확히 전달(이동)됩니다.
    auto boss = resourceFactory(resource_name, std::move(buffer));

    // 결과: buffer는 정확히 의도한 대로 소유권을 빼앗겨 빈 껍데기가 됩니다.
    std::cout << "팩토리 호출 후 원본 buffer 크기: " << buffer.size() << "\n"; // 0 출력

    return 0;
}


// [Resource] Boss_Texture 생성 완료. (이동 최적화 적용)
// 팩토리 호출 후 원본 buffer 크기: 0