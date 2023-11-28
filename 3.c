#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

void listFilesRecursively(char *basePath) {
    char path[1000];
    struct dirent *file;
    DIR *directory;

    if (!(directory = opendir(basePath))) {
        return;
    }

    while ((file = readdir(directory)) != NULL) {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            printf("%s/%s\n", basePath, file->d_name);

            if (file->d_type == DT_DIR) {
                sprintf(path, "%s/%s", basePath, file->d_name);
                listFilesRecursively(path);
            }
        }
    }

    closedir(directory);
}

int main() {
    printf("Files and directories in current directory:\n");
    listFilesRecursively(".");

    return 0;
}

