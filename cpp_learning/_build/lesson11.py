"""Lesson 11: 종합 프로젝트 — 미니 키-값 저장소 + HTTP 서버."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 11 · 종합 프로젝트 — 미니 키-값 저장소 + HTTP 서버

> **이 강의의 목표** — 지금까지 배운 모든 도구(클래스/RAII/스마트포인터/템플릿/concepts/ranges/concurrency/networking/optimization)를 **하나의 실전 프로젝트** 에 통합한다.

만들 것: 메모리 안에서 동작하는 키-값 저장소를 HTTP 서버로 노출한다. Redis 의 0.001% 같은 것.

```
GET  /kv/<key>          → 값 반환 (없으면 404)
PUT  /kv/<key>  body    → 저장
DEL  /kv/<key>          → 삭제
GET  /stats             → 통계 (count, hits, misses)
```

빌드는 단계적으로:

1. **Step 1** — 단일 스레드 KV 저장소 (`std::unordered_map` 래핑).
2. **Step 2** — 스레드 안전 래퍼 (mutex / shared_mutex).
3. **Step 3** — TCP 서버 골격 (Lesson 09 응용).
4. **Step 4** — 미니 HTTP 파서 (요청 라인 + 헤더 + 바디).
5. **Step 5** — 라우터 + 핸들러 결합.
6. **Step 6** — 멀티스레드 (스레드 풀) + 통계.
7. **Step 7** — 성능 측정.

각 단계가 그대로 실행된다. 마지막엔 `curl` 로 직접 두드린다."""))

cells.append(md(r"""## 0. 빌드 헬퍼"""))

cells.append(code(r"""#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

void run_cpp(const std::string& code,
             const std::string& opt = "-O2",
             const std::string& std_ver = "c++20",
             const std::string& extra = "-pthread") {
    std::system("mkdir -p /tmp/cpplab/kv");
    FILE* f = std::fopen("/tmp/cpplab/kv/snippet.cpp", "w");
    std::fputs(code.c_str(), f);
    std::fclose(f);
    std::string cmd = "g++ -std=" + std_ver + " " + opt + " -Wall -Wextra " + extra
                    + " /tmp/cpplab/kv/snippet.cpp -o /tmp/cpplab/kv/snippet 2>&1 "
                      "&& /tmp/cpplab/kv/snippet";
    std::system(cmd.c_str());
}

void build_only(const std::string& code, const std::string& target,
                const std::string& opt = "-O2",
                const std::string& std_ver = "c++20",
                const std::string& extra = "-pthread") {
    std::system("mkdir -p /tmp/cpplab/kv");
    std::string src = "/tmp/cpplab/kv/" + target + ".cpp";
    std::string bin = "/tmp/cpplab/kv/" + target;
    FILE* f = std::fopen(src.c_str(), "w");
    std::fputs(code.c_str(), f);
    std::fclose(f);
    std::string cmd = "g++ -std=" + std_ver + " " + opt + " -Wall -Wextra " + extra
                    + " " + src + " -o " + bin + " 2>&1 && echo '빌드 성공: " + bin + "'";
    std::system(cmd.c_str());
}"""))

cells.append(md(r"""## Step 1 — 단일 스레드 KV 저장소

설계 포인트:

- 외부에 std::unordered_map 을 노출하지 않는다 (캡슐화).
- 값은 `std::string` 으로만. (간단함을 위해.)
- API: `put / get / erase / size`. `get` 은 `std::optional<std::string>` 으로 — 없는 키와 빈 문자열을 구분."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

class KvStore {
public:
    void put(std::string key, std::string value) {
        // emplace 대신 [] 로 단순하게 — 같은 키 덮어쓰기 의미가 명확
        data_[std::move(key)] = std::move(value);
    }

    std::optional<std::string> get(const std::string& key) const {
        if (auto it = data_.find(key); it != data_.end())
            return it->second;
        return std::nullopt;
    }

    bool erase(const std::string& key) {
        return data_.erase(key) > 0;
    }

    std::size_t size() const noexcept { return data_.size(); }

private:
    std::unordered_map<std::string, std::string> data_;
};

int main() {
    KvStore kv;
    kv.put("name", "claude");
    kv.put("lang", "cpp20");

    if (auto v = kv.get("name"))   std::cout << "name=" << *v << "\n";
    if (!kv.get("missing"))        std::cout << "missing 없음 (정상)\n";

    std::cout << "size=" << kv.size() << "\n";
    kv.erase("lang");
    std::cout << "erase 후 size=" << kv.size() << "\n";
}
)CPP", "-O2");"""))

cells.append(md(r"""**주목할 점**

- `std::move(key)` 로 키를 받아 들여 *복사 대신 이동*. 호출자가 임시 객체를 넘기면 비용 0.
- `std::optional<std::string>` 으로 "없음" 을 표현. C++17 의 깔끔한 패턴.
- `if (auto it = data_.find(key); it != data_.end())` — C++17 if-init. 스코프를 좁게.

이 클래스는 *단일 스레드* 에선 안전하다. 여러 스레드가 동시에 put/get 하면 깨진다 → Step 2."""))

cells.append(md(r"""## Step 2 — 스레드 안전 래퍼

`std::shared_mutex` 를 쓴다. 읽기는 동시 가능, 쓰기는 배타적.

- 읽기(`get`, `size`): `std::shared_lock`
- 쓰기(`put`, `erase`): `std::unique_lock`

**왜 단순 mutex 가 아닌가** — 읽기 비율이 높은 워크로드에서 처리량이 크게 차이 난다."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <atomic>

class KvStore {
public:
    void put(std::string key, std::string value) {
        std::unique_lock lock(m_);
        data_[std::move(key)] = std::move(value);
    }

    std::optional<std::string> get(const std::string& key) const {
        std::shared_lock lock(m_);
        if (auto it = data_.find(key); it != data_.end()) return it->second;
        return std::nullopt;
    }

    bool erase(const std::string& key) {
        std::unique_lock lock(m_);
        return data_.erase(key) > 0;
    }

    std::size_t size() const {
        std::shared_lock lock(m_);
        return data_.size();
    }

private:
    mutable std::shared_mutex m_;        // const 메서드에서 lock 해야 하므로 mutable
    std::unordered_map<std::string, std::string> data_;
};

int main() {
    KvStore kv;
    std::atomic<int> done{0};

    // writer 4개, reader 4개 동시에 굴린다
    std::vector<std::thread> ts;
    for (int w = 0; w < 4; ++w) {
        ts.emplace_back([&, w] {
            for (int i = 0; i < 5000; ++i)
                kv.put("k" + std::to_string((w * 5000 + i) % 1000),
                       "v" + std::to_string(i));
            ++done;
        });
    }
    for (int r = 0; r < 4; ++r) {
        ts.emplace_back([&] {
            int hits = 0;
            for (int i = 0; i < 5000; ++i)
                if (kv.get("k" + std::to_string(i % 1000))) ++hits;
            // hits 는 시점에 따라 달라짐
        });
    }
    for (auto& t : ts) t.join();

    std::cout << "최종 size = " << kv.size() << " (≤1000)\n";
    std::cout << "writer 완료 = " << done.load() << "/4\n";
}
)CPP", "-O2");"""))

cells.append(md(r"""## Step 3 — TCP 서버 골격

Lesson 09 의 echo 서버를 KV 서버 베이스로 변형. 한 번에 한 클라이언트만 받는 형태로 시작 → Step 6 에서 멀티스레드로."""))

cells.append(md(r"""## Step 4 — 미니 HTTP 파서

HTTP/1.1 의 *최소* 부분만 다룬다:

```
PUT /kv/foo HTTP/1.1\r\n
Host: localhost\r\n
Content-Length: 5\r\n
\r\n
hello
```

파서가 해야 할 일:

1. 첫 줄을 split → method, path, version.
2. 헤더를 `\r\n\r\n` 까지 읽어 `Content-Length` 추출.
3. 그 길이만큼 바디를 더 읽음.

응답 형식:

```
HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello
```"""))

cells.append(md(r"""## Step 5–6 — 한 파일에 통합 (멀티스레드 KV 서버)

지금까지 조각들을 합친 완성판이다. 한 파일로 만들어 빌드만 한다 (실행은 다음 셀에서 백그라운드로)."""))

cells.append(code(r"""std::string server_code = R"CPP(
// =============================================================
//  Mini KV HTTP Server — Lesson 11 capstone
//  - C++20, POSIX sockets, 스레드 풀, shared_mutex
// =============================================================
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <deque>
#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

// ---------- KvStore (Step 2 그대로) ----------
class KvStore {
public:
    void put(std::string k, std::string v) {
        std::unique_lock lock(m_);
        data_[std::move(k)] = std::move(v);
    }
    std::optional<std::string> get(const std::string& k) const {
        std::shared_lock lock(m_);
        if (auto it = data_.find(k); it != data_.end()) return it->second;
        return std::nullopt;
    }
    bool erase(const std::string& k) {
        std::unique_lock lock(m_);
        return data_.erase(k) > 0;
    }
    std::size_t size() const {
        std::shared_lock lock(m_);
        return data_.size();
    }
private:
    mutable std::shared_mutex m_;
    std::unordered_map<std::string, std::string> data_;
};

// ---------- 통계 ----------
struct Stats {
    std::atomic<long> hits{0}, misses{0}, puts{0}, dels{0}, requests{0};
};

// ---------- 작은 HTTP 파서 ----------
struct Request {
    std::string method, path, body;
    std::size_t content_length = 0;
};

// recv 로 데이터를 buf 에 누적, 헤더 끝(\r\n\r\n) 까지 읽고 그 후 body 도 읽는다
bool read_request(int fd, Request& req) {
    std::string buf;
    char tmp[4096];
    // 헤더 읽기
    while (buf.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
        if (n <= 0) return false;
        buf.append(tmp, tmp + n);
        if (buf.size() > 65536) return false;       // 헤더 폭주 방지
    }
    auto header_end = buf.find("\r\n\r\n");
    std::string header = buf.substr(0, header_end);
    std::string after_header = buf.substr(header_end + 4);

    // 첫 줄: METHOD PATH HTTP/1.1
    auto first_eol = header.find("\r\n");
    std::string first = header.substr(0, first_eol);
    {
        std::istringstream iss(first);
        std::string ver;
        iss >> req.method >> req.path >> ver;
    }

    // Content-Length 헤더 추출
    std::size_t pos = first_eol + 2;
    while (pos < header.size()) {
        auto eol = header.find("\r\n", pos);
        if (eol == std::string::npos) eol = header.size();
        std::string line = header.substr(pos, eol - pos);
        // 대소문자 무시 비교
        if (line.size() > 15) {
            std::string lower; lower.reserve(line.size());
            for (char c : line) lower.push_back(std::tolower(c));
            if (lower.starts_with("content-length:")) {
                req.content_length = std::stoul(line.substr(15));
            }
        }
        pos = eol + 2;
    }

    // 바디 마저 읽기
    req.body = after_header;
    while (req.body.size() < req.content_length) {
        ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
        if (n <= 0) return false;
        req.body.append(tmp, tmp + n);
    }
    if (req.body.size() > req.content_length)
        req.body.resize(req.content_length);
    return true;
}

void send_response(int fd, int code, std::string_view status,
                   std::string_view body, std::string_view ctype = "text/plain") {
    std::string head;
    head.reserve(128);
    head += "HTTP/1.1 " + std::to_string(code) + " " + std::string(status) + "\r\n";
    head += "Content-Type: " + std::string(ctype) + "\r\n";
    head += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    head += "Connection: close\r\n\r\n";
    ::send(fd, head.data(), head.size(), 0);
    if (!body.empty()) ::send(fd, body.data(), body.size(), 0);
}

// ---------- 라우터 ----------
void handle(int fd, KvStore& kv, Stats& st) {
    Request req;
    if (!read_request(fd, req)) { ::close(fd); return; }
    ++st.requests;

    constexpr std::string_view PFX = "/kv/";
    if (req.path.starts_with(PFX)) {
        std::string key(req.path.begin() + PFX.size(), req.path.end());
        if (req.method == "GET") {
            if (auto v = kv.get(key)) { ++st.hits; send_response(fd, 200, "OK", *v); }
            else                      { ++st.misses; send_response(fd, 404, "Not Found", "missing\n"); }
        } else if (req.method == "PUT" || req.method == "POST") {
            kv.put(std::move(key), std::move(req.body));
            ++st.puts; send_response(fd, 200, "OK", "stored\n");
        } else if (req.method == "DELETE") {
            bool ok = kv.erase(key);
            ++st.dels;
            send_response(fd, ok ? 200 : 404, ok ? "OK" : "Not Found", ok ? "deleted\n" : "missing\n");
        } else {
            send_response(fd, 405, "Method Not Allowed", "");
        }
    } else if (req.path == "/stats" && req.method == "GET") {
        std::string body;
        body.reserve(128);
        body += "{\n";
        body += "  \"size\": "     + std::to_string(kv.size())          + ",\n";
        body += "  \"hits\": "     + std::to_string(st.hits.load())     + ",\n";
        body += "  \"misses\": "   + std::to_string(st.misses.load())   + ",\n";
        body += "  \"puts\": "     + std::to_string(st.puts.load())     + ",\n";
        body += "  \"dels\": "     + std::to_string(st.dels.load())     + ",\n";
        body += "  \"requests\": " + std::to_string(st.requests.load()) + "\n";
        body += "}\n";
        send_response(fd, 200, "OK", body, "application/json");
    } else {
        send_response(fd, 404, "Not Found", "unknown route\n");
    }
    ::close(fd);
}

// ---------- 스레드 풀 ----------
class ThreadPool {
public:
    explicit ThreadPool(std::size_t n) {
        for (std::size_t i = 0; i < n; ++i)
            workers_.emplace_back([this]{ loop(); });
    }
    ~ThreadPool() { stop(); }

    void submit(std::function<void()> job) {
        {
            std::lock_guard lk(m_);
            q_.push_back(std::move(job));
        }
        cv_.notify_one();
    }

    void stop() {
        { std::lock_guard lk(m_); stop_ = true; }
        cv_.notify_all();
        for (auto& t : workers_) if (t.joinable()) t.join();
        workers_.clear();
    }

private:
    void loop() {
        for (;;) {
            std::function<void()> job;
            {
                std::unique_lock lk(m_);
                cv_.wait(lk, [&]{ return stop_ || !q_.empty(); });
                if (stop_ && q_.empty()) return;
                job = std::move(q_.front()); q_.pop_front();
            }
            try { job(); } catch (...) { /* 작업 실패 격리 */ }
        }
    }
    std::vector<std::thread> workers_;
    std::deque<std::function<void()>> q_;
    std::mutex m_; std::condition_variable cv_; bool stop_ = false;
};

// ---------- main ----------
int main(int argc, char** argv) {
    int port = (argc > 1) ? std::atoi(argv[1]) : 9000;

    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1; ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (::bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (::listen(srv, 64) < 0)                           { perror("listen"); return 1; }

    KvStore kv; Stats st; ThreadPool pool(std::thread::hardware_concurrency());
    std::cout << "KV server listening on :" << port << " (threads="
              << std::thread::hardware_concurrency() << ")\n";

    for (;;) {
        sockaddr_in c{}; socklen_t cl = sizeof(c);
        int fd = ::accept(srv, (sockaddr*)&c, &cl);
        if (fd < 0) { if (errno == EINTR) continue; perror("accept"); break; }
        pool.submit([fd, &kv, &st]{ handle(fd, kv, st); });
    }
    ::close(srv);
}
)CPP";

build_only(server_code, "kv_server");"""))

cells.append(md(r"""빌드만 했다. 이제 백그라운드로 실행하고 `curl` 로 두드린다."""))

cells.append(code(r"""// 이전 인스턴스 종료 후 백그라운드 실행
std::system("pkill -f /tmp/cpplab/kv/kv_server >/dev/null 2>&1; sleep 0.2");
std::system("nohup /tmp/cpplab/kv/kv_server 9000 >/tmp/cpplab/kv/server.log 2>&1 &");
std::system("sleep 0.4 && cat /tmp/cpplab/kv/server.log");"""))

cells.append(code(r"""// PUT
std::system("curl -s -X PUT --data-binary 'hello world' http://127.0.0.1:9000/kv/greeting; echo");
std::system("curl -s -X PUT --data-binary 'C++20 forever' http://127.0.0.1:9000/kv/lang; echo");

// GET
std::system("echo 'GET greeting:'; curl -s http://127.0.0.1:9000/kv/greeting; echo");
std::system("echo 'GET lang:';     curl -s http://127.0.0.1:9000/kv/lang;     echo");
std::system("echo 'GET missing:';  curl -s -w '%{http_code}\\n' http://127.0.0.1:9000/kv/missing");

// DELETE
std::system("echo 'DELETE lang:';  curl -s -X DELETE http://127.0.0.1:9000/kv/lang; echo");

// stats
std::system("echo '--- stats ---'; curl -s http://127.0.0.1:9000/stats");"""))

cells.append(md(r"""## Step 7 — 성능 측정

`ab` (Apache Bench) 또는 `wrk` 가 표준이지만, 도커 이미지에 항상 있는 건 아니다. 대신 셸 루프 + xargs 병렬화로 충분히 의미 있는 수치가 나온다."""))

cells.append(code(r"""// 1000 회 GET 의 처리량 (직렬)
std::system(R"BASH(
{
  start=$(date +%s.%N)
  for i in $(seq 1 1000); do
    curl -s http://127.0.0.1:9000/kv/greeting > /dev/null
  done
  end=$(date +%s.%N)
  awk -v s=$start -v e=$end 'BEGIN{printf "직렬 1000 요청: %.3f s (%.0f req/s)\n", e-s, 1000/(e-s)}'
}
)BASH");

// 100 동시 × 100 요청 (ab 대용)
std::system(R"BASH(
{
  start=$(date +%s.%N)
  seq 1 100 | xargs -n1 -P100 -I{} bash -c '
    for i in $(seq 1 100); do curl -s http://127.0.0.1:9000/kv/greeting > /dev/null; done'
  end=$(date +%s.%N)
  awk -v s=$start -v e=$end 'BEGIN{printf "병렬 10000 요청: %.3f s (%.0f req/s)\n", e-s, 10000/(e-s)}'
}
)BASH");

// 최종 stats
std::system("echo '--- 최종 stats ---'; curl -s http://127.0.0.1:9000/stats");"""))

cells.append(md(r"""성능을 더 끌어올리는 방향(이 강의의 범위 밖이지만 알아둘 가치):

1. **Connection: keep-alive** — 매 요청마다 TCP 연결 다시 만드는 비용을 제거. 핵심.
2. **이벤트 루프 (epoll/io_uring)** — 스레드 수 ≪ 연결 수. nginx, redis 가 쓰는 모델.
3. **zero-copy** (`sendfile`, `splice`).
4. **메모리 풀** — 자주 쓰는 객체의 할당을 미리.
5. **샤딩** — KV 를 N 개 맵으로 쪼개 락 경합 줄이기.

지금 우리 서버는 *연결당 스레드* 가 아니라 *연결당 풀 작업* 이라 백 개 동시 연결을 받아도 무너지지 않는다. 그게 ThreadPool 의 역할이다."""))

cells.append(md(r"""## 8. 정리 (서버 종료)"""))

cells.append(code(r"""std::system("pkill -f /tmp/cpplab/kv/kv_server; sleep 0.3; echo '서버 종료'");"""))

cells.append(md(r"""## 9. 회고 — 이 한 프로젝트에 들어간 모든 것

| 강의 | 사용된 곳 |
|------|-----------|
| 01 변수/함수 | 어디든 |
| 02 메모리 모델 | recv 버퍼, sockaddr_in 구조체 |
| 03 RAII / 스마트포인터 | `lock_guard`, `unique_lock`, `shared_lock` 모두 RAII |
| 04 헷갈리는 문법 | `mutable`, `if constexpr`, const 정확성 |
| 05 템플릿 | `ThreadPool::submit(std::function<void()>)`, `optional<T>` |
| 06 C++20 | `string_view::starts_with`, structured binding |
| 07 C++23 | (이번엔 안 썼다 — std::expected 로 에러 처리 리팩토링이 자연스러운 다음 단계) |
| 08 동시성 | shared_mutex, atomic, condition_variable, ThreadPool |
| 09 네트워크 | socket/bind/listen/accept, recv/send |
| 10 최적화 | -O2, std::move 로 키 이동, 컨테이너 선택 |

## 10. 다음 단계 — 더 가지고 놀아보기

- **HTTP keep-alive** 지원: 응답 헤더 `Connection: close` 빼고, 같은 연결로 다음 요청 받기.
- **TTL (만료)**: PUT 에 `?ttl=10` 쿼리 받아서 10초 후 자동 삭제 (별도 정리 스레드).
- **디스크 영속**: 시작할 때 파일에서 로드, 종료할 때 저장.
- **샤딩**: `std::array<KvStore, 16>` 으로 키 해시 기반 분산.
- **`std::expected` 로 에러 리팩토링** (C++23) — 라우터 핸들러가 `expected<Response, Error>` 반환.
- **클라이언트 라이브러리** — C++ 클라이언트 클래스 (`KvClient::get(...)` 등) 작성.
- **벤치마크**: `wrk -t4 -c100 -d10s http://localhost:9000/kv/greeting` 로 정식 측정.

축하한다 — C++17, C++20, C++23 의 핵심 기능과 시스템 프로그래밍의 기본기를 모두 한 프로젝트에 묶어 돌려봤다. 여기서부터는 cppreference.com 을 사전처럼 읽으면서 본인 도메인의 코드를 짜는 단계다."""))

nb = make_notebook(cells, kernel="xcpp17")
out = os.path.join(os.path.dirname(__file__), "..", "notebooks", "11_capstone.ipynb")
save(nb, out)
