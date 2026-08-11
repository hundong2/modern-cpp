#include <iostream>
#include <memory>
#include <string>
#include <utility>

// ---------------------------------------------------------
// [1] 리소스 클래스 정의
// ---------------------------------------------------------
class Texture {
private:
    std::string name_;
public:
    // explicit: Texture tex = "hero"; 같은 암시적 변환 차단
    explicit Texture(std::string name) : name_{std::move(name)} {
        std::cout << "[할당] 텍스처 로드: " << name_ << "\n";
    }
    // 소멸자: 스마트 포인터가 자동으로 호출해 줍니다.
    ~Texture() {
        std::cout << "[해제] 텍스처 삭제: " << name_ << "\n";
    }
    
    // const: 멤버 변수(상태)를 변경하지 않는 읽기 전용 함수임을 보장
    void render() const {
        std::cout << "렌더링 중: " << name_ << "\n";
    }
};

// ---------------------------------------------------------
// [2] 스마트 포인터 실무 활용 예시
// ---------------------------------------------------------
class GraphicEngine {
private:
    // std::unique_ptr: "이 메모리는 나 혼자만(독점) 소유한다!"
    // GraphicEngine이 파괴될 때, 이 텍스처도 자동으로 파괴됩니다. 메모리 누수 0% 보장.
    std::unique_ptr<Texture> backgroundTexture_;

public:
    // 생성자
    GraphicEngine() {
        // std::make_unique (C++14): 원시 포인터(new)를 쓰지 않고 가장 안전하고 빠르게 생성.
        backgroundTexture_ = std::make_unique<Texture>("Background_4K.png");
    }

    // constexpr: 컴파일 타임에 반환값이 결정되게 하여 런타임 비용 0으로 만듦
    constexpr int getEngineVersion() const {
        return 2024;
    }

    void draw() const {
        if (backgroundTexture_) {
            // unique_ptr은 포인터처럼 -> 연산자로 내부 객체에 접근합니다.
            backgroundTexture_->render(); 
        }
    }

    // 소유권 이전 (Transfer Ownership)
    // unique_ptr은 복사(Copy)가 불가능합니다. 무조건 이동(Move)만 가능합니다.
    std::unique_ptr<Texture> releaseBackground() {
        // std::move: "내 텍스처 소유권을 밖으로 던져줄 테니, 내 포인터는 비워둬라"
        return std::move(backgroundTexture_); 
    }
};

int main() {
    std::cout << "--- 엔진 시작 ---\n";
    {
        GraphicEngine engine;
        engine.draw();
        
        // [중요] engine 객체가 중괄호(스코프)를 벗어나는 순간, 
        // 내부에 있는 unique_ptr이 알아서 Texture의 소멸자를 호출(delete)합니다.
    } 
    std::cout << "--- 엔진 종료 (메모리 누수 없음) ---\n\n";

    // ---------------------------------------------------------
    // [3] shared_ptr: 자원 공유 (예: 여러 UI가 하나의 폰트를 공유할 때)
    // ---------------------------------------------------------
    // std::make_shared: 참조 카운트(Reference Count) 블록과 객체를 한 번에 메모리에 연속 할당하여 캐시 효율 극대화.
    std::shared_ptr<Texture> sharedIcon = std::make_shared<Texture>("Common_Icon.png");
    
    {
        // auto: 타입 추론. 긴 타입을 다 쓰지 않아도 됩니다. (우변을 보고 std::shared_ptr<Texture>로 추론)
        auto uiButton1 = sharedIcon; // 참조 카운트 증가 (현재 2)
        auto uiButton2 = sharedIcon; // 참조 카운트 증가 (현재 3)
        
        std::cout << "현재 아이콘 참조 횟수: " << sharedIcon.use_count() << "\n";
        // 이 중괄호가 끝나면 uiButton1, 2가 파괴되며 카운트가 1로 줄어듦. (객체는 아직 삭제 안 됨)
    }
    
    std::cout << "uiButton들 소멸 후 참조 횟수: " << sharedIcon.use_count() << "\n";
    
    // main 함수가 끝나면서 sharedIcon도 파괴될 때 비로소 카운트가 0이 되며 텍스처가 삭제됨.
    return 0;
}


// --- 엔진 시작 ---
// [할당] 텍스처 로드: Background_4K.png
// 렌더링 중: Background_4K.png
// [해제] 텍스처 삭제: Background_4K.png
// --- 엔진 종료 (메모리 누수 없음) ---

// [할당] 텍스처 로드: Common_Icon.png
// 현재 아이콘 참조 횟수: 3
// uiButton들 소멸 후 참조 횟수: 1
// [해제] 텍스처 삭제: Common_Icon.png