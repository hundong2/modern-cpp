# 2026-09-09 CHECKPOINT

아래 질문은 코드를 다시 보지 않고 먼저 답한다. “대충 복사한다”처럼 끝내지 말고 **정확한 타입·값 범주·수신 객체 상태·인자·반환값 사용·호출 뒤 상태·복잡도·할당/무효화/수명·오류·스레드 보장**을 말해야 통과다.

## 1. C++ 기초 문법

- [ ] 1. `int score{};`와 `int score;`의 초기값 차이와 미초기화 값을 읽을 때의 위험을 설명한다.
- [ ] 2. 함수 선언에서 반환형, 함수 이름, 값 매개변수, 참조 매개변수, 뒤의 `const`를 각각 가리킨다.
- [ ] 3. `const Record& record`의 두 기호가 각각 금지하는 것과, 원본 수명을 늘리지 않는다는 사실을 말한다.
- [ ] 4. pointer가 null일 수 있지만 reference는 유효한 객체에 바인딩되어야 한다는 차이를 설명한다.
- [ ] 5. `if`, `for`, `while`, `continue`, 중괄호가 제어 흐름과 변수 수명 범위를 어떻게 만드는지 코드 예로 설명한다.
- [ ] 6. `struct Record`의 기본 접근과 `class RecordBook`의 기본 접근이 각각 무엇인지 말한다.
- [ ] 7. `public`/`private`가 이름 접근을 제어할 뿐 자동으로 메모리 배치나 실행 성능을 바꾸지 않는 이유를 설명한다.
- [ ] 8. 생성자에 반환형이 없는 이유와 `explicit`이 막는 암시 변환 경로를 예로 든다.
- [ ] 9. 멤버 초기화 목록이 생성자 본문보다 먼저 실행되고 멤버가 선언 순서로 초기화된다는 규칙을 설명한다.
- [ ] 10. `using Page = std::ranges::subrange<ConstIterator>;`가 새 타입을 정의하는지, `subrange`의 명시·생략 템플릿 인자가 무엇인지 답한다.

## 2. 값 범주·소유권·수명

- [ ] 11. 이름 있는 `original`, `independent_copy`, `page`가 lvalue인 이유를 말한다.
- [ ] 12. `PageSummary{count,total}`과 값 반환 함수 호출이 prvalue인 이유를 말한다.
- [ ] 13. `std::move(independent_copy)`가 xvalue를 만들 뿐 실제 vector 저장소 이동은 후속 생성자가 한다는 두 단계를 설명한다.
- [ ] 14. `RecordBook independent_copy{original}`에서 owner 깊은 복사가 원소·저장소·관찰자에 미치는 영향을 설명한다.
- [ ] 15. 이동 뒤 `independent_copy`에 대해 보장되는 “유효하지만 값 미지정”의 의미를 말하고, 안전한 연산 두 개와 하면 안 되는 가정을 든다.
- [ ] 16. 함수가 `return RecordBook{...};`로 같은 타입 값을 반환할 때 C++17 이후 목적 객체 직접 구성과 선택적 NRVO의 차이를 설명한다.
- [ ] 17. `RecordBook::Page`를 복사할 때 iterator 값과 실제 `Record` 원소 중 무엇이 복사되는지 답한다.
- [ ] 18. page가 함수 밖에서도 안전하려면 owner·재할당·erase·동시 수정에 어떤 수명/동기화 조건이 필요한지 모두 말한다.
- [ ] 19. `const_iterator`가 원본의 다른 non-const 별칭을 통한 mutation까지 차단하지 못하는 이유를 설명한다.
- [ ] 20. owner를 공유 소유 스마트 포인터로 묶는 설계와 오늘의 짧은 비소유 view 설계의 비용·책임 차이를 비교한다.

## 3. 실제 식 해석

각 식에 대해 타입, 값 범주, 선택되는 overload, 반환과 사용, 호출 뒤 상태를 순서대로 적는다.

- [ ] 21. `const std::size_t bounded_offset{offset < current_size ? offset : current_size};`
- [ ] 22. `records_.cbegin() + static_cast<Difference>(bounded_offset)`
- [ ] 23. `Page{first, last}`
- [ ] 24. `for (const Record& record : page)`가 개념적으로 요청하는 `begin`, `end`, 비교, 역참조, 증가 연산
- [ ] 25. `page.size()`가 가능한 이유와 `sized_sentinel_for`인 random-access iterator 쌍의 역할
- [ ] 26. `RecordBook independent_copy{original};`
- [ ] 27. `RecordBook moved_owner{std::move(independent_copy)};`
- [ ] 28. `std::cout << summary.count << ' ' << summary.total << ' ' << moved_owner.size() << '\n';`
- [ ] 29. `graph[static_cast<std::size_t>(first)].push_back(second);`
- [ ] 30. `std::vector<int> depth_frequency(static_cast<std::size_t>(target_distance + 1), 0);`

## 4. 표준 라이브러리 호출 계약

- [ ] 31. `std::ranges::subrange(first,last)`의 목적 객체 초기 상태, 대표 시그니처·템플릿 인자, 두 인자의 타입/값 범주/소유권/허용 관계, 반환 여부, 생성 뒤 상태, O(1)·할당·수명·오류·스레드 계약을 설명한다.
- [ ] 32. `subrange::begin()`과 `end()`가 원소 참조가 아니라 iterator/sentinel 값을 반환하며 끝 sentinel은 역참조하면 안 된다는 점을 설명한다.
- [ ] 33. `subrange::size()`의 반환형·의미, 사용 여부, 계산 복잡도와 가능한 전제조건을 설명한다.
- [ ] 34. `vector::cbegin()`/`size()`의 const overload, 반환형, 상태 변화, noexcept 여부와 무효화 조건을 구분한다.
- [ ] 35. `vector::operator[]`가 참조를 반환하는 방식, O(1), 범위 검사 부재와 범위 밖 미정의 동작을 설명한다.
- [ ] 36. `vector::reserve(n)`의 size/capacity 사후조건, 재할당, strong exception guarantee가 깨질 수 있는 일반 원소 조건, 관찰자 무효화를 설명한다.
- [ ] 37. `vector::push_back(value)`의 lvalue/rvalue overload, 반환 `void`, 분할 상환 O(1), 재할당과 예외를 설명한다.
- [ ] 38. `vector::empty()`/`back()`/`pop_back()` 각각의 반환과 사용, 빈 vector 전제조건, 원소 수명·iterator 무효화를 설명한다.
- [ ] 39. `ConstIterator::operator+(Difference)`가 허용하는 같은 배열의 `[begin,end]` 범위, 반환 iterator, O(1), owner 수명·재할당 무효화를 설명한다.
- [ ] 40. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`의 인자·반환·호출 순서 전제와 이후 C stdio/대화형 출력 주의를 설명한다.
- [ ] 41. `std::cin >> node_count >> target_distance`의 두 호출, 반환 stream reference 연쇄, 성공/실패 후 대상과 stream 상태를 설명한다.
- [ ] 42. `std::cout << answer << '\n'`의 long long 멤버 삽입과 char 비멤버 삽입을 구분하고, 각 반환 참조 사용 여부·오류·복잡도·스레드 보장을 설명한다.

## 5. 센트로이드 분할 알고리즘

- [ ] 43. 센트로이드의 정의를 “제거 뒤 가장 큰 활성 성분 크기”로 쓰고 그 값이 왜 절반 이하인지 설명한다.
- [ ] 44. 센트로이드를 지나는 쌍에 `dist(u,v)=dist(u,c)+dist(v,c)`가 성립하는 조건을 말한다.
- [ ] 45. `depth_frequency[x]`가 어느 정점들을 세는지 정확한 불변식을 적는다.
- [ ] 46. 한 자식 성분의 깊이를 **조회한 뒤 등록**해야 하는 이유를 같은 자식의 두 정점 반례로 설명한다.
- [ ] 47. `frequency[0]=1`이 센트로이드 자신과 다른 정점의 경로를 어떻게 세는지 예를 든다.
- [ ] 48. 임의의 정점 쌍이 “처음 갈라지는” 센트로이드 단계에서 정확히 한 번만 세어진다는 증명을 쓴다.
- [ ] 49. 제거된 센트로이드의 살아 있는 각 이웃을 다음 작업 root로 넣어도 컴포넌트가 중복되지 않는 이유를 설명한다.
- [ ] 50. 깊이가 `k`보다 큰 frame을 확장하지 않아도 되는 근거를 비음수 간선 거리로 설명한다.
- [ ] 51. 매 단계 `depth_frequency[0..k]` 전체를 초기화하면 왜 `O(nk)` 위험이 있고 touched depth만 되돌리면 어떤 합으로 줄어드는지 설명한다.
- [ ] 52. 정점 하나가 `O(log n)`개 활성 성분에만 포함되는 절반 감소식을 써서 전체 `O(n log n)`을 도출한다.
- [ ] 53. 답의 최댓값 `n(n-1)/2`를 `n=200,000`에 대입해 32-bit `int`가 부족한 이유를 보인다.
- [ ] 54. 반복형 component traversal·distance collection·pending stack이 일자 트리의 재귀 호출 스택 overflow를 피하는 이유를 설명한다.

## 6. 손으로 푸는 검증

1. 경로 `1-2-3-4-5-6`, `k=3`에서 가능한 세 쌍을 직접 쓴다.
2. 중심 1과 잎 2..6인 별, `k=2`에서 답이 `C(5,2)=10`임을 깊이 빈도로 재현한다.
3. 공식 예제에서 센트로이드 3을 먼저 잡았다고 가정하고 각 자식 방향의 깊이 목록과 누적 subtotal을 적는다.
4. `n=1,k=1`과 `n=5,k=5`의 답이 0인 이유를 트리 지름 상한으로 설명한다.
5. 작은 트리 하나에서 모든 정점 쌍의 BFS 거리를 직접 표로 만들고 프로그램 출력과 비교한다.

## 7. 실기 통과 기준

- [ ] `original.page(3,99)`를 호출하고 안전하게 끝에서 잘린 page의 출력 결과를 먼저 예측했다.
- [ ] page를 보관한 뒤 owner vector를 재할당하는 의도적 잘못된 코드를 작성하고 sanitizer/문서로 위험을 확인한 뒤 되돌렸다.
- [ ] ICPC 코드의 조회-후-등록 순서를 뒤집어 별/가지 반례 테스트가 실패함을 확인한 뒤 되돌렸다.
- [ ] 100개 이상의 작은 무작위 트리를 all-pairs BFS oracle과 대조했다.
- [ ] `n=200,000` 사슬과 별을 실행해 재귀 stack overflow가 없고 시간·메모리가 제한 안임을 확인했다.
- [ ] w64devkit 높은 경고 빌드, CTest, 알고리즘 문서 예제, UTF-8·링크·Mermaid, 표준 라이브러리 `latest`와 `all` 감사를 모두 통과했다.

## 최종 자기 설명

`subrange`를 “슬라이스”라고만 부르면 부족하다. **iterator/sentinel을 값으로 소유하지만 원소는 소유하지 않는 view**, 생성·관찰 비용, owner 수명과 vector 무효화, const가 막지 못하는 외부 mutation까지 설명해야 한다. 센트로이드 분할은 코드 없이도 절반 불변식, 조회-후-등록, 유일 담당 단계, touched 초기화, `O(n log n)`/`O(n)`과 반복형 stack 안전성을 다시 구성할 수 있어야 한다.
