#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class Status {
public:
    static Status Ok() {
        return Status{true, ""};
    }

    static Status Error(std::string message) {
        return Status{false, std::move(message)};
    }

    [[nodiscard]] bool ok() const {
        return ok_;
    }

    [[nodiscard]] const std::string& message() const {
        return message_;
    }

private:
    Status(bool ok, std::string message)
        : ok_{ok}, message_{std::move(message)} {}

    bool ok_{};
    std::string message_;
};

template <typename T>
class Result {
public:
    Result(T value) : value_{std::move(value)}, status_{Status::Ok()} {}
    Result(Status status) : status_{std::move(status)} {}

    [[nodiscard]] bool ok() const {
        return status_.ok();
    }

    [[nodiscard]] const T& value() const {
        return value_;
    }

    [[nodiscard]] const Status& status() const {
        return status_;
    }

private:
    // 간단한 학습 예제라 기본 생성 가능한 T만 다룹니다.
    // 실무에서는 std::optional<T>, std::variant<T, Status>,
    // 또는 C++23 std::expected<T, E> 같은 타입을 고려합니다.
    T value_{};
    Status status_{Status::Ok()};
};

Result<int> ParsePositiveInt(std::string_view text) {
    int value = 0;

    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return Status::Error("not a number");
        }
        value = value * 10 + (ch - '0');
    }

    if (value == 0) {
        return Status::Error("zero is not positive");
    }

    return value;
}

int main() {
    const std::vector<std::string_view> inputs{"42", "0", "12x"};

    for (std::string_view input : inputs) {
        const auto parsed = ParsePositiveInt(input);

        if (!parsed.ok()) {
            std::cout << input << " failed: " << parsed.status().message() << '\n';
            continue;
        }

        std::cout << input << " parsed=" << parsed.value() << '\n';
    }

    // [[nodiscard]]를 붙이면 반환 상태를 무시하는 실수를 컴파일러가 경고할 수 있습니다.
    // 대규모 서버 코드에서는 예외 대신 Status/Result를 반환해 오류 흐름을 명시하기도 합니다.
}

