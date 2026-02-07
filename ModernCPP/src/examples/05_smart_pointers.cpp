// C++20 Smart Pointers 예제
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// 간단한 클래스
class Person {
public:
    Person(std::string name, int age) 
        : name_(std::move(name)), age_(age) {
        std::cout << "Person 생성: " << name_ << std::endl;
    }
    
    ~Person() {
        std::cout << "Person 소멸: " << name_ << std::endl;
    }
    
    void introduce() const {
        std::cout << "이름: " << name_ << ", 나이: " << age_ << std::endl;
    }

private:
    std::string name_;
    int age_;
};

// unique_ptr 예제
void example_unique_ptr() {
    std::cout << "\n=== unique_ptr 예제 ===" << std::endl;
    
    // 생성
    auto person1 = std::make_unique<Person>("Alice", 30);
    person1->introduce();
    
    // 이동 가능 (복사 불가)
    auto person2 = std::move(person1);
    // person1은 이제 nullptr
    
    if (!person1) {
        std::cout << "person1은 이제 비어있습니다" << std::endl;
    }
    
    person2->introduce();
}

// shared_ptr 예제
void example_shared_ptr() {
    std::cout << "\n=== shared_ptr 예제 ===" << std::endl;
    
    auto person1 = std::make_shared<Person>("Bob", 25);
    std::cout << "참조 카운트: " << person1.use_count() << std::endl;
    
    {
        // 복사 가능 - 참조 카운트 증가
        auto person2 = person1;
        std::cout << "참조 카운트: " << person1.use_count() << std::endl;
        
        auto person3 = person1;
        std::cout << "참조 카운트: " << person1.use_count() << std::endl;
        
        person2->introduce();
    }  // person2, person3 소멸
    
    std::cout << "참조 카운트: " << person1.use_count() << std::endl;
}

// weak_ptr 예제
void example_weak_ptr() {
    std::cout << "\n=== weak_ptr 예제 ===" << std::endl;
    
    std::weak_ptr<Person> weak_person;
    
    {
        auto person = std::make_shared<Person>("Charlie", 35);
        weak_person = person;  // weak_ptr로 관찰
        
        std::cout << "person 존재, 참조 카운트: " << person.use_count() << std::endl;
        
        // weak_ptr에서 shared_ptr 얻기
        if (auto locked = weak_person.lock()) {
            locked->introduce();
            std::cout << "weak_ptr로부터 접근 성공" << std::endl;
        }
    }  // person 소멸
    
    // person이 소멸된 후
    if (auto locked = weak_person.lock()) {
        std::cout << "접근 성공" << std::endl;
    } else {
        std::cout << "객체가 이미 소멸됨 (weak_ptr가 만료됨)" << std::endl;
    }
}

// 컨테이너와 함께 사용
void example_container() {
    std::cout << "\n=== 컨테이너와 스마트 포인터 ===" << std::endl;
    
    std::vector<std::unique_ptr<Person>> people;
    
    people.push_back(std::make_unique<Person>("David", 28));
    people.push_back(std::make_unique<Person>("Emma", 32));
    people.push_back(std::make_unique<Person>("Frank", 45));
    
    std::cout << "\n모든 사람들:" << std::endl;
    for (const auto& person : people) {
        person->introduce();
    }
}

int main() {
    std::cout << "=== C++20 스마트 포인터 예제 ===" << std::endl;
    
    example_unique_ptr();
    example_shared_ptr();
    example_weak_ptr();
    example_container();
    
    std::cout << "\n프로그램 종료 - 모든 객체 자동 정리됨" << std::endl;
    
    return 0;
}
