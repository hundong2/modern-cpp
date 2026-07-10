#include <iostream>
#include <memory>
#include <string>
#include <utility>

class Widget {
public:
    Widget(std::string name, int priority)
        : name_{std::move(name)}, priority_{priority} {
        std::cout << "Widget(name, priority)\n";
    }

    void Print() const {
        std::cout << name_ << " priority=" << priority_ << '\n';
    }

private:
    std::string name_;
    int priority_{};
};

template <typename T, typename... Args>
std::unique_ptr<T> MakeOwned(Args&&... args) {
    // Args&&...는 템플릿 타입 추론과 함께 쓰이면 forwarding reference입니다.
    // lvalue는 lvalue로, rvalue는 rvalue로 보존해서 생성자에 전달할 수 있습니다.
    // std::forward를 빼면 모든 args가 함수 안에서 이름 있는 변수이므로 lvalue가 됩니다.
    return std::make_unique<T>(std::forward<Args>(args)...);
}

void UseName(const std::string& name) {
    auto widget = MakeOwned<Widget>(name, 1);
    // name은 lvalue입니다. Widget 생성자에 전달될 때 복사됩니다.
    widget->Print();
}

int main() {
    std::string name = "cached-widget";
    UseName(name);

    auto widget = MakeOwned<Widget>(std::string{"temporary-widget"}, 5);
    // 임시 string은 rvalue입니다. std::forward 덕분에 Widget 생성자로 이동됩니다.
    widget->Print();
}

