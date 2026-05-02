# C++ Learning Environment (C++17 / C++20 / C++23)
# Base: Miniforge (conda) - xeus-cling이 conda-forge로 가장 안정적으로 설치됩니다.
FROM condaforge/miniforge3:latest

LABEL maintainer="cpp-learning"
LABEL description="C++17/20/23 학습환경: Jupyter + xeus-cling + GCC-13/Clang + 네트워크 실습 도구"

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# ─────────────────────────────────────────────────────────────────────────────
# 1) 시스템 패키지: 최신 GCC/Clang, 네트워크 실습 도구, 빌드 도구
# ─────────────────────────────────────────────────────────────────────────────
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        gcc-12 g++-12 \
        clang-14 \
        cmake \
        make \
        ninja-build \
        gdb \
        valgrind \
        strace \
        ltrace \
        linux-perf \
        net-tools \
        iproute2 \
        iputils-ping \
        netcat-openbsd \
        tcpdump \
        curl \
        wget \
        git \
        vim \
        nano \
        htop \
        tree \
        pkg-config \
        libssl-dev \
        zlib1g-dev \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# 기본 컴파일러를 gcc-12 / g++-12 로 설정 (Ubuntu 22.04 기준 최신 안정 버전)
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100 \
    && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-12 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-12 100

# ─────────────────────────────────────────────────────────────────────────────
# 2) Conda 환경: Jupyter + xeus-cling (C++ 인터프리터 커널)
#    - xeus-cling은 C++14/17 노트북 셀 실행을 지원합니다.
#    - C++20/23 기능은 셀에서 system() 호출로 g++ 빌드/실행을 사용합니다.
# ─────────────────────────────────────────────────────────────────────────────
RUN mamba install -y -n base -c conda-forge \
        python=3.11 \
        jupyterlab \
        notebook \
        ipykernel \
        xeus-cling \
        cmake \
        nlohmann_json \
    && mamba clean -afy

# ─────────────────────────────────────────────────────────────────────────────
# 3) 작업 디렉토리 준비
# ─────────────────────────────────────────────────────────────────────────────
WORKDIR /workspace
RUN mkdir -p /workspace/notebooks /workspace/src /workspace/build

# Jupyter 설정 (토큰 없이 로컬 접속, 실습용)
RUN mkdir -p /root/.jupyter && \
    echo "c.ServerApp.ip = '0.0.0.0'" > /root/.jupyter/jupyter_server_config.py && \
    echo "c.ServerApp.port = 8888" >> /root/.jupyter/jupyter_server_config.py && \
    echo "c.ServerApp.open_browser = False" >> /root/.jupyter/jupyter_server_config.py && \
    echo "c.ServerApp.token = ''" >> /root/.jupyter/jupyter_server_config.py && \
    echo "c.ServerApp.password = ''" >> /root/.jupyter/jupyter_server_config.py && \
    echo "c.ServerApp.allow_root = True" >> /root/.jupyter/jupyter_server_config.py && \
    echo "c.ServerApp.root_dir = '/workspace'" >> /root/.jupyter/jupyter_server_config.py

# 컴파일러 버전 확인용 헬퍼 스크립트
RUN echo '#!/bin/bash\n\
echo "=== C++ Compiler Versions ==="\n\
g++ --version | head -1\n\
clang++ --version | head -1\n\
echo ""\n\
echo "=== Standard Support ==="\n\
echo "C++17: $(echo "int main(){}" | g++ -std=c++17 -x c++ - -o /tmp/t17 && echo OK || echo FAIL)"\n\
echo "C++20: $(echo "int main(){}" | g++ -std=c++20 -x c++ - -o /tmp/t20 && echo OK || echo FAIL)"\n\
echo "C++23: $(echo "int main(){}" | g++ -std=c++2b -x c++ - -o /tmp/t23 && echo OK || echo FAIL)"\n\
' > /usr/local/bin/check-cpp && chmod +x /usr/local/bin/check-cpp

EXPOSE 8888

# 기본 명령: JupyterLab 실행
CMD ["jupyter", "lab", "--config=/root/.jupyter/jupyter_server_config.py"]
