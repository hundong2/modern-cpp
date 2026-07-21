#include <algorithm> // std::copy로 입력 바이트를 소유 버퍼에 복사하기 위해 포함한다.
#include <atomic> // 여러 스레드가 공유하는 완료 횟수를 데이터 레이스 없이 갱신한다.
#include <cstddef> // 바이트 크기를 표현하는 std::size_t와 std::byte를 제공한다.
#include <cstdint> // 체크섬과 카운터 크기를 고정하는 std::uint64_t를 제공한다.
#include <iostream> // 검증 결과를 표준 출력 스트림에 기록한다.
#include <memory> // unique_ptr, shared_ptr와 C++17 원자적 shared_ptr 연산을 제공한다.
#include <mutex> // 여러 스레드의 로그 한 줄이 서로 섞이지 않도록 직렬화한다.
#include <stdexcept> // 클래스 불변식 위반을 std::invalid_argument로 보고한다.
#include <string> // 설정 이름과 로그 메시지를 수명 안전하게 소유한다.
#include <string_view> // 입력 문자열을 복사하지 않는 읽기 전용 비소유 뷰로 전달한다.
#include <thread> // 작업을 여러 OS 스레드에서 실행해 동시성 규칙을 관찰한다.
#include <utility> // 값 인자의 자원을 멤버로 넘기는 std::move를 제공한다.
#include <vector> // 생성한 작업 스레드의 수명과 join을 한 컨테이너에서 관리한다.

class ServiceConfig final { // 상속 확장보다 값 검증을 우선하는 불변 설정 타입을 정의한다.
public: // 검증된 설정을 만드는 생성자와 읽기 전용 관찰 함수를 공개한다.
    explicit ServiceConfig(std::string version, const std::size_t maximum_payload) // 한 인자 변환을 막고 크기를 값으로 받는다.
        : version_{std::move(version)}, maximum_payload_{maximum_payload} { // 멤버 선언 순서대로 즉시 생성해 대입 단계를 없앤다.
        if (version_.empty()) { // 비어 있는 버전이 객체 불변식을 깨는지 생성 시점에 검사한다.
            throw std::invalid_argument{"version must not be empty"}; // 잘못된 객체가 외부에 보이기 전에 생성을 실패시킨다.
        } // 버전 검사 분기를 끝낸다.
        if (maximum_payload_ == 0U) { // 0바이트 한도가 모든 요청을 거부하므로 잘못된 설정으로 본다.
            throw std::invalid_argument{"maximum payload must be positive"}; // 호출자에게 구체적인 계약 위반을 전달한다.
        } // 크기 검사 분기를 끝낸다.
    } // 모든 멤버가 유효한 상태일 때만 생성자를 정상 종료한다.

    ServiceConfig(const ServiceConfig&) = delete; // 설정 스냅샷의 의도치 않은 값 복사를 컴파일 타임에 금지한다.
    ServiceConfig& operator=(const ServiceConfig&) = delete; // 이미 공개된 설정의 덮어쓰기도 금지해 불변성을 지킨다.
    ServiceConfig(ServiceConfig&&) noexcept = default; // 새 객체로 소유권을 넘기는 이동은 멤버 이동에 위임한다.
    ServiceConfig& operator=(ServiceConfig&&) noexcept = default; // 이동 대입 역시 표준 멤버 동작을 사용한다.

    [[nodiscard]] const std::string& version() const noexcept { return version_; } // 소유권 없이 문자열을 읽고 객체 상태를 바꾸지 않는다.
    [[nodiscard]] std::size_t maximum_payload() const noexcept { return maximum_payload_; } // 레지스터 크기의 정숫값을 저렴하게 반환한다.

private: // 외부 코드가 검증을 우회하여 멤버를 수정하지 못하게 숨긴다.
    std::string version_; // 설정 스냅샷이 버전 문자의 힙 저장소를 독점 소유한다.
    std::size_t maximum_payload_; // 허용할 최대 바이트 수를 객체 내부 값으로 보관한다.
}; // ServiceConfig 타입 정의를 끝낸다.

class PacketBuffer final { // 동적 배열의 단일 소유권과 크기를 함께 묶는 RAII 타입을 정의한다.
public: // 생성, 이동, 바이트 접근에 필요한 최소 인터페이스만 공개한다.
    explicit PacketBuffer(const std::size_t size) // 정수가 버퍼로 암시 변환되지 않도록 explicit을 사용한다.
        : data_{std::make_unique<std::byte[]>(size)}, size_{size} { // 배열을 한 번 할당하고 unique_ptr이 해제를 책임지게 한다.
    } // 생성자 본문에는 사후 대입이 없으며 멤버는 이미 완성된 상태다.

    ~PacketBuffer() = default; // unique_ptr 소멸자가 delete[]를 호출하므로 수동 해제 코드는 필요 없다.
    PacketBuffer(const PacketBuffer&) = delete; // 하나의 배열을 두 객체가 독점 소유하는 모순을 컴파일 타임에 막는다.
    PacketBuffer& operator=(const PacketBuffer&) = delete; // 복사 대입으로 인한 이중 소유와 이중 해제를 방지한다.

    PacketBuffer(PacketBuffer&& other) noexcept // 임시 객체나 명시적으로 이동된 객체에서 소유권을 받는다.
        : data_{std::move(other.data_)}, size_{other.size_} { // unique_ptr 내부 주소와 정수 크기만 이전한다.
        other.size_ = 0U; // 이동된 원본도 소멸 및 size 호출이 가능한 명시적 빈 상태로 만든다.
    } // 새 힙 할당이나 바이트 단위 복사 없이 이동 생성을 끝낸다.

    PacketBuffer& operator=(PacketBuffer&& other) noexcept { // 기존 배열을 정리하면서 새 배열 소유권을 받을 수 있게 한다.
        if (this != &other) { // 자기 자신을 이동 대입하는 드문 경우에도 상태를 보존한다.
            data_ = std::move(other.data_); // 기존 배열은 unique_ptr 대입 과정에서 먼저 해제되고 새 주소가 이전된다.
            size_ = other.size_; // 새 주소에 대응하는 논리적 바이트 크기를 복사한다.
            other.size_ = 0U; // 원본을 다시 비어 있는 유효 상태로 만든다.
        } // 자기 대입 보호 분기를 끝낸다.
        return *this; // 연쇄 대입 관례에 맞게 현재 객체의 lvalue 참조를 반환한다.
    } // 이동 대입 연산을 끝낸다.

    [[nodiscard]] std::byte* data() noexcept { return data_.get(); } // 소유권을 넘기지 않고 쓰기 가능한 시작 주소만 관찰한다.
    [[nodiscard]] const std::byte* data() const noexcept { return data_.get(); } // const 객체에는 읽기 전용 시작 주소를 제공한다.
    [[nodiscard]] std::size_t size() const noexcept { return size_; } // 배열 범위를 검사할 때 쓸 크기를 값으로 반환한다.

private: // 주소와 크기가 항상 함께 이동되도록 표현 세부를 숨긴다.
    std::unique_ptr<std::byte[]> data_; // 배열 주소를 유일하게 소유하며 스코프 종료 시 자동 해제한다.
    std::size_t size_{0U}; // 이동된 객체까지 안전하게 표현하기 위해 0으로 기본 초기화한다.
}; // PacketBuffer 타입 정의를 끝낸다.

class Logger { // 런타임에 구현을 교체할 수 있는 동적 다형성 경계를 정의한다.
public: // 파생 구현을 인터페이스 포인터로 안전하게 파괴하고 호출할 함수를 공개한다.
    virtual ~Logger() = default; // 기반 포인터로 파생 객체를 지울 때 전체 소멸자 체인을 실행한다.
    virtual void write(std::string_view message) = 0; // 실제 출력 정책을 파생 클래스가 제공하도록 순수 가상 함수로 둔다.
}; // Logger 인터페이스 정의를 끝낸다.

class ConsoleLogger final : public Logger { // 더 파생되지 않는 스레드 안전 콘솔 구현을 제공한다.
public: // Logger 계약을 구현하는 단일 연산을 공개한다.
    void write(const std::string_view message) override { // override로 시그니처 불일치를 컴파일러가 검사하게 한다.
        const std::lock_guard<std::mutex> lock{mutex_}; // 함수가 끝날 때 자동 unlock되는 RAII 잠금으로 출력 임계 구역을 보호한다.
        std::cout << message << '\n'; // 잠금 안에서 메시지와 줄바꿈을 하나의 논리적 로그로 출력한다.
    } // lock_guard가 역순으로 소멸하면서 mutex를 자동으로 해제한다.

private: // 호출자가 동기화 객체 자체를 조작하지 못하게 숨긴다.
    std::mutex mutex_; // 운영체제 또는 런타임 동기화 프리미티브를 감싸 공유 스트림 접근을 직렬화한다.
}; // ConsoleLogger 구현 정의를 끝낸다.

template <typename Derived> // 구체 처리기 타입을 컴파일 타임 매개변수로 받는다.
class ByteProcessor { // vtable 없이 공통 진입점을 제공하는 CRTP 기반 클래스를 정의한다.
public: // 구체 타입에 위임하는 비가상 함수를 호출자에게 제공한다.
    [[nodiscard]] std::uint64_t process(const PacketBuffer& packet) const noexcept { // 읽기 전용 패킷으로 체크섬을 계산한다.
        return static_cast<const Derived&>(*this).do_process(packet); // 정적 타입이 확정된 파생 구현을 직접 호출해 인라인 후보로 만든다.
    } // 파생 구현이 계산한 고정 폭 결과를 반환한다.
}; // CRTP 기반 클래스 정의를 끝낸다.

class AdditiveChecksum final : public ByteProcessor<AdditiveChecksum> { // 각 바이트 합을 구하는 구체 정적 정책을 정의한다.
public: // CRTP 기반 클래스가 호출할 구현 함수를 공개한다.
    [[nodiscard]] std::uint64_t do_process(const PacketBuffer& packet) const noexcept { // 패킷을 수정하지 않고 순차 순회한다.
        std::uint64_t sum{0U}; // 오버플로 여유가 큰 64비트 누산기를 레지스터 후보로 초기화한다.
        for (std::size_t index{0U}; index < packet.size(); ++index) { // 유효한 [0, size) 주소만 접근한다.
            sum += std::to_integer<unsigned int>(packet.data()[index]); // 바이트를 정수로 변환해 누산 load/add 연산을 수행한다.
        } // 모든 바이트의 순차 접근을 끝내며 캐시 지역성을 활용한다.
        return sum; // 계산된 체크섬을 일반적으로 반환값 레지스터를 통해 호출자에게 전달한다.
    } // 체크섬 계산 함수를 끝낸다.
}; // AdditiveChecksum 정책 정의를 끝낸다.

class ProcessingService final { // 여섯 개념 중 자원, 정책, 동시성을 한 실무형 객체로 통합한다.
public: // 의존성 주입, 핫 리로드, 요청 처리, 통계 출력을 위한 API를 공개한다.
    ProcessingService(std::shared_ptr<const ServiceConfig> config, std::shared_ptr<Logger> logger) // 공유 수명이 필요한 의존성을 값으로 받는다.
        : config_{std::move(config)}, logger_{std::move(logger)} { // shared_ptr 제어 블록 소유권을 멤버로 이동해 불필요한 증감을 줄인다.
        if (!config_ || !logger_) { // null 의존성이 나중에 역참조되는 오류를 객체 경계에서 차단한다.
            throw std::invalid_argument{"service dependencies must not be null"}; // 불완전한 서비스 생성을 즉시 실패시킨다.
        } // 의존성 검사를 끝낸다.
    } // 유효한 설정과 로거를 보유한 뒤 생성자를 끝낸다.

    ProcessingService(const ProcessingService&) = delete; // mutex와 원자 카운터가 있는 서비스의 모호한 복사를 금지한다.
    ProcessingService& operator=(const ProcessingService&) = delete; // 실행 중 서비스 상태를 복사 대입하지 못하게 한다.

    void reload(std::shared_ptr<const ServiceConfig> next) { // 새로운 불변 설정 스냅샷을 다른 스레드에 게시한다.
        if (!next) { // null 게시가 모든 reader의 안전을 깨므로 먼저 검사한다.
            throw std::invalid_argument{"next config must not be null"}; // 잘못된 핫 리로드 요청을 호출자에게 알린다.
        } // null 검사를 끝낸다.
        std::atomic_store_explicit(&config_, std::move(next), std::memory_order_release); // C++17 API로 shared_ptr 값과 소유권을 원자적으로 교체한다.
    } // release 게시가 앞선 설정 초기화를 reader에게 보이게 한 뒤 함수를 끝낸다.

    [[nodiscard]] std::uint64_t handle(const std::string_view input) { // 문자열 뷰의 바이트를 처리하고 체크섬을 반환한다.
        const auto snapshot = std::atomic_load_explicit(&config_, std::memory_order_acquire); // 한 요청 동안 살아 있는 일관된 설정 스냅샷을 잡는다.
        if (input.size() > snapshot->maximum_payload()) { // 현재 스냅샷의 정책보다 큰 요청인지 검사한다.
            throw std::length_error{"payload exceeds current configuration"}; // 버퍼 할당 전에 과대 요청을 거부한다.
        } // 요청 크기 검사를 끝낸다.
        PacketBuffer packet{input.size()}; // 검증된 크기의 힙 배열을 RAII 객체로 한 번 할당한다.
        std::transform(input.begin(), input.end(), packet.data(), [](const char value) noexcept { // char 입력을 명시적으로 byte 출력으로 변환한다.
            return static_cast<std::byte>(static_cast<unsigned char>(value)); // 음수 char의 부호 확장을 피해 원래 8비트 패턴을 보존한다.
        }); // 모든 입력 문자를 소유 버퍼로 복사하고 변환을 끝낸다.
        const auto checksum = checksum_.process(packet); // CRTP 정적 디스패치로 바이트 체크섬을 계산한다.
        completed_.fetch_add(1U, std::memory_order_relaxed); // 순서 동기화 없이 분실되지 않는 통계 증가만 원자적으로 수행한다.
        return checksum; // 지역 PacketBuffer가 해제되기 전에 계산된 값만 호출자에게 돌려준다.
    } // packet 소멸자가 배열을 자동 해제하고 요청 처리를 끝낸다.

    void report() { // 처리 결과를 동적 로거 구현을 통해 한 번 출력한다.
        const auto snapshot = std::atomic_load_explicit(&config_, std::memory_order_acquire); // 보고 시점에도 설정 수명을 안전하게 고정한다.
        const auto count = completed_.load(std::memory_order_relaxed); // 다른 데이터와의 순서 제약 없이 정확한 카운터 값을 읽는다.
        logger_->write("version=" + snapshot->version() + ", completed=" + std::to_string(count)); // virtual 경계를 통해 실제 로그 목적지에 메시지를 전달한다.
    } // 보고에 사용한 shared_ptr 지역 복사본이 제어 블록 강한 참조를 하나 내린다.

private: // 동시성 규칙을 우회하지 못하도록 상태를 외부에서 숨긴다.
    std::shared_ptr<const ServiceConfig> config_; // C++17 atomic_load/store로만 접근하는 불변 설정 스냅샷이다.
    std::shared_ptr<Logger> logger_; // 서비스와 호출자가 로거 수명을 공동 소유하되 구현은 런타임 교체 가능하다.
    AdditiveChecksum checksum_; // 빈 정적 정책 객체이며 vptr이나 가상 호출이 필요 없다.
    std::atomic<std::uint64_t> completed_{0U}; // 여러 작업 스레드가 동시에 증가시키는 통계를 원자 객체로 저장한다.
}; // ProcessingService 타입 정의를 끝낸다.

int main() { // 운영체제가 프로세스를 시작한 뒤 호출하는 프로그램 진입점을 정의한다.
    auto config = std::make_shared<const ServiceConfig>("v1", 1024U); // 객체와 제어 블록을 보통 한 할당에 만들고 불변 공유 소유권을 얻는다.
    auto logger = std::make_shared<ConsoleLogger>(); // 구체 로거를 shared_ptr로 생성해 서비스와 수명을 공유한다.
    ProcessingService service{config, logger}; // 검증된 의존성을 주입해 처리 서비스를 생성한다.
    constexpr std::size_t worker_count{4U}; // 예제를 재현 가능하게 실행할 고정 작업자 수를 컴파일 타임 상수로 둔다.
    constexpr std::size_t requests_per_worker{250U}; // 각 스레드가 수행할 요청 수를 정한다.
    std::vector<std::thread> workers; // join 가능한 스레드 객체들의 소유 컨테이너를 만든다.
    workers.reserve(worker_count); // 재할당 없이 스레드 핸들을 넣도록 필요한 용량을 미리 확보한다.
    std::atomic<std::uint64_t> checksum_total{0U}; // 여러 스레드가 계산한 값을 손실 없이 합칠 원자 누산기를 만든다.
    for (std::size_t worker{0U}; worker < worker_count; ++worker) { // 요청을 병렬 실행할 작업자 수만큼 반복한다.
        workers.emplace_back([&service, &checksum_total]() { // 서비스와 누산기의 비소유 참조를 캡처하며 main이 join까지 수명을 보장한다.
            for (std::size_t request{0U}; request < requests_per_worker; ++request) { // 한 작업자가 맡은 요청 수만큼 반복한다.
                const auto checksum = service.handle("abc"); // 공유 서비스에서 독립 버퍼를 만들고 97+98+99를 계산한다.
                checksum_total.fetch_add(checksum, std::memory_order_relaxed); // 합계 자체의 원자성만 필요하므로 relaxed로 누적한다.
            } // 현재 작업자의 모든 요청 처리를 끝낸다.
        }); // 람다를 새 OS 스레드의 진입 함수로 전달한다.
    } // 모든 작업 스레드 생성을 끝낸다.
    std::shared_ptr<const ServiceConfig> next = std::make_shared<const ServiceConfig>("v2", 2048U); // 완전히 초기화된 새 스냅샷을 준비한다.
    service.reload(std::move(next)); // reader가 v1 또는 v2 중 완전한 한 객체만 보도록 원자 게시한다.
    for (auto& worker : workers) { // 소유한 모든 스레드 핸들을 순회한다.
        worker.join(); // 각 OS 스레드 종료를 기다리며 이후 통계 읽기와 happens-before 관계를 만든다.
    } // 모든 작업자 join을 끝낸다.
    service.report(); // 동적 다형성 로거를 통해 최종 버전과 정확한 요청 수를 출력한다.
    std::cout << "checksum=" << checksum_total.load(std::memory_order_relaxed) << '\n'; // 294000이라는 결정적 누산 결과를 확인한다.
    return 0; // 성공 상태를 운영체제에 반환하며 지역 RAII 객체를 역순으로 소멸시킨다.
} // main 스택 프레임의 수명과 프로그램 실행을 끝낸다.
