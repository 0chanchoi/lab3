#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void executeCommand(char *command) {
    int status;

    // 새로운 프로세스 생성
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "프로세스 생성 실패\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 자식 프로세스
        system(command); // 시스템 명령 실행
        exit(EXIT_SUCCESS);
    } else {
        // 부모 프로세스
        waitpid(pid, &status, 0); // 자식 프로세스가 종료될 때까지 대기
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "사용법: %s 명령\n", argv[0]);
        return EXIT_FAILURE;
    }

    char command[1000] = "";
    for (int i = 1; i < argc; ++i) {
        strcat(command, argv[i]); // 명령어로부터 받은 각 단어를 command 문자열에 추가
        strcat(command, " ");
    }

    executeCommand(command); // 사용자 정의 함수 실행

    return EXIT_SUCCESS;
}

