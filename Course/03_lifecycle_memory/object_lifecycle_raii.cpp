#include <iostream>
#include <memory>
#include <string>
#include <utility>

class FileHandle {
public:
    explicit FileHandle(std::string path) : path_{std::move(path)} {
        std::cout << "open: " << path_ << '\n';
    }

    ~FileHandle() {
        std::cout << "close: " << path_ << '\n';
    }

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept : path_{std::move(other.path_)} {
        other.path_ = "<moved>";
    }

    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            path_ = std::move(other.path_);
            other.path_ = "<moved>";
        }
        return *this;
    }

    void write_line(const std::string& text) const {
        std::cout << "write to " << path_ << ": " << text << '\n';
    }

private:
    std::string path_;
};

std::unique_ptr<FileHandle> create_log_file() {
    // FileHandle 객체는 힙에 생성됩니다.
    // unique_ptr 지역 변수는 스택에 있고, 힙 객체의 주소를 소유합니다.
    return std::make_unique<FileHandle>("system.log");
}

int main() {
    auto log = create_log_file();
    log->write_line("service started");

    {
        FileHandle temporary{"temporary.log"};
        temporary.write_line("short lived object");
    } // temporary의 수명이 끝나며 소멸자가 호출됩니다.

    // main이 끝나면 log unique_ptr의 소멸자가 호출되고,
    // 그 안에서 힙의 FileHandle 객체도 delete됩니다.
}

