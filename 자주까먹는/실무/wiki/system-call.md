# 시스템 호출과 파일 디스크립터

Linux/POSIX에서 소켓은 작은 정수 파일 디스크립터로 표현된다. 이 숫자는 프로세스의 커널 객체 테이블 인덱스와 비슷한 핸들이며, 사용 후 `close`해야 한다. `read`/`write`는 요청보다 적게 처리할 수 있고 신호로 중단되거나 non-blocking 모드에서 `EAGAIN`을 돌려줄 수 있다.

Windows의 `SOCKET`, `closesocket`, Winsock 초기화 규칙과는 호환되지 않는다. 이 교재의 POSIX 예제는 Linux/WSL에서 빌드한다.
