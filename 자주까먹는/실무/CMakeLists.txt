cmake_minimum_required(VERSION 3.20) # C++ 표준 기능과 테스트 설정에 필요한 최소 CMake 버전이다.
project(practical_cpp_guide LANGUAGES CXX) # C++만 사용하는 독립 실습 프로젝트를 선언한다.

set(CMAKE_CXX_STANDARD 17) # 기본 예제가 요구하는 최소 언어 버전을 C++17로 정한다.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 더 낮은 표준으로 조용히 대체하지 못하게 한다.
set(CMAKE_CXX_EXTENSIONS OFF) # 컴파일러 고유 gnu++ 확장 대신 표준 모드를 요청한다.

if(MSVC) # Microsoft 컴파일러일 때 경고 옵션 문법을 선택한다.
  add_compile_options(/W4 /permissive-) # 높은 경고와 더 엄격한 표준 준수를 요청한다.
else() # GCC 또는 Clang 계열 컴파일러 경로다.
  add_compile_options(-Wall -Wextra -Wpedantic) # 흔한 오류와 비표준 코드를 경고한다.
endif() # 컴파일러별 옵션 선택을 끝낸다.

set(PORTABLE_EXAMPLES # 운영체제나 외부 SDK 없이 빌드되는 예제 목록을 시작한다.
  example.cpp
  exercise.cpp
  examples/day01_templates_smart_ptr.cpp
  examples/day01b_value_move.cpp
  examples/day01c_inline_constexpr.cpp
  examples/day03_threaded_counter.cpp
  examples/day06_packet_framing.cpp
  examples/day07_layered_server.cpp
  examples/day08_thread_pool.cpp
  examples/day09_future.cpp
  examples/day10_asio_lifetime.cpp
  examples/day11_strand_model.cpp
  examples/day13_protobuf_model.cpp
  examples/day14_redis_model.cpp
  examples/day15_mysql_model.cpp
  examples/day16_packet_pipeline.cpp
  examples/day17_false_sharing.cpp
  examples/day18_udp_model.cpp
  examples/day19_object_pool.cpp
  examples/day20_cas_stack_model.cpp
  examples/day21_game_loop.cpp
  examples/day22_grid_aoi.cpp
  examples/day23_interpolation.cpp
) # portable 목록을 끝낸다.

foreach(source IN LISTS PORTABLE_EXAMPLES) # 각 소스 파일마다 독립 실행 파일을 만든다.
  get_filename_component(target "${source}" NAME_WE) # 디렉터리와 확장자를 떼어 target 이름을 만든다.
  add_executable("${target}" "${source}") # 해당 예제를 하나의 실행 파일로 등록한다.
  list(APPEND PORTABLE_TARGETS "${target}") # 뒤의 smoke test에서 실행할 target 이름을 보관한다.
endforeach() # portable target 생성을 끝낸다.

find_package(Threads REQUIRED) # std::thread가 요구할 플랫폼 thread 라이브러리를 찾는다.
foreach(target day03_threaded_counter day08_thread_pool day09_future day11_strand_model day15_mysql_model day17_false_sharing) # thread 사용 target을 고른다.
  target_link_libraries("${target}" PRIVATE Threads::Threads) # 선택된 target에 thread 라이브러리를 연결한다.
endforeach() # thread 연결 설정을 끝낸다.

add_executable(day12_coroutine_model examples/day12_coroutine_model.cpp) # C++20 coroutine 예제를 별도 target으로 만든다.
set_property(TARGET day12_coroutine_model PROPERTY CXX_STANDARD 20) # 이 target만 C++20으로 올린다.

option(BUILD_POSIX_NETWORK "Build Linux/POSIX socket examples" OFF) # 기본 Windows 빌드에서는 POSIX 예제를 제외한다.
if(BUILD_POSIX_NETWORK) # 사용자가 Linux/WSL에서 명시적으로 켰는지 확인한다.
  if(NOT UNIX) # POSIX API가 없는 플랫폼에서 잘못 켠 경우다.
    message(FATAL_ERROR "BUILD_POSIX_NETWORK requires Linux or another POSIX system") # 명확한 구성 오류를 낸다.
  endif() # UNIX 검사 끝이다.
  foreach(source day02_tcp_server_posix day02_tcp_client_posix day04_select_model day05_epoll_model) # POSIX 예제 이름을 순회한다.
    add_executable("${source}" "examples/${source}.cpp") # 각 POSIX 소스를 실행 파일로 만든다.
  endforeach() # POSIX target 생성을 끝낸다.
endif() # 선택 빌드 블록을 끝낸다.

enable_testing() # CTest 사용을 켠다.
foreach(target IN LISTS PORTABLE_TARGETS) # 모든 표준 C++ 예제를 실제로 한 번씩 실행한다.
  add_test(NAME "smoke_${target}" COMMAND "${target}") # 각 예제의 종료 코드 기반 자체 검사를 등록한다.
endforeach() # portable smoke test 등록을 끝낸다.
add_test(NAME smoke_day12_coroutine_model COMMAND day12_coroutine_model) # 별도 C++20 예제를 실행 검사한다.
if(BUILD_POSIX_NETWORK) # Linux 전용 readiness 모형이 빌드된 경우다.
  add_test(NAME smoke_day04_select_model COMMAND day04_select_model) # 외부 연결 없이 select 모형을 검사한다.
  add_test(NAME smoke_day05_epoll_model COMMAND day05_epoll_model) # 외부 연결 없이 epoll drain을 검사한다.
endif() # POSIX smoke test 등록을 끝낸다.
