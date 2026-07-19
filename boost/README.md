# Boost 실무 커리큘럼

## Getting Start

```
./run_script.sh <file name>
```


참고 공식 문서:

- Boost: <https://www.boost.org/>
- Boost 1.91.0 릴리스: <https://www.boost.org/releases/>
- Boost 네트워킹 가이드: <https://docs.cppalliance.org/user-guide/task-networking.html>
- Boost.Beast: <https://www.boost.org/libs/beast>

Boost는 표준 라이브러리에 들어가기 전 실험적/실용적 기능을 제공해 온 C++
라이브러리 모음입니다. 실무에서는 “표준에 아직 없거나”, “표준보다 기능이 풍부하거나”,
“오래된 C++ 버전에서도 비슷한 기능을 써야 하는” 경우에 자주 등장합니다.

이 폴더는 오픈소스에서 자주 만나는 Boost 코드를 카테고리별로 읽을 수 있게 만든
커리큘럼입니다.

## 빌드 전제

Boost 헤더가 include path에 있어야 합니다. 일부 라이브러리는 별도 링크가 필요합니다.
현재 portable GCC 환경에는 Boost가 기본 포함되어 있지 않으므로, Boost를 별도로 설치하거나
`CMAKE_PREFIX_PATH`/`BOOST_ROOT` 등으로 위치를 알려주어야 합니다.

```powershell
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S boost -B boost/build
cmake --build boost/build
```

현재 저장소 환경에서는 C++ 컴파일러가 PATH에 잡혀 있지 않거나 Boost include 경로를 찾지 못하면
CMake configure 단계에서 실패합니다.

## 파일 리스트와 카테고리

- [01_algorithm_string.cpp](./01_algorithm_string.cpp)
  - 카테고리: 문자열 처리
  - `boost::algorithm::trim_copy`, `split`, `join`, `to_lower_copy`
  - 설정 파일, CSV 비슷한 텍스트, 로그 라인 전처리에 자주 보입니다.

- [02_lexical_cast_uuid.cpp](./02_lexical_cast_uuid.cpp)
  - 카테고리: 변환과 식별자
  - `boost::lexical_cast`, `boost::uuids`
  - 문자열 기반 설정값 변환, 요청 ID/트랜잭션 ID 생성에 자주 보입니다.

- [03_multi_index.cpp](./03_multi_index.cpp)
  - 카테고리: 컨테이너 설계
  - `boost::multi_index_container`
  - 하나의 저장소를 id, name 같은 여러 인덱스로 조회해야 할 때 사용합니다.

- [04_signals2_observer.cpp](./04_signals2_observer.cpp)
  - 카테고리: 이벤트/옵저버
  - `boost::signals2`
  - GUI, 상태 변경 알림, 플러그인 훅처럼 구독자에게 이벤트를 전달할 때 사용합니다.

- [05_asio_timer.cpp](./05_asio_timer.cpp)
  - 카테고리: 비동기 I/O 모델
  - `boost::asio::io_context`, `steady_timer`
  - 네트워크 서버/클라이언트 코드의 기본 실행 모델을 이해하는 입문 예제입니다.

- [06_beast_http_message.cpp](./06_beast_http_message.cpp)
  - 카테고리: HTTP/WebSocket
  - `boost::beast::http::request`, `response`
  - 실제 네트워크 연결 없이 HTTP 메시지 타입 구조를 읽는 예제입니다.

- [07_json_value.cpp](./07_json_value.cpp)
  - 카테고리: JSON
  - `boost::json::value`, `parse`, `serialize`
  - REST API, 설정, 메시지 payload 처리에 자주 등장합니다.

- [08_program_options_shape.cpp](./08_program_options_shape.cpp)
  - 카테고리: CLI/설정
  - `boost::program_options`
  - 별도 링크가 필요한 대표 라이브러리입니다. 커맨드라인 파싱 코드 형태를 이해합니다.

## 선택 기준

- 문자열 전처리: Boost.Algorithm.String
- 표준 컨테이너 하나로 조회 요구를 만족하기 어렵다: Boost.MultiIndex
- 이벤트 구독/해제가 필요하다: Boost.Signals2
- 네트워크/비동기 I/O: Boost.Asio
- HTTP/WebSocket: Boost.Beast
- JSON payload: Boost.JSON
- CLI 옵션/설정 파일: Boost.Program_options

Boost는 템플릿을 많이 사용하므로 컴파일 에러가 길 수 있습니다. 예제를 볼 때는
먼저 “이 타입이 무엇을 소유하는가”, “복사가 발생하는가”, “콜백이 언제 실행되는가”를
추적하면 오픈소스 코드를 읽기 쉬워집니다.

