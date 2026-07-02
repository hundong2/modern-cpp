#include <algorithm>
#include <cassert>
#include <expected>
#include <iostream>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// This example intentionally stays at C++23 so it can be built today with the
// installed GCC 16 toolchain. The README explains how this maps to the C++26
// reflection/out_ptr discussion from the briefing.

// -----------------------------------------------------------------------------
// 1. Legacy C-style API layer
// -----------------------------------------------------------------------------
// A C library often exposes plain structs and functions. It may allocate memory
// internally and return ownership through an output parameter such as T**.
struct LegacyPacket {
    int packet_id;
    const char* raw_payload;
};

// Simulated C API: returns 0 on success and writes a newly allocated object into
// out_packet. The caller is responsible for releasing it with legacy_network_free.
int legacy_network_recv(LegacyPacket** out_packet) {
    if (out_packet == nullptr) {
        return -1;
    }

    *out_packet = new LegacyPacket{7007, "COMPILER_LAB_METRIC_DATA"};
    return 0;
}

// Matching C API cleanup function. Real C libraries often require a specific
// release function instead of plain delete/free.
void legacy_network_free(LegacyPacket* packet) {
    delete packet;
}

// -----------------------------------------------------------------------------
// 2. RAII wrapper layer
// -----------------------------------------------------------------------------
// A custom deleter teaches std::unique_ptr how to release a resource that came
// from a C API. This is the practical C++23-compatible shape of the std::out_ptr
// idea from the briefing.
struct LegacyPacketDeleter {
    void operator()(LegacyPacket* packet) const noexcept {
        legacy_network_free(packet);
    }
};

using PacketOwner = std::unique_ptr<LegacyPacket, LegacyPacketDeleter>;

// std::expected<T, E> expresses either a success value T or an error value E.
// That is clearer than returning nullptr and asking every caller to guess why.
std::expected<PacketOwner, std::string> fetch_network_packet_safe() {
    LegacyPacket* raw_packet = nullptr;

    // A legacy C API writes the allocated pointer into raw_packet.
    const int result = legacy_network_recv(&raw_packet);
    if (result != 0 || raw_packet == nullptr) {
        return std::unexpected("legacy_network_recv failed");
    }

    // From this line onward, the resource is owned by a smart pointer. Even if
    // later code throws or returns early, the deleter will run automatically.
    return PacketOwner(raw_packet);
}

// -----------------------------------------------------------------------------
// 3. Logging/inspection layer
// -----------------------------------------------------------------------------
// std::string_view is a non-owning view of text. It avoids copying, but it must
// not outlive the original string data.
void print_packet(std::string_view channel, const LegacyPacket& packet) {
    std::cout << "[packet]\n";
    std::cout << "  channel    : " << channel << '\n';
    std::cout << "  packet_id  : " << packet.packet_id << '\n';
    std::cout << "  raw_payload: " << packet.raw_payload << '\n';
}

// Variadic templates accept any number of extra arguments. This C++23 version
// does not use C++26 pack indexing(args...[n]); instead it passes the target
// packet explicitly and keeps extra metadata extensible.
template <typename... Metadata>
void dispatch_to_broker(std::string_view channel,
                        const LegacyPacket& packet,
                        Metadata&&... metadata) {
    std::cout << "[broker] dispatch begin\n";
    print_packet(channel, packet);

    // Fold expression: expands the parameter pack and prints every metadata
    // value. For example, (print(a), print(b), print(c)).
    ((std::cout << "  metadata   : " << std::forward<Metadata>(metadata) << '\n'), ...);

    std::cout << "[broker] dispatch end\n";
}

// A service class groups workflow logic. In production code this boundary makes
// testing easier because callers depend on a small public API instead of raw C
// functions scattered everywhere.
class PacketGateway {
public:
    explicit PacketGateway(std::string channel) : channel_(std::move(channel)) {}

    std::expected<void, std::string> receive_and_dispatch() const {
        auto packet_result = fetch_network_packet_safe();
        if (!packet_result) {
            return std::unexpected(packet_result.error());
        }

        // std::move transfers unique_ptr ownership out of expected. Only one
        // owner is allowed, which prevents double-free bugs.
        PacketOwner packet = std::move(packet_result.value());
        dispatch_to_broker(channel_, *packet, "source=legacy-c-api", "standard=c++23");
        return {};
    }

private:
    std::string channel_;
};

// -----------------------------------------------------------------------------
// 4. Coding test algorithm: shortest subarray with sum >= target
// -----------------------------------------------------------------------------
// std::span is a lightweight view over contiguous elements. It lets the function
// accept vectors, arrays, or other contiguous buffers without copying them.
int min_subarray_len(int target, std::span<const int> nums) {
    int left = 0;
    int current_sum = 0;
    int min_length = std::numeric_limits<int>::max();

    for (int right = 0; right < static_cast<int>(nums.size()); ++right) {
        current_sum += nums[right];

        while (current_sum >= target) {
            min_length = std::min(min_length, right - left + 1);
            current_sum -= nums[left];
            ++left;
        }
    }

    return min_length == std::numeric_limits<int>::max() ? 0 : min_length;
}

void run_tests() {
    {
        const std::vector<int> nums{2, 3, 1, 2, 4, 3};
        assert(min_subarray_len(7, nums) == 2);
    }
    {
        const std::vector<int> nums{1, 1, 1, 1, 1, 1, 1, 1};
        assert(min_subarray_len(11, nums) == 0);
    }
    {
        const std::vector<int> nums{1, 4, 4};
        assert(min_subarray_len(4, nums) == 1);
    }
}

int main(int argc, char* argv[]) {
    run_tests();

    PacketGateway gateway("NIC_CHANNEL_0");
    const auto dispatch_result = gateway.receive_and_dispatch();
    if (!dispatch_result) {
        std::cerr << "[error] " << dispatch_result.error() << '\n';
        return 1;
    }

    const std::vector<int> nums{2, 3, 1, 2, 4, 3};
    const int target = argc >= 2 ? std::stoi(argv[1]) : 7;
    std::cout << "[algorithm]\n";
    std::cout << "  target     : " << target << '\n';
    std::cout << "  min length : " << min_subarray_len(target, nums) << '\n';

    std::cout << "[tests] all tests passed\n";
    return 0;
}
