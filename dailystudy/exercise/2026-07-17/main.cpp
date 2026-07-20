/*
[기초 문법부터 읽는 순서]
1. Book 구조체는 문자열 두 개와 bool 상태를 한 객체로 묶습니다. bool은 true/false입니다.
2. class는 데이터와 동작을 묶고 public/private로 접근 범위를 나눕니다.
3. virtual 함수는 BookRepository 참조를 통해 실제 저장소 구현을 호출하게 합니다.
   `= 0`은 파생 클래스가 반드시 구현해야 하는 순수 가상 함수입니다.
4. override는 기반 클래스 함수를 정확히 재정의했는지, final은 더 상속하지 않을지를
   컴파일러가 검사하게 합니다.
5. vector<Book>은 책 목록을 소유하며 std::move는 소유한 자원을 다른 객체로 넘길
   수 있게 합니다.
6. find_if의 람다 `[id](const Book& book)`는 id를 값으로 캡처하고 각 책을 참조로 읽습니다.
7. optional<Book>은 검색 결과가 있거나 없음을 표현합니다. !book은 없음, book->title은
   들어 있는 Book의 멤버 접근입니다.
8. LendingService는 인터페이스 참조만 의존하므로 메모리 저장소를 DB 저장소로 바꿔도
   대출 규칙을 수정할 필요가 없습니다.
*/

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
    bool is_borrowed{};
};

// The service depends on this small abstraction, not on a database or vector.
class BookRepository {
public:
    virtual ~BookRepository() = default;
    [[nodiscard]] virtual std::optional<Book> find_by_id(std::string_view id) const = 0;
    virtual void save(Book book) = 0;
};

class InMemoryBookRepository final : public BookRepository {
public:
    explicit InMemoryBookRepository(std::vector<Book> books)
        : books_{std::move(books)} {
    }

    [[nodiscard]] std::optional<Book> find_by_id(std::string_view id) const override {
        const auto found = std::find_if(books_.begin(), books_.end(), [id](const Book& book) {
            return book.id == id;
        });

        if (found == books_.end()) {
            return std::nullopt;
        }
        return *found; // Return a copy so callers cannot secretly change repository state.
    }

    void save(Book book) override {
        const auto found = std::find_if(books_.begin(), books_.end(), [&book](const Book& stored) {
            return stored.id == book.id;
        });

        if (found == books_.end()) {
            books_.push_back(std::move(book));
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
