// <iostream>은 결과를 화면에 쓰는 std::cout과 << 연산자를 제공한다.
#include <iostream>
// <ranges>는 C++20 std::views::filter와 std::views::transform을 제공한다.
#include <ranges>
// <string>은 문자열을 소유하는 std::string 타입을 제공한다.
#include <string>
// <utility>는 이동을 요청하는 std::move를 제공한다.
#include <utility>
// <vector>는 같은 타입 원소 여러 개를 소유하는 std::vector를 제공한다.
#include <vector>

// struct의 기본 접근은 public이며 상품 데이터를 단순히 묶는다.
struct Product {
    std::string name{}; // 문자열 멤버가 상품 이름의 수명과 메모리를 소유한다.
    int price{};        // 기본 정수 타입 멤버를 중괄호로 0 초기화한다.
};

// using 별칭에서 <Product>는 vector에 저장할 원소 타입인 템플릿 인자다.
using Catalog = std::vector<Product>;

class CatalogService {
public:
    // 생성자는 반환형이 없다. explicit은 Catalog 하나가 서비스로 암시적 변환되는 일을 막는다.
    explicit CatalogService(Catalog products)
        : products_{std::move(products)} { // 값 매개변수를 xvalue로 바꿔 소유권을 멤버로 옮긴다.
    }

    // 반환형 auto는 조합된 뷰 타입을 추론하며, int limit은 가격 상한 매개변수다.
    [[nodiscard]] auto affordable_names(int limit) const {
        // TODO 학습 핵심: 아래 파이프를 직접 다시 작성해 본다.
        return products_
             // filter는 const Product& lvalue 참조로 원소를 읽고 <= 비교 결과로 통과 여부를 정한다.
             | std::views::filter([limit](const Product& product) {
                   return product.price <= limit;
               })
             // transform은 통과한 상품에서 이름에 대한 const 참조를 투영해 문자열 복사를 피한다.
             | std::views::transform([](const Product& product) -> const std::string& {
                   return product.name;
               });
    }

private:
    Catalog products_{}; // class의 private 소유 멤버라 외부가 불변식을 직접 깨뜨릴 수 없다.
};

// int 반환형 main은 매개변수 없이 호출되는 프로그램 진입 함수다.
int main() {
    // Catalog{...}는 prvalue이고 service의 값 매개변수 및 멤버로 이동될 수 있다.
    const CatalogService service{Catalog{
        Product{"book", 1200},
        Product{"pen", 500},
        Product{"bag", 3000},
    }};

    // 이름 있는 names는 lvalue인 비소유 뷰이며 service보다 먼저 파괴되어 안전하다.
    auto names{service.affordable_names(1500)};
    int count{0};

    // const std::string&는 뷰가 가리키는 기존 문자열 lvalue에 바인딩되어 복사하지 않는다.
    for (const std::string& name : names) {
        std::cout << name << '\n'; // 표준 라이브러리 출력과 << 연산자를 실제로 호출한다.
        ++count;                   // 전위 ++ 연산자가 로드한 값을 증가시켜 다시 저장하는 효과를 낸다.
    }

    // 비교 결과 bool에 따라 성공/실패 반환 경로로 조건 분기한다.
    if (count == 2) {
        return 0;
    }
    return 1;
}
