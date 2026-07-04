#include <iostream>
#include <optional>
#include <string>

namespace legacy_api {
int read_device_status_code() {
    return 1;
}

const char* read_device_name() {
    return "pump-01";
}
} // namespace legacy_api

enum class DeviceStatus {
    normal,
    warning,
    failed,
};

struct DeviceSnapshot {
    std::string name;
    DeviceStatus status;
};

class LegacyDeviceAdapter {
public:
    std::optional<DeviceSnapshot> read() const {
        const int code = legacy_api::read_device_status_code();
        const char* name = legacy_api::read_device_name();

        const auto status = to_status(code);
        if (!status.has_value()) {
            return std::nullopt;
        }

        return DeviceSnapshot{std::string{name}, *status};
    }

private:
    static std::optional<DeviceStatus> to_status(int code) {
        switch (code) {
        case 0:
            return DeviceStatus::normal;
        case 1:
            return DeviceStatus::warning;
        case 2:
            return DeviceStatus::failed;
        default:
            // 알 수 없는 외부 값은 내부 도메인으로 억지 변환하지 않습니다.
            return std::nullopt;
        }
    }
};

std::string to_string(DeviceStatus status) {
    switch (status) {
    case DeviceStatus::normal:
        return "normal";
    case DeviceStatus::warning:
        return "warning";
    case DeviceStatus::failed:
        return "failed";
    }

    return "unknown";
}

int main() {
    LegacyDeviceAdapter adapter;
    const auto snapshot = adapter.read();

    if (!snapshot) {
        std::cout << "device status is invalid\n";
        return 1;
    }

    std::cout << snapshot->name << ": " << to_string(snapshot->status) << '\n';
}

