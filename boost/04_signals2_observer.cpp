#include <boost/signals2.hpp>

#include <iostream>
#include <string_view>

class DownloadService {
public:
    boost::signals2::connection OnProgress(
        const boost::signals2::signal<void(int)>::slot_type& slot) {
        return progress_.connect(slot);
    }

    void Download(std::string_view file_name) {
        std::cout << "download: " << file_name << '\n';
        progress_(10);
        progress_(50);
        progress_(100);
    }

private:
    boost::signals2::signal<void(int)> progress_;
};

int main() {
    DownloadService service;

    boost::signals2::connection connection = service.OnProgress([](int percent) {
        std::cout << "progress=" << percent << "%\n";
    });

    service.Download("report.csv");

    connection.disconnect();
    service.Download("ignored.csv");

    // signals2는 observer 패턴 구현에 자주 쓰입니다.
    // connection 객체를 보관하면 구독 해제를 명시적으로 처리할 수 있습니다.
}

