#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MSG_SIZE 256

struct msgBuffer {
    long msgType;
    char msgText[MSG_SIZE];
};

int main() {
    key_t key;
    int msgID;
    struct msgBuffer message;

    // 메시지 큐 생성
    key = ftok("progfile", 65);
    msgID = msgget(key, 0666 | IPC_CREAT);

    // 메시지 수신
    msgrcv(msgID, &message, sizeof(message), 1, 0);

    printf("수신된 메시지: %s\n", message.msgText);

    return 0;
}

