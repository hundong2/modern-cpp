#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Book {
    std::string id;
    std::string title;
    bool is_borrowed{}; // {}로 false 초기화. bool의 실제 메모리 크기는 구현에 따라 달라질 수 있다.
};

// The service depends on this small abstraction, not on a database or vector.
class BookRepository {
public:
    virtual ~BookRepository() = default; // 기반 포인터 삭제 시 파생 소멸자까지 호출되도록 가상화한다.
    [[nodiscard]] virtual std::optional<Book> find_by_id(std::string_view id) const = 0;
    virtual void save(Book book) = 0;
};

class InMemoryBookRepository final : public BookRepository {
public:
    explicit InMemoryBookRepository(std::vector<Book> books)
        // books는 이름이 있어 lvalue지만 move가 xvalue로 바꿔 vector 버퍼 소유권 이전을 허용한다.
        : books_{std::move(books)} {
    }

    [[nodiscard]] std::optional<Book> find_by_id(std::string_view id) const override {
        // [id]는 string_view를 람다 객체 안에 값 복사한다. book은 각 vector 원소의 const lvalue 참조다.
        const auto found = std::find_if(books_.begin(), books_.end(), [id](const Book& book) {
            return book.id == id;
        });

        if (found == books_.end()) {
            return std::nullopt;
        }
        return *found; // *found는 Book lvalue이므로 복사해 반환하여 저장소 내부를 외부가 몰래 바꾸지 못하게 한다.
    }

    void save(Book book) override {
        const auto found = std::find_if(books_.begin(), books_.end(), [&book](const Book& stored) {
            return stored.id == book.id;
        });

        if (found == books_.end()) {
            books_.push_back(std::move(book)); // xvalue를 전달해 문자열 버퍼 이동 생성자를 선택할 수 있다.
        } else {
            *found = std::move(book);
        }
    }

private:
    std::vector<Book> books_;
};

enum class LoanStatus {
    borrowed,
    returned,
    not_found,
    already_borrowed,
    already_available
};

struct LoanResult {
    LoanStatus status;
    std::string message;
};

class LendingService {
public:
    // The repository must outlive this service because repository_ is a reference.
    explicit LendingService(BookRepository& repository)
        : repository_{repository} {
    }

    [[nodiscard]] LoanResult borrow(std::string_view id) {
        // 반환 optional prvalue로 지역 객체를 초기화한다. 저장소와 독립된 Book 복사본을 소유한다.
        auto book = repository_.find_by_id(id);
        if (!book) {
            return {LoanStatus::not_found, "book was not found"};
        }
        if (book->is_borrowed) {
            return {LoanStatus::already_borrowed, book->title + " is already borrowed"};
        }

        book->is_borrowed = true;
        const std::string title = book->title;
        repository_.save(std::move(*book));
        return {LoanStatus::borrowed, title + " was borrowed"};
    }

    [[nodiscard]] LoanResult return_book(std::string_view id) {
        auto book = repository_.find_by_id(id);
        if (!book) {
            return {LoanStatus::not_found, "book was not found"};
        }
        if (!book->is_borrowed) {
            return {LoanStatus::already_available, book->title + " is already available"};
        }

        book->is_borrowed = false;
        const std::string title = book->title;
        repository_.save(std::move(*book));
        return {LoanStatus::returned, title + " was returned"};
    }

private:
    BookRepository& repository_;
};

void require(bool condition, std::string_view explanation) {
    if (!condition) {
        std::cerr << "[FAILED] " << explanation << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void print_result(const LoanResult& result) {
    std::cout << "[RESULT] " << result.message << '\n';
}

int main() {
    InMemoryBookRepository repository{{
        {"cpp-101", "C++ Foundations", false},
        {"arch-20", "Small Software Architecture", true},
    }};
    LendingService service{repository};

    const auto first = service.borrow("cpp-101");
    const auto second = service.borrow("cpp-101");
    const auto third = service.return_book("arch-20");
    const auto missing = service.borrow("missing");

    print_result(first);
    print_result(second);
    print_result(third);
    print_result(missing);

    require(first.status == LoanStatus::borrowed, "an available book can be borrowed");
    require(second.status == LoanStatus::already_borrowed, "a second loan is rejected");
    require(third.status == LoanStatus::returned, "a borrowed book can be returned");
    require(missing.status == LoanStatus::not_found, "an unknown id is reported");
    require(repository.find_by_id("cpp-101")->is_borrowed, "the loan was saved");
    require(!repository.find_by_id("arch-20")->is_borrowed, "the return was saved");

    std::cout << "[TESTS] library architecture example passed\n";
}
