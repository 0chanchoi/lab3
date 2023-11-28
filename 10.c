#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#define FILE_SIZE 1000
#define SEM_KEY 1234
#define SHM_KEY 5678

// 세마포어 구조체 정의
struct sembuf acquire = {0, -1, SEM_UNDO};
struct sembuf release = {0, 1, SEM_UNDO};

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int shmid, semid;
    key_t key_shm = SHM_KEY;
    key_t key_sem = SEM_KEY;

    int *shm_data; // 공유 메모리를 가리키는 포인터

    // 공유 메모리 생성 및 연결
    shmid = shmget(key_shm, sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1) {
        error("shmget");
    }

    // 공유 메모리를 프로세스 주소 공간에 첨부
    shm_data = shmat(shmid, NULL, 0);
    if (shm_data == (int *)(-1)) {
        error("shmat");
    }

    // 세마포어 생성 또는 연결
    semid = semget(key_sem, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        if (errno == EEXIST) {
            semid = semget(key_sem, 1, 0);
        } else {
            error("semget");
        }
    } else {
        // 세마포어 초기화
        if (semctl(semid, 0, SETVAL, 1) == -1) {
            error("semctl");
        }
    }

    pid_t pid = fork();

    if (pid < 0) {
        error("fork");
    } else if (pid == 0) { // 자식 프로세스 (쓰는 프로세스)
        FILE *src, *dest;
        char source_file[] = "/home/linux/lab3/source.txt"; // 복사할 파일 경로
        char dest_file[] = "/home/linux/lab3/destination.txt"; // 복사될 파일 경로
        char buffer[FILE_SIZE];

        // 세마포어를 이용한 잠금
        if (semop(semid, &acquire, 1) == -1) {
            error("semop");
        }

        // 파일 복사
        src = fopen(source_file, "r");
        if (src == NULL) {
            error("fopen source");
        }

        dest = fopen(dest_file, "w");
        if (dest == NULL) {
            error("fopen destination");
        }

        while (fgets(buffer, FILE_SIZE, src) != NULL) {
            fputs(buffer, dest);
        }

        fclose(src);
        fclose(dest);

        // 세마포어를 이용한 잠금 해제
        if (semop(semid, &release, 1) == -1) {
            error("semop");
        }

        exit(EXIT_SUCCESS);
    } else { // 부모 프로세스 (읽는 프로세스)
        wait(NULL); // 자식 프로세스의 종료를 기다림

        // 공유 메모리 및 세마포어 삭제
        if (shmdt(shm_data) == -1) {
            error("shmdt");
        }
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            error("shmctl");
        }
        if (semctl(semid, 0, IPC_RMID) == -1) {
            error("semctl");
        }
    }

    return 0;
}

