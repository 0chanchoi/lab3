#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LENGTH 1000

int main() {
    char sentence[MAX_LENGTH];
    char input[MAX_LENGTH];
    int wrongTyping = 0;
    int charactersTyped = 0;
    float timeElapsed;
    clock_t start, end;

    printf("타자 연습을 시작합니다.\n");
    printf("문장을 입력하세요: ");
    fgets(sentence, sizeof(sentence), stdin);

    // 줄 바꿈 문자 제거
    sentence[strcspn(sentence, "\n")] = 0;

    printf("\n입력할 문장: %s\n\n", sentence);
    printf("타이핑을 시작합니다...\n");

    start = clock();

    do {
        printf("입력: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // 줄 바꿈 문자 제거

        for (int i = 0; i < strlen(input); i++) {
            if (input[i] != sentence[i]) {
                wrongTyping++;
            }
            charactersTyped++;
        }
    } while (strcmp(sentence, input) != 0);

    end = clock();

    timeElapsed = ((float)(end - start)) / CLOCKS_PER_SEC;
    float averageTypingSpeed = (charactersTyped / 5) / (timeElapsed / 60); // 한 문장 당 평균 단어 수 계산

    printf("\n타자 연습이 완료되었습니다!\n");
    printf("잘못 타이핑한 횟수: %d\n", wrongTyping);
    printf("총 타이핑한 문자 수: %d\n", charactersTyped);
    printf("소요된 시간: %.2f 초\n", timeElapsed);
    printf("평균 분당 타자수: %.2f\n", averageTypingSpeed);

    return 0;
}

