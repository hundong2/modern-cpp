# CTAD 

- [복사 비용을 줄이는 ranges를 활용한 CTAD 예제](./cpp20/CTAD.cpp). 
    - 주로 사용 되는 곳 
        1. 네트워크 패킷 제로카피(Zero-Copy) 파싱:
        서버 코어 로직에서 TCP로 들어온 바이트 버퍼(char[]나 vector<uint8_t>)를 복사 없이 암호화/압축 해제 모듈로 넘길 때 사용합니다. 사용자는 그냥 컨테이너를 던지기만 하면 되므로 실수가 원천 차단됩니다.

        2. 그래픽스(DirectX/Vulkan) 정점(Vertex) 데이터 전송:
        수십만 개의 3D 모델 좌표 데이터를 렌더링 파이프라인 클래스에 전달할 때, 호출부의 코드는 깔끔하게 유지하면서 내부적인 힙 할당(Heap Allocation)을 0으로 만듭니다.

        3. 범용 수학 / 통계 라이브러리 제작:
        동료 개발자들이 당신이 만든 StatAnalyzer를 사용할 때, 그들이 벡터를 쓰든, 어레이를 쓰든, 날것의 배열을 쓰든 단 하나의 인터페이스(StatAnalyzer{데이터})로 통일시킬 수 있습니다.


## History

- C++98/03/11/14의 암흑기 (make_ 함수의 시대):

함수 템플릿은 인자를 보고 타입을 추론할 수 있었지만, 클래스 템플릿은 그게 불가능했습니다.
그래서 std::pair<int, double> p(1, 3.14); 처럼 타입을 일일이 적어야 했습니다. 이게 너무 귀찮아서 사람들은 std::make_pair(1, 3.14); 같은 헬퍼(Helper) 함수를 만들어 우회했습니다. 수많은 make_tuple, make_shared, make_unique가 등장한 이유입니다.

- C++17의 혁명 (CTAD와 추론 가이드의 등장):

마침내 클래스 템플릿도 생성자에 들어온 값을 보고 타입을 눈치껏 추론(CTAD)할 수 있게 되었습니다.
이제 std::pair p(1, 3.14); 만 적어도 컴파일러가 알아서 <int, double>을 붙여줍니다.
하지만, 컴파일러가 바보같이 추론할 때가 있습니다. 이를 바로잡기 위해 개발자가 컴파일러에게 쥐여주는 '정답지'가 바로 추론 가이드(Deduction Guide)입니다.

- C++20/23의 진화 (Aggregate CTAD):

C++20부터는 구조체에 명시적인 생성자가 없어도(Aggregate), 중괄호 {} 초기화를 할 때 컴파일러가 타입을 추론할 수 있도록 더욱 똑똑해졌습니다.

## 대표적인 사용 예 

```
가장 대표적인 실무의 골칫거리가 '문자열 리터럴(String Literal)'입니다.  
"Hello"라는 글자를 템플릿에 넣으면, 컴파일러는 이를 std::string이 아니라 
const char[6](길이가 6인 문자 배열) 또는 const char*(포인터)로 바보같이 추론합니다. 
이대로 놔두면 메모리 릭이나 포인터 붕괴(Pointer Decay) 버그가 터집니다.
````

- [example code with iterator](./cpp17/CTAD_with_iterator.cpp).  
- `std::input_iterator` ( cpp20 concepts )
    - 템플릿은 아무 타입이나 다 받아버려서 문제(에러 폭탄)가 발생합니다. C++20부터는 "이 타입은 최소한 앞으로 읽고 나갈 수 있는 반복자여야만 해!"라고 컴파일러 단에서 입구 컷을 해버립니다. 가독성과 안정성이 비약적으로 상승합니다.

## reference 

- [CTAD code](./cpp17/CTAD.cpp). 
    - [CTAD Explain document](./cpp17/REAMDE.md). 
- [std::span](./std::span.md). 
