#include <boost/program_options.hpp>

#include <iostream>
#include <string>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    po::options_description description{"options"};
    description.add_options()
        ("help,h", "show help")
        ("port,p", po::value<int>()->default_value(8080), "listen port")
        ("config,c", po::value<std::string>(), "config file path");

    po::variables_map variables;
    po::store(po::parse_command_line(argc, argv, description), variables);
    po::notify(variables);

    if (variables.count("help") != 0) {
        std::cout << description << '\n';
        return 0;
    }

    const int port = variables["port"].as<int>();
    std::cout << "port=" << port << '\n';

    if (variables.count("config") != 0) {
        std::cout << "config=" << variables["config"].as<std::string>() << '\n';
    }

    // program_options는 header-only가 아니므로 보통 boost_program_options 링크가 필요합니다.
    // 서버/CLI 도구의 옵션 파싱 코드에서 자주 보이는 형태입니다.
}

