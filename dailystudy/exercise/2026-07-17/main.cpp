#include <algorithm>    // 조건에 맞는 원소를 찾는 std::find_if를 사용한다.
#include <cstdlib>      // 실패 시 std::exit와 EXIT_FAILURE를 사용한다.
#include <iostream>     // cout과 cerr로 결과와 오류를 출력한다.
#include <optional>     // 책을 찾았거나 못 찾은 상태를 optional로 표현한다.
#include <string>       // 책 식별자·제목·메시지의 문자를 소유한다.
#include <string_view>  // 조회 문자열을 복사하지 않고 읽는다.
#include <utility>      // std::move로 소유권 이동 후보를 만든다.
#include <vector>       // 메모리 저장소가 Book 목록을 소유한다.

struct Book {
    // struct 멤버는 기본 public이며 두 string이 문자 저장 공간을 직접 소유한다.
    std::string id;
    std::string title;
    bool is_borrowed{}; // {}로 false 초기화. bool의 실제 메모리 크기는 구현에 따라 달라질 수 있다.
};

// 서비스는 DB나 vector 구현이 아니라 이 작은 추상 인터페이스에만 의존한다.
class BookRepository {
public:
    // 가상 소멸자는 기반 포인터로 파생 객체를 삭제할 때 전체 소멸을 보장한다.
    virtual ~BookRepository() = default; // 기반 포인터 삭제 시 파생 소멸자까지 호출되도록 가상화한다.
    // virtual과 =0은 파생 저장소가 구현해야 하는 순수 가상 함수 계약이다.
    [[nodiscard]] virtual std::optional<Book> find_by_id(std::string_view id) const = 0;
    // Book을 값으로 받아 저장소가 복사 또는 이동을 통해 독립 소유하게 한다.
    virtual void save(Book book) = 0;
};

class InMemoryBookRepository final : public BookRepository {
public:
    // final은 추가 상속을 막고 public 상속은 저장소 계약 구현을 뜻한다.
    // explicit은 vector에서 저장소로의 뜻밖의 암시적 변환을 막는다.
    explicit InMemoryBookRepository(std::vector<Book> books)
        // books는 이름이 있어 lvalue지만 move가 xvalue로 바꿔 vector 버퍼 소유권 이전을 허용한다.
        : books_{std::move(books)} {
    }

    [[nodiscard]] std::optional<Book> find_by_id(std::string_view id) const override {
        // override는 기반 클래스의 가상 함수 서명과 정확히 일치하는지 검사한다.
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
        // &book 캡처는 지역 매개변수의 주소를 빌린다. 람다는 이 함수 호출 안에서만 사용된다.
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
    // private vector가 모든 Book 원소의 수명과 저장 공간을 소유한다.
    std::vector<Book> books_;
};

enum class LoanStatus {
    // enum class는 대출 결과를 제한된 이름 집합으로 만들고 정수와의 혼용을 막는다.
    borrowed,
    returned,
    not_found,
    already_borrowed,
    already_available
};

struct LoanResult {
    LoanStatus status;    // 결과 분기에 사용하는 타입 안전한 상태다.
    std::string message;  // 사용자 메시지의 문자 버퍼를 결과가 소유한다.
};

class LendingService {
public:
    // repository_는 비소유 참조이므로 저장소 객체가 서비스보다 오래 살아야 한다.
    // 참조 매개변수와 멤버는 저장소를 소유하지 않으므로 저장소가 더 오래 살아야 한다.
    explicit LendingService(BookRepository& repository)
        : repository_{repository} {
    }

    [[nodiscard]] LoanResult borrow(std::string_view id) {
        // 반환 optional prvalue로 지역 객체를 초기화한다. 저장소와 독립된 Book 복사본을 소유한다.
        auto book = repository_.find_by_id(id);
        if (!book) {
            // optional이 비었으면 조기 반환해 이후 -> 접근을 안전하게 막는다.
            return {LoanStatus::not_found, "book was not found"};
        }
        if (book->is_borrowed) {
            return {LoanStatus::already_borrowed, book->title + " is already borrowed"};
        }

        book->is_borrowed = true;
        // ->는 optional 내부 Book의 public 멤버에 접근한다.
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
    // private 비소유 참조는 외부 저장소 구현을 감추고 인터페이스에만 의존한다.
    BookRepository& repository_;
};

void require(bool condition, std::string_view explanation) {
    // bool 조건이 거짓일 때만 오류를 출력하고 프로세스를 실패 코드로 종료한다.
    if (!condition) {
        std::cerr << "[FAILED] " << explanation << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void print_result(const LoanResult& result) {
    std::cout << "[RESULT] " << result.message << '\n';
}

int main() {
    // 중첩 중괄호는 vector<Book> 매개변수와 그 안 Book 원소들을 직접 초기화한다.
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
