# 베이스 이미지로 최신 Ubuntu 사용
FROM ubuntu:latest

# 필요한 패키지 설치
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    clang \
    && apt-get clean

# 작업 디렉토리 설정
WORKDIR /app

# CMakeLists.txt와 소스 파일을 컨테이너로 복사
COPY . .
# CMake를 사용하여 빌드 디렉토리 생성 및 빌드
RUN mkdir -p build
WORKDIR /app/build
RUN cd /app/build
# CMake 캐시 파일 삭제
RUN rm -f CMakeCache.txt
RUN cmake -DCMAKE_CXX_COMPILER=clang++ ..
RUN make

# 실행 파일을 기본 명령어로 설정
CMD ["/app/build/ExampleMain"]
#CMD ["/app/build/ExampleOptional"]