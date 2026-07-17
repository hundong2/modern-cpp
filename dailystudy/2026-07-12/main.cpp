/*
Practice: Modern Loop Style

This example avoids manual index-heavy loops where a range or algorithm makes
the intent clearer. The code stays C++23-compatible.
*/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

struct NetworkPacket {
    int id;
    std::string_view data;
    bool is_corrupted;
};

std::vector<NetworkPacket> receive_network_stream() {
    return {
        {1, "SYS_OK", false},
        {2, "SYS_FAIL", true},
        {3, "SYS_READY", false}
    };
}

int process_modern_loops() {
    std::cout << "[MODERN LOOP LOG]\n";

    int valid_count = 0;
    for (const NetworkPacket& packet : receive_network_stream()) {
        if (!packet.is_corrupted) {
            ++valid_count;
            std::cout << "  valid packet id: " << packet.id
                      << ", count: " << valid_count << '\n';
        }
    }

    std::vector<std::string_view> logs{"INFO_A", "ERROR_B", "INFO_C", "FATAL_D"};
    auto errors = logs | std::views::filter([](std::string_view log) {
        return log.starts_with("ERROR") || log.starts_with("FATAL");
    });

    for (std::string_view error : errors) {
        std::cout << "  detected error: " << error << '\n';
    }

    return valid_count;
}

void hardware_dispatch(std::vector<NetworkPacket>& packets) {
    std::ranges::for_each(packets, [](NetworkPacket& packet) {
        if (!packet.is_corrupted) {
            packet.data = "DISPATCHED";
        }
    });
}

void run_tests() {
    std::vector<NetworkPacket> packets{{101, "RAW", false}, {102, "RAW", true}};
    hardware_dispatch(packets);
    assert(packets[0].data == "DISPATCHED");
    assert(packets[1].data == "RAW");
}

int main() {
    run_tests();

    const int valid_count = process_modern_loops();
    std::vector<NetworkPacket> hardware_buffer{
        {101, "RAW", false},
        {102, "RAW", false},
        {103, "RAW", false}
    };
    hardware_dispatch(hardware_buffer);

    std::cout << "[RESULT] valid stream packets: " << valid_count << '\n';
    std::cout << "[RESULT] first hardware packet: " << hardware_buffer.front().data << '\n';
    std::cout << "[TESTS] modern loop tests passed\n";
    return 0;
}

/*
Execution result:
[MODERN LOOP LOG]
  valid packet id: 1, count: 1
  valid packet id: 3, count: 2
  detected error: ERROR_B
  detected error: FATAL_D
[RESULT] valid stream packets: 2
[RESULT] first hardware packet: DISPATCHED
[TESTS] modern loop tests passed
*/
