#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// 시그널 핸들러 함수
void signalHandler(int signal) {
    printf("시그널 %d 받음 (Ctrl+C)\n", signal);
    exit(signal); // 시그널 번호를 리턴하여 프로그램 종료
}

int main() {
    // SIGINT 시그널에 대한 핸들러 등록
    signal(SIGINT, signalHandler);

    printf("Ctrl+C를 눌러보세요 (시그널 SIGINT)\n");

    // 프로그램이 종료될 때까지 대기
    while (1) {
        sleep(1);
    }

    return 0;
}

