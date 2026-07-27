#include <iostream>
#include <string>
#include <vector>
#include <memory>  // 스마트 포인터 (std::unique_ptr, std::make_unique)
#include <utility> // std::move

// ==============================================================================
// 모던 C++ 기반: 스마트 포인터를 활용한 무거운 객체 시뮬레이션
// ==============================================================================
class HeavyAsset {
private:
    // 원시 포인터(int*) 대신 오직 '독점적 소유권'만 가지는 unique_ptr 사용
    std::unique_ptr<int[]> data; 
    std::string name;

public:
    // [1] 일반 생성자
    // 메모리 할당 시에도 new를 쓰지 않고 std::make_unique를 사용하는 것이 정석입니다.
    HeavyAsset(std::string n) : name(n), data(std::make_unique<int[]>(1000)) {
        std::cout << "  (+) [생성] '" << name << "' 만들어짐 (주소: " << data.get() << ")\n";
    }

    // [2] 소멸자
    ~HeavyAsset() {
        // 💡 [모던 C++의 장점] delete를 직접 쳐줄 필요가 아예 없습니다!
        // 스코프를 벗어나면 unique_ptr이 알아서 메모리를 해제합니다.
        // 여기서는 단지 로깅을 위해 data가 살아있는지 검사만 합니다.
        if (data) {
            std::cout << "  (-) [소멸] '" << name << "' 파괴됨 (unique_ptr이 메모리 자동 해제)\n";
        } else {
            std::cout << "  (-) [소멸] '" << name << "' 파괴됨 (이미 털린 빈 껍데기)\n";
        }
    }

    // ==============================================================================
    // [3] 복사 생성자 (lvalue 전달 시)
    // 🚨 중요: std::unique_ptr은 이름 그대로 '독점'이므로 복사 자체가 문법적으로 불가능합니다!
    // 따라서 객체 복사를 허용하려면, 우리가 직접 새 메모리를 파서 '깊은 복사'를 해줘야 합니다.
    // 이는 개발자가 "아, 내가 지금 성능 깎아먹는 무거운 짓을 하고 있구나"라고 인지하게 만듭니다.
    // ==============================================================================
    HeavyAsset(const HeavyAsset& other) : name(other.name + "_복사본") {
        if (other.data) {
            data = std::make_unique<int[]>(1000); // 새로운 공간 할당
            std::copy(other.data.get(), other.data.get() + 1000, data.get()); // 실제 데이터 복사
        }
        std::cout << "  (C) [복사] '" << other.name << "' 깊은 복사 발생! (비용 큼)\n";
        std::cout << "      -> 원본: " << other.data.get() << " | 사본: " << data.get() << "\n";
    }

    // ==============================================================================
    // [4] 이동 생성자 (rvalue 전달 시)
    // 💡 [모던 C++의 장점] 수동으로 nullptr을 넣을 필요가 없습니다.
    // std::move로 unique_ptr을 넘기면, 자원을 뺏어옴과 동시에 원본을 빈 껍데기로 알아서 만듭니다.
    // ==============================================================================
    HeavyAsset(HeavyAsset&& other) noexcept : name(other.name + "_이동됨") {
        data = std::move(other.data); // 스마트 포인터의 소유권 다이렉트 이전!
        std::cout << "  (M) [이동] '" << other.name << "' 의 unique_ptr 소유권 훔쳐옴.\n";
        std::cout << "      -> 훔친 주소: " << data.get() << "\n";
    }
};

// ==============================================================================
// 메인 테스트 함수
// ==============================================================================
int main() {
    std::cout << "\n=== 1. lvalue 전달 (비싼 복사 발생) ===\n";
    HeavyAsset asset1("성검"); 
    HeavyAsset asset2 = asset1; // lvalue: 깊은 복사 생성자 호출


    std::cout << "\n=== 2. rvalue 전달 (Zero-Copy 소유권 이동) ===\n";
    // 임시 객체는 rvalue이므로 이동 생성자가 호출되어 소유권만 쏙 빼옵니다.
    HeavyAsset asset3 = HeavyAsset("임시방패"); 


    std::cout << "\n=== 3. std::move로 lvalue 강제 이동시키기 ===\n";
    // asset2를 더 이상 안 쓸 것이므로, std::move로 강제로 rvalue 캐스팅을 합니다.
    // asset2 내부에 있던 unique_ptr의 데이터는 asset4로 완벽히 소유권이 넘어갑니다.
    HeavyAsset asset4 = std::move(asset2); 
    

    std::cout << "\n=== 4. std::vector 실무 사용 시뮬레이션 ===\n";
    std::vector<HeavyAsset> inventory;
    inventory.reserve(10); 

    std::cout << "[lvalue 삽입]\n";
    inventory.push_back(asset1); // 복사됨 (비용 큼)

    std::cout << "[rvalue 삽입]\n";
    inventory.push_back(HeavyAsset("화살")); // 임시 객체: 훔쳐옴 (비용 없음)

    std::cout << "\n[std::move를 통한 lvalue 삽입]\n";
    inventory.push_back(std::move(asset4)); // asset4의 자원을 벡터 안으로 훔쳐 넣음!

    std::cout << "\n=== 프로그램 종료 (안전한 자동 소멸 시작) ===\n";
    return 0;
}