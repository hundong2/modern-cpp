#include <iostream>
#include <memory>
#include <string>
#include <utility>

class Resource {
public:
    explicit Resource(std::string name) : name_{std::move(name)} {
        std::cout << "Resource constructed: " << name_ << '\n';
    }

    ~Resource() {
        std::cout << "Resource destroyed: " << name_ << '\n';
    }

    void Use() const {
        std::cout << "use: " << name_ << '\n';
    }

private:
    std::string name_;
};

void ObserveByReference(const Resource& resource) {
    // reference는 "반드시 존재하는 객체를 빌려서 본다"는 의미입니다.
    // nullptr이 될 수 없으므로 호출자는 항상 유효한 객체를 넘겨야 합니다.
    // 소유권은 전혀 없습니다. 이 함수가 resource를 delete하면 안 됩니다.
    resource.Use();
}

void ObserveByPointer(const Resource* resource) {
    // raw pointer는 주소 값입니다.
    // nullptr일 수 있음을 표현하거나, 배열/외부 API와 연결할 때 자주 보입니다.
    // 하지만 "소유한다"는 뜻은 아닙니다. delete 책임이 있는지 불분명해지기 쉽습니다.
    if (resource == nullptr) {
        std::cout << "no resource\n";
        return;
    }
    resource->Use();
}

std::unique_ptr<Resource> MakeUniqueResource() {
    // unique_ptr는 단독 소유권입니다.
    // 이 함수가 만든 힙 객체는 반환된 unique_ptr이 책임집니다.
    // unique_ptr가 스코프를 벗어나면 자동으로 delete가 호출됩니다.
    return std::make_unique<Resource>("unique");
}

void TakeOwnership(std::unique_ptr<Resource> resource) {
    // unique_ptr를 값으로 받으면 소유권을 넘겨받겠다는 API입니다.
    // 호출자는 std::move로 명시적으로 넘겨야 하므로 의도가 분명합니다.
    resource->Use();
} // 여기서 resource가 파괴되고 Resource도 같이 파괴됩니다.

void ShareOwnership(std::shared_ptr<Resource> resource) {
    // shared_ptr를 값으로 받으면 참조 카운트가 1 증가합니다.
    // 함수가 끝나면 이 복사본이 파괴되어 참조 카운트가 1 감소합니다.
    // 소유권 공유가 필요 없고 읽기만 한다면 const Resource&가 더 가볍습니다.
    std::cout << "inside ShareOwnership use_count=" << resource.use_count() << '\n';
    resource->Use();
}

void ObserveSharedObject(const std::shared_ptr<Resource>& resource) {
    // const shared_ptr&는 shared_ptr 객체 자체를 복사하지 않으므로
    // 참조 카운트 증가/감소 비용은 없습니다.
    // 하지만 이 함수가 소유권을 연장하지 않는다는 점을 API 의미로 확인해야 합니다.
    std::cout << "observe shared object use_count=" << resource.use_count() << '\n';
    resource->Use();
}

struct BadNode {
    explicit BadNode(std::string value) : value{std::move(value)} {
        std::cout << "BadNode constructed: " << this->value << '\n';
    }

    ~BadNode() {
        std::cout << "BadNode destroyed: " << value << '\n';
    }

    std::string value;
    std::shared_ptr<BadNode> next;
    std::shared_ptr<BadNode> prev;
};

struct GoodNode {
    explicit GoodNode(std::string value) : value{std::move(value)} {
        std::cout << "GoodNode constructed: " << this->value << '\n';
    }

    ~GoodNode() {
        std::cout << "GoodNode destroyed: " << value << '\n';
    }

    std::string value;
    std::shared_ptr<GoodNode> next;
    std::weak_ptr<GoodNode> prev;
};

void RawPointerBasics() {
    std::cout << "\n== raw pointer and reference ==\n";

    Resource stack_resource{"stack"};
    ObserveByReference(stack_resource);
    ObserveByPointer(&stack_resource);

    Resource* raw = new Resource{"manual-new"};
    raw->Use();

    delete raw;
    raw = nullptr;
    // delete 후 raw가 예전 주소를 계속 들고 있으면 dangling pointer입니다.
    // dangling pointer를 역참조하면 이미 반환된 메모리를 읽을 수 있어 정의되지 않은 동작입니다.
    // 그래서 수동 new/delete는 실무 코드에서 최대한 피하고 RAII 타입을 사용합니다.
}

void UniquePointerBasics() {
    std::cout << "\n== unique_ptr ownership ==\n";

    auto owned = MakeUniqueResource();
    ObserveByReference(*owned);
    ObserveByPointer(owned.get());
    // get()은 내부 raw pointer를 빌려줍니다.
    // get()으로 받은 포인터를 delete하면 unique_ptr도 나중에 delete하려 하므로 이중 해제입니다.

    TakeOwnership(std::move(owned));

    if (!owned) {
        std::cout << "owned is empty after move\n";
    }
}

void SharedPointerBasics() {
    std::cout << "\n== shared_ptr reference count ==\n";

    auto shared = std::make_shared<Resource>("shared");
    // make_shared는 Resource 객체와 shared_ptr 제어 블록을 한 번의 할당으로
    // 함께 만들 수 있어 shared_ptr<Resource>(new Resource)보다 보통 효율적입니다.
    //
    // 제어 블록(control block)에는 보통 다음 정보가 들어갑니다.
    // - 강한 참조 카운트: shared_ptr 개수
    // - 약한 참조 카운트: weak_ptr 개수
    // - deleter/allocator 정보
    std::cout << "after make_shared use_count=" << shared.use_count() << '\n';

    {
        auto copied = shared;
        std::cout << "after copy use_count=" << shared.use_count() << '\n';

        ShareOwnership(shared);
        ObserveSharedObject(shared);
    }

    std::cout << "after copied destroyed use_count=" << shared.use_count() << '\n';
}

void WeakPointerBasics() {
    std::cout << "\n== weak_ptr observer ==\n";

    std::weak_ptr<Resource> weak;

    {
        auto shared = std::make_shared<Resource>("weak-target");
        weak = shared;

        if (auto locked = weak.lock()) {
            // lock은 객체가 아직 살아 있으면 shared_ptr를 만들어 소유권을 잠시 연장합니다.
            locked->Use();
            std::cout << "locked use_count=" << locked.use_count() << '\n';
        }
    }

    if (weak.expired()) {
        std::cout << "weak-target already destroyed\n";
    }
}

void CircularReferenceProblem() {
    std::cout << "\n== circular shared_ptr problem ==\n";

    {
        auto first = std::make_shared<BadNode>("first");
        auto second = std::make_shared<BadNode>("second");

        first->next = second;
        second->prev = first;

        std::cout << "first use_count=" << first.use_count() << '\n';
        std::cout << "second use_count=" << second.use_count() << '\n';
    }

    // 여기서 BadNode destroyed가 출력되지 않습니다.
    // first와 second 지역 shared_ptr는 사라졌지만,
    // first->next와 second->prev가 서로를 shared_ptr로 잡고 있어 참조 카운트가 0이 되지 않습니다.

    std::cout << "\n== break cycle with weak_ptr ==\n";

    {
        auto first = std::make_shared<GoodNode>("first");
        auto second = std::make_shared<GoodNode>("second");

        first->next = second;
        second->prev = first;

        std::cout << "first use_count=" << first.use_count() << '\n';
        std::cout << "second use_count=" << second.use_count() << '\n';
    }

    // GoodNode는 prev가 weak_ptr이므로 소유권 순환이 생기지 않습니다.
    // 스코프를 벗어나면 참조 카운트가 0이 되어 정상적으로 파괴됩니다.
}

int main() {
    RawPointerBasics();
    UniquePointerBasics();
    SharedPointerBasics();
    WeakPointerBasics();
    CircularReferenceProblem();

    // 실무 판단 기준:
    // 1. 소유하지 않고 반드시 존재한다: T& 또는 const T&
    // 2. 소유하지 않고 없을 수도 있다: T* 또는 const T*
    // 3. 단독 소유한다: std::unique_ptr<T>
    // 4. 공동 소유가 진짜 필요하다: std::shared_ptr<T>
    // 5. shared_ptr 객체를 관찰하되 수명을 늘리고 싶지 않다: std::weak_ptr<T>
}

