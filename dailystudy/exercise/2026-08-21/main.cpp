// <iostream>은 실무 예제 결과를 쓰는 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <string>은 HTTP 메서드와 URL을 독점 소유하는 std::string을 선언한다.
#include <string>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move 함수 템플릿을 선언한다.
#include <utility>
// <vector>는 헤더 목록을 연속 저장하는 std::vector를 선언한다.
#include <vector>

// struct는 기본 접근이 public이다. 이름과 값만 묶는 단순 전송 레코드라 공개 집합체가 알맞다.
struct Header {
    std::string name{};  // 각 string이 문자를 소유하며 Header와 함께 수명이 끝난다.
    std::string value{};
};

// class는 기본 접근이 private이다. 완성된 요청 상태를 외부가 직접 훼손하지 못하게 한다.
class HttpRequest {
public:
    // 생성자에는 반환형이 없다. 세 값 매개변수를 받아 이 객체가 독립적으로 소유하게 한다.
    HttpRequest(std::string method, std::string url, std::vector<Header> headers)
        // std::move<T>(T&&)는 이름 있는 매개변수 lvalue를 xvalue로 바꾼다. 반환은 각 T&&이며 별도 객체를 만들지 않는다.
        // 실제 버퍼 소유권 이전은 string/vector 이동 생성자가 수행한다. 이동 뒤 매개변수는 유효하지만 값은 미지정이다.
        // 멤버 초기화 목록은 생성자 본문보다 먼저 세 private 멤버를 직접 초기화한다.
        : method_{std::move(method)}, url_{std::move(url)}, headers_{std::move(headers)} {}

    // const 참조 반환은 복사를 피하지만 반환 참조는 *this보다 오래 보관하면 안 된다.
    [[nodiscard]] const std::string& method() const noexcept { return method_; }
    [[nodiscard]] const std::string& url() const noexcept { return url_; }
    [[nodiscard]] const std::vector<Header>& headers() const noexcept { return headers_; }

private:
    std::string method_{};
    std::string url_{};
    std::vector<Header> headers_{};
};

// Builder는 변경 가능한 조립 단계와 완성된 불변에 가까운 값 객체 사이의 아키텍처 경계다.
class RequestBuilder {
public:
    // explicit은 std::string 하나가 RequestBuilder로 암시 변환되는 일을 막는다.
    // RequestBuilder builder{std::string{"/health"}}처럼 직접 초기화해야 의도가 드러난다.
    explicit RequestBuilder(std::string url)
        : url_{std::move(url)} {}

    // 뒤의 &는 *this가 lvalue일 때만 이 오버로드를 선택한다. 반환 참조는 같은 builder를 빌린다.
    RequestBuilder& header(std::string name, std::string value) & {
        add_header(std::move(name), std::move(value)); // 값 매개변수의 소유권을 내부 목록으로 전달한다.
        return *this; // *this는 항상 lvalue 식이므로 RequestBuilder&에 바인딩된다.
    }

    // 뒤의 &&는 임시 객체나 std::move(builder) 같은 xvalue 수신 객체에만 선택된다.
    RequestBuilder&& header(std::string name, std::string value) && {
        add_header(std::move(name), std::move(value));
        // std::move(*this)는 RequestBuilder lvalue 식을 RequestBuilder&& xvalue로 바꿔 다음 && 한정 호출을 유지한다.
        // 소유권은 아직 옮기지 않고 참조만 반환하며, 수신 임시 객체 수명은 원래 전체 식 끝까지다.
        return std::move(*this);
    }

    // const & build는 이름 있는 재사용 builder를 바꾸지 않고 현재 상태를 값으로 복사한다.
    [[nodiscard]] HttpRequest build() const & {
        // method_, url_, headers_는 const lvalue이므로 HttpRequest의 값 매개변수가 복사된다.
        // 반환 HttpRequest prvalue는 호출자의 목적 객체를 직접 초기화해 복사 생략될 수 있다.
        return HttpRequest{method_, url_, headers_};
    }

    // && build는 곧 수명이 끝날 builder의 저장소를 소비해 불필요한 깊은 복사를 피한다.
    [[nodiscard]] HttpRequest build() && {
        // 세 std::move 결과는 xvalue이고 HttpRequest 값 매개변수와 멤버로 순차 이동될 수 있다.
        // 호출 뒤 *this는 파괴 가능한 유효 상태지만 그 문자열·vector 내용은 미지정이다.
        return HttpRequest{std::move(method_), std::move(url_), std::move(headers_)};
    }

private:
    // private 보조 함수는 두 값 매개변수를 소비해 중복된 삽입 코드를 한곳에 둔다.
    void add_header(std::string name, std::string value) {
        // vector<Header>::push_back(Header&&)가 선택된다. 인자는 Header prvalue이며 두 문자열을 소유한다.
        // 반환형 void라 결과를 저장하지 않는다. 성공하면 headers_ 크기가 1 늘고 인자 자원은 새 원소로 이동된다.
        // 상각 O(1), 용량 부족 시 O(N) 재할당과 bad_alloc 가능성이 있고 기존 포인터·참조·반복자는 모두 무효화된다.
        headers_.push_back(Header{std::move(name), std::move(value)});
    }

    std::string method_{"GET"}; // string(const char*)가 세 문자를 복사 소유하며 할당 실패가 가능하다.
    std::string url_{};
    std::vector<Header> headers_{};
};

int main() {
    // 이름 있는 reusable은 lvalue이므로 header(...) &와 build() const &가 선택된다.
    // string(const char*, allocator 기본값)는 첫 인자 리터럴의 포인터를 받아 7개 문자를 복사 소유하고 두 번째 할당자는 기본값이다.
    // 생성된 string prvalue가 RequestBuilder의 url 값 매개변수로 이동되고 멤버가 다시 이동받는다. 할당 실패 시 bad_alloc이다.
    RequestBuilder reusable{std::string{"/health"}};
    // header의 1·2번 const char* 인자는 각각 string prvalue "Accept"와 "application/json"을 만든다.
    // 두 값 매개변수가 문자를 소유하고 add_header로 이동된 뒤 파괴된다. header 반환 RequestBuilder&는 build 수신에 사용한다.
    const HttpRequest snapshot{reusable.header("Accept", "application/json").build()};

    // RequestBuilder{...}는 prvalue다. header(...) &&가 같은 임시 객체를 xvalue로 돌려주고 build() &&가 자원을 이동한다.
    // 임시 builder는 세미콜론의 전체 식 끝에 파괴되지만 one_shot은 이동받은 문자열과 vector를 계속 소유한다.
    // "/orders", "Authorization", "Bearer token"은 각각 string(const char*)로 소유 prvalue가 된 뒤 해당 값 매개변수로 이동된다.
    // 문자열 생성은 각 길이에 선형이고 할당/bad_alloc 가능성이 있으며 원본 리터럴의 정적 수명과 내용은 유지된다.
    const HttpRequest one_shot{
        RequestBuilder{std::string{"/orders"}}.header("Authorization", "Bearer token").build()};

    // vector::size()의 대표 형태는 size_type size() const noexcept다. 수신 객체는 snapshot 내부의 const vector이고 인자는 없다.
    // 반환 원소 수를 출력에 사용하며 상태·용량·수명은 바뀌지 않고 O(1), 무할당·무예외다.
    const auto snapshot_header_count{snapshot.headers().size()};
    const auto one_shot_header_count{one_shot.headers().size()};

    // ostream operator<<는 각 값을 const 참조 또는 값으로 받아 std::cout 버퍼/상태를 갱신하고 ostream&를 반환한다.
    // 연쇄 반환 참조는 다음 삽입에 쓰고 최종 참조는 버린다. I/O 실패는 기본적으로 예외보다 상태 비트로 남는다.
    std::cout << snapshot.method() << ' ' << snapshot.url() << ' ' << snapshot_header_count << '\n';
    std::cout << one_shot.method() << ' ' << one_shot.url() << ' ' << one_shot_header_count << '\n';

    // && 경로가 만든 객체가 임시 builder와 독립적으로 유효한지 반환 코드로도 검증한다.
    return snapshot_header_count == 1U && one_shot_header_count == 1U ? 0 : 1;
}
