#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

int main() {
    DIR *directory;
    struct dirent *file;

    // 현재 디렉토리 열기
    directory = opendir(".");

    if (directory != NULL) {
        printf("Files and directories in current directory:\n");

        // 디렉토리 내의 파일과 디렉토리 정보 출력
        while ((file = readdir(directory)) != NULL) {
            printf("%s\n", file->d_name);
        }

        closedir(directory);
    } else {
        perror("Error while opening directory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

