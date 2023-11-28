#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid;

    // fork() 함수를 사용하여 새로운 프로세스 생성
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "프로세스 생성 실패\n");
        return 1;
    } else if (pid == 0) {
        // 자식 프로세스일 때
        printf("자식 프로세스 - PID: %d\n", getpid());
        printf("부모 프로세스 - PID: %d\n", getppid());
    } else {
        // 부모 프로세스일 때
        printf("부모 프로세스 - PID: %d\n", getpid());
        printf("새로운 자식 프로세스 생성 - PID: %d\n", pid);
    }

    return 0;
}

