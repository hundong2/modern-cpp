# zero-copy의 여러 의미

“zero-copy”는 문맥에 따라 다르다. C++ 객체 이동은 보통 소유 핸들을 넘겨 **애플리케이션 객체의 deep copy**를 피한다. OS zero-copy(`sendfile`, DMA, buffer registration 등)는 커널/사용자 공간 사이 데이터 복사를 줄이는 I/O 기법이다. 이동 생성자를 썼다고 네트워크 경로가 zero-copy가 되는 것은 아니다.
