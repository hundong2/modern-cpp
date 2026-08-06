// Rust의 enum은 C++의 std::variant와 동일한 역할을 합니다.
enum MyData {
    Int(i32),
    Double(f64),
}

fn main() {
    let data = MyData::Int(100);

    // C++의 std::visit + overloaded가 하는 역할을 match가 언어 레벨에서 완벽히 처리합니다.
    match data {
        MyData::Int(i) => println!("[Rust] 정수 처리: {}", i),
        MyData::Double(d) => println!("[Rust] 실수 처리: {}", d),
    }
}

// == execute: /modern-cpp/자주까먹는/build/rust/rust/enum ==

// [Rust] 정수 처리: 100