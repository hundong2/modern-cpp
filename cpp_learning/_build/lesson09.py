"""Lesson 09: 네트워크 — OSI 계층, TCP/UDP 소켓, 에코 서버."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 09 · 네트워크 프로그래밍과 OSI 계층

> **이 강의의 목표** — 클라이언트와 서버가 *바이트를 어떻게 주고받는지* 메모리·시스템콜 레벨에서 이해한다. POSIX 소켓 API 로 직접 에코 서버를 작성하고, `tcpdump` 로 그 패킷을 본다.

## 1. OSI 7계층 — 공식과 실제

교과서의 7계층 모델:

```
7. Application   — HTTP, SSH, DNS, FTP                  ← 우리가 짜는 코드
6. Presentation  — 인코딩, 압축, 암호화 (TLS 일부)
5. Session       — 세션 관리 (실제 인터넷에선 잘 안 보임)
4. Transport     — TCP, UDP                              ← socket() 의 SOCK_STREAM/DGRAM
3. Network       — IP (라우팅)                            ← 소켓의 ip 주소
2. Data Link     — Ethernet, Wi-Fi (MAC 주소)
1. Physical      — 케이블, 전파, 광섬유
```

실무에서는 보통 **TCP/IP 4계층 모델**로 통합해서 본다:

```
Application  (HTTP, gRPC, ssh, ...)
Transport    (TCP / UDP)
Internet     (IP)
Link         (Ethernet, Wi-Fi)
```

> **C++ 서버 개발자가 만지는 곳** — 거의 항상 4 (Transport) ~ 7 (Application). 1, 2 계층은 OS 와 NIC 드라이버가 처리한다.

## 2. TCP vs UDP — 한 컷 비교

| | TCP | UDP |
|---|-----|-----|
| 연결 | 있음 (3-way handshake) | 없음 |
| 신뢰성 | 패킷 손실 시 재전송 | 손실되면 그냥 잃음 |
| 순서 | 보장 | 보장 안 됨 |
| 속도 | 상대적 느림 | 빠름 |
| 흐름 제어 | 있음 | 없음 |
| 용도 | HTTP, SSH, gRPC, DB | DNS, VoIP, 게임 실시간 통신 |

C++ 서버 개발에서 90% 이상은 TCP 다. UDP 는 실시간성이 신뢰성보다 중요한 경우 — 게임의 위치 동기화, 비디오 스트리밍.

## 3. TCP 핸드셰이크와 4-way 종료 — 그림

```
Client                                         Server
  │                                              │
  │ ── SYN (seq=x) ──────────────────────────▶  │   1. 연결 요청
  │                                              │
  │ ◀────────────────  SYN+ACK (seq=y, ack=x+1) │   2. 응답
  │                                              │
  │ ── ACK (ack=y+1) ────────────────────────▶  │   3. 확인 — 연결 수립
  │                                              │
  │ ─── 데이터 ─────────────────────────────▶   │
  │ ◀──────────────────────────── 데이터 ───   │
  │                                              │
  │ ── FIN ──────────────────────────────────▶  │   4-way 종료
  │ ◀────────────────────────────────── ACK ──  │
  │ ◀────────────────────────────────── FIN ──  │
  │ ── ACK ─────────────────────────────────▶   │
```

이 단계 각각이 socket API 의 `connect`, `accept`, `recv`, `send`, `close` 에 대응된다."""))

cells.append(md(r"""## 4. POSIX 소켓 API — 큰 그림

### 서버 측

```
socket()    → fd 생성
  ↓
bind()      → IP:port 에 바인딩
  ↓
listen()    → 대기 큐 생성
  ↓
accept()    → 클라이언트 연결 수락 (블록), 새 fd 반환
  ↓
read/write  → 데이터 송수신
  ↓
close()     → 연결 종료
```

### 클라이언트 측

```
socket()    → fd 생성
  ↓
connect()   → 서버 IP:port 에 연결
  ↓
read/write  → 데이터 송수신
  ↓
close()     → 연결 종료
```

핵심은 **fd (file descriptor)**. 리눅스에서 *모든 것이 파일이다* — 소켓도 fd 다. 그래서 `read`/`write` 가 그대로 통한다."""))

cells.append(md(r"""## 5. 직접 만들어보는 TCP 에코 서버

xeus-cling 안에서 서버를 띄우면 노트북이 블록되어 다른 셀을 못 쓴다. 그래서 **소스 파일을 만들어 백그라운드로 실행**하고, 그 다음 셀에서 클라이언트로 접속한다."""))

cells.append(code(r"""#include <cstdio>
#include <cstdlib>
#include <iostream>

const char* server_src = R"CPP(
// echo_server.cpp — 가장 단순한 TCP 에코 서버
//   1) 클라이언트가 보낸 데이터를 그대로 돌려보낸다.
//   2) 동시 클라이언트는 처리 안 함 (순차) — 다음 단계에서 thread/select 로 확장.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    int port = (argc > 1) ? std::atoi(argv[1]) : 9000;

    // 1. 소켓 생성: AF_INET = IPv4, SOCK_STREAM = TCP, 0 = 기본 프로토콜
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }

    // SO_REUSEADDR: 서버를 빨리 재시작할 때 'Address already in use' 방지
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // 2. 바인딩할 주소 구조체
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // 모든 인터페이스 (0.0.0.0)
    addr.sin_port        = htons(port);          // 호스트 → 네트워크 바이트 순서
    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }

    // 3. 리슨 — backlog 큐 크기 32
    if (listen(srv, 32) < 0) { perror("listen"); return 1; }
    std::cerr << "[server] listening on port " << port << "\n";

    while (true) {
        sockaddr_in cli_addr{};
        socklen_t   cli_len = sizeof(cli_addr);

        // 4. accept — 새 연결을 기다린다 (블록)
        int cli = accept(srv, (sockaddr*)&cli_addr, &cli_len);
        if (cli < 0) { perror("accept"); continue; }

        char ip[64];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        std::cerr << "[server] connected from " << ip << ":" << ntohs(cli_addr.sin_port) << "\n";

        // 5. 에코 루프: 받은 만큼 그대로 돌려보낸다
        char buf[1024];
        while (true) {
            ssize_t n = recv(cli, buf, sizeof(buf), 0);
            if (n <= 0) break;        // 0 = 상대가 close, 음수 = 에러
            ssize_t sent = 0;
            while (sent < n) {
                ssize_t k = send(cli, buf + sent, n - sent, 0);
                if (k <= 0) break;
                sent += k;
            }
        }
        close(cli);
        std::cerr << "[server] connection closed\n";
    }

    close(srv);
}
)CPP";

std::system("mkdir -p /tmp/cpplab/net");
{
    FILE* f = std::fopen("/tmp/cpplab/net/echo_server.cpp", "w");
    std::fputs(server_src, f);
    std::fclose(f);
}
std::system("g++ -std=c++17 -O2 -Wall /tmp/cpplab/net/echo_server.cpp "
            "-o /tmp/cpplab/net/echo_server 2>&1");
std::cout << "서버 빌드 완료\n";"""))

cells.append(md(r"""**`htons` / `htonl` 의 정체**: 네트워크는 빅 엔디안이지만 x86 은 리틀 엔디안이다. `htons` (host→network short, 16비트), `htonl` (long, 32비트) 가 자동 변환을 해준다. 이 함수가 빠지면 포트 번호가 엉뚱하게 해석되어 *연결만 안 된다* — 디버깅 한참 한다.

이제 서버를 백그라운드로 띄운다:"""))

cells.append(code(r"""// 기존 서버가 떠 있다면 종료
std::system("pkill -f /tmp/cpplab/net/echo_server 2>/dev/null; sleep 0.3");
// 포트 9000 으로 백그라운드 실행
std::system("nohup /tmp/cpplab/net/echo_server 9000 "
            "> /tmp/cpplab/net/server.log 2>&1 &");
std::system("sleep 0.5 && cat /tmp/cpplab/net/server.log");"""))

cells.append(md(r"""## 6. 클라이언트 작성"""))

cells.append(code(r"""const char* client_src = R"CPP(
// echo_client.cpp — 한 줄을 보내고 응답을 받는다
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 4) { std::cerr << "usage: " << argv[0] << " <ip> <port> <message>\n"; return 1; }
    const char* ip = argv[1];
    int port = std::atoi(argv[2]);
    std::string msg = argv[3];

    int s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(s, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); return 1; }

    send(s, msg.data(), msg.size(), 0);

    char buf[1024];
    ssize_t n = recv(s, buf, sizeof(buf), 0);
    if (n > 0) std::cout.write(buf, n);
    std::cout << "\n";

    close(s);
}
)CPP";

{
    FILE* f = std::fopen("/tmp/cpplab/net/echo_client.cpp", "w");
    std::fputs(client_src, f);
    std::fclose(f);
}
std::system("g++ -std=c++17 -O2 -Wall /tmp/cpplab/net/echo_client.cpp "
            "-o /tmp/cpplab/net/echo_client 2>&1");
std::cout << "클라이언트 빌드 완료\n";

// 클라이언트 호출 — 여러 메시지 보내보기
std::system("/tmp/cpplab/net/echo_client 127.0.0.1 9000 'Hello, server!'");
std::system("/tmp/cpplab/net/echo_client 127.0.0.1 9000 'Second message'");
std::system("/tmp/cpplab/net/echo_client 127.0.0.1 9000 'C++ rocks'");"""))

cells.append(md(r"""## 7. 패킷을 직접 보기 — `tcpdump`

이 단계가 *진짜로 어떤 일이 일어나는지* 보는 순간이다."""))

cells.append(code(r"""// 백그라운드로 tcpdump 시작 (loopback 인터페이스 lo 의 9000 포트만)
std::system("pkill tcpdump 2>/dev/null; sleep 0.2");
std::system("tcpdump -i lo -nn -l 'tcp port 9000' "
            "> /tmp/cpplab/net/dump.txt 2>&1 &");
std::system("sleep 0.5");

// 트래픽 발생
std::system("/tmp/cpplab/net/echo_client 127.0.0.1 9000 'capture-me'");
std::system("sleep 0.5");

// 캡처 종료 후 결과
std::system("pkill tcpdump 2>/dev/null; sleep 0.2");
std::cout << "── tcpdump 결과 ──\n";
std::system("cat /tmp/cpplab/net/dump.txt");"""))

cells.append(md(r"""**출력 읽는 법**:

- `[S]` 또는 `Flags [S]` — SYN 패킷 (3-way handshake 의 1단계)
- `[S.]` — SYN+ACK (2단계)
- `[.]`  — ACK
- `[P.]` — Push + ACK (실제 데이터)
- `[F.]` — FIN (종료)

위 캡처에서 SYN → SYN+ACK → ACK → 데이터 → FIN 의 흐름이 그대로 보일 것이다. 책에서 본 다이어그램이 *실제로 살아 움직이는* 순간이다.

## 8. 동시 클라이언트 처리 — thread per connection 에서 `epoll` 까지

위 서버는 한 번에 한 클라이언트만 처리한다. 실제 서버는:

### 단계 1: thread per connection
가장 간단. accept 한 fd 를 새 스레드에 넘긴다. 1만 동시 연결이 넘어가면 스레드 자체의 메모리 (보통 8MB 스택) 가 부담.

### 단계 2: thread pool
미리 N 개의 스레드를 만들어두고 작업 큐로 분배. 스레드 수 = CPU 코어 수에 가깝게.

### 단계 3: I/O 다중화 — `select` / `poll` / `epoll` (리눅스) / `kqueue` (BSD/macOS) / `IOCP` (Windows)
한 스레드가 *여러 fd 의 이벤트를 한꺼번에 기다린다*. 10만 동시 연결도 한 스레드로 가능.

### 단계 4: 비동기 I/O — `io_uring` (리눅스 5.1+)
시스템 콜의 비용 자체를 줄인다. 메모리 매핑된 ring buffer 를 통해 커널과 통신.

C++ 에서 이 모든 것을 *추상화*해주는 라이브러리가 있다 — 가장 유명한 게 **boost::asio** (C++26 표준화 진행 중). 실무 서버는 거의 다 asio 또는 그 위에 올라간 프레임워크를 쓴다.

## 9. 응용 계층 프로토콜 — HTTP 의 첫 줄을 직접 본다

위 에코 서버에 *진짜 브라우저처럼* 요청해본다."""))

cells.append(code(r"""// 서버를 다시 깨끗이 띄움
std::system("pkill -f /tmp/cpplab/net/echo_server 2>/dev/null; sleep 0.3");
std::system("nohup /tmp/cpplab/net/echo_server 9000 > /tmp/cpplab/net/server.log 2>&1 &");
std::system("sleep 0.3");

// HTTP 형태의 메시지를 보낸다 (서버는 그냥 에코해주므로 '받은 그대로' 보임)
std::system("printf 'GET / HTTP/1.1\\r\\nHost: example.com\\r\\nUser-Agent: cpplab\\r\\n\\r\\n' "
            "| nc -q 1 127.0.0.1 9000");"""))

cells.append(md(r"""HTTP 의 한 줄짜리 본질: **텍스트 프로토콜이다**. `GET / HTTP/1.1\r\n` + 헤더들 + `\r\n\r\n` (헤더 끝) + 바디. 그게 전부. 우리가 평소 쓰는 거대한 웹 프레임워크는 이 텍스트를 파싱해서 라우팅·미들웨어·뷰로 분배할 뿐이다.

## 10. 서버 정리"""))

cells.append(code(r"""std::system("pkill -f /tmp/cpplab/net/echo_server 2>/dev/null");
std::cout << "서버 종료\n";"""))

cells.append(md(r"""## 11. 실습

다음을 직접 해보자:

1. **서버를 thread-per-connection 으로 확장하라**. accept 한 fd 를 `std::thread` 에 넘겨 처리. 두 개의 클라이언트가 동시에 통신하는지 확인.
2. **클라이언트가 종료되었을 때 서버가 *루프를 빠져나가는지* 확인하라**. recv 가 0 을 반환하는 시점. 만약 0 처리를 빼면 어떻게 되나?
3. **메시지 끝을 어떻게 알지?** 위 에코는 1024바이트 버퍼 단위. 하지만 진짜 프로토콜은 *프레이밍* 이 필요하다 — (a) 길이 prefix 방식 (앞 4바이트가 길이) (b) 구분자 방식 (`\n` 이나 `\r\n\r\n`). 둘 중 하나로 자체 프로토콜을 짜본다.
4. **`tcpdump` 로 데이터 패킷의 페이로드를 보라**. `tcpdump -X` 옵션으로 hex+ASCII 덤프.

## 12. 정리

- 7계층 모델을 외우는 것보다 *어떤 계층에서 어떤 일이 일어나는지* 직관을 가져라. 우리가 코드로 만지는 건 거의 4~7.
- 소켓 API: `socket → bind → listen → accept → read/write → close`. 클라이언트는 `socket → connect → read/write → close`.
- `htons`/`htonl` 은 빠지지 마라.
- 동시성 — 시작은 thread per connection, 규모가 커지면 epoll/asio 로 간다.
- HTTP 는 단순한 텍스트 프로토콜이다. 한번 직접 보내보면 신비가 사라진다.

다음 강의 — Lesson 10: 성능 최적화 (캐시, 프로파일링, move/inline, undefined behavior 의 함정).
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/09_networking.ipynb")
