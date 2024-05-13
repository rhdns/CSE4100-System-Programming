MyShell : Phase1

MyShell은 C 언어로 구현된 간단한 UNIX 쉘이며 기본적인 쉘 작업을 처리하고 명령어를 실행

기능
    명령어 실행: /bin 디렉토리의 UNIX 명령어 실행 가능.
    내장 명령어: cd, exit, quit의 명령어 실행 가능.
    오류 처리: 알려지지 않은 명령어에 대한 기본 오류 처리 기능.

컴파일 방법
제공된 Makefile을 사용하여 쉘을 컴파일:

    make

컴파일 후, 쉘을 시작하려면 다음과 같이 실행:

    ./myShell

오브젝트 파일과 실행 파일의 제거:

    make clean