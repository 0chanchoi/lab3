#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int pipefd[2]; // 파이프용 파일 디스크립터 배열
    char buffer[BUFFER_SIZE];
    pid_t pid;
    int status;

    if (pipe(pipefd) == -1) {
        perror("파이프 생성 실패");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid < 0) {
        perror("프로세스 생성 실패");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // 부모 프로세스
        close(pipefd[0]); // 부모 프로세스는 파이프의 읽기를 닫음

        printf("부모 프로세스가 데이터를 파이프로 전송합니다.\n");
        char message[] = "Hello, child process!";
        write(pipefd[1], message, sizeof(message));
        close(pipefd[1]);

        waitpid(pid, &status, 0); // 자식 프로세스 종료 대기
    } else {
        // 자식 프로세스
        close(pipefd[1]); // 자식 프로세스는 파이프의 쓰기를 닫음

        printf("자식 프로세스가 파이프로부터 데이터를 수신합니다.\n");
        read(pipefd[0], buffer, sizeof(buffer));
        printf("수신된 데이터: %s\n", buffer);
        close(pipefd[0]);

        exit(EXIT_SUCCESS);
    }

    return 0;
}

