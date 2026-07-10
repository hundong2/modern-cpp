#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/tag.hpp>

#include <iostream>
#include <string>

struct Employee {
    int id{};
    std::string name;
    int team{};
};

struct by_id {};
struct by_name {};
struct by_team {};

using EmployeeTable = boost::multi_index_container<
    Employee,
    boost::multi_index::indexed_by<
        boost::multi_index::hashed_unique<
            boost::multi_index::tag<by_id>,
            boost::multi_index::member<Employee, int, &Employee::id>>,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<by_name>,
            boost::multi_index::member<Employee, std::string, &Employee::name>>,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<by_team>,
            boost::multi_index::member<Employee, int, &Employee::team>>>>;

int main() {
    EmployeeTable employees;
    employees.insert({1, "kim", 10});
    employees.insert({2, "lee", 20});
    employees.insert({3, "park", 10});

    const auto& id_index = employees.get<by_id>();
    if (auto found = id_index.find(2); found != id_index.end()) {
        std::cout << "id lookup: " << found->name << '\n';
    }

    const auto& team_index = employees.get<by_team>();
    const auto [first, last] = team_index.equal_range(10);

    for (auto it = first; it != last; ++it) {
        std::cout << "team 10: " << it->name << '\n';
    }

    // 하나의 Employee 객체 저장소에 여러 인덱스를 얹습니다.
    // map 두 개를 따로 유지하는 방식보다 동기화 버그를 줄일 수 있습니다.
    // 대신 템플릿 타입이 복잡해져 컴파일 에러가 길어질 수 있습니다.
}

