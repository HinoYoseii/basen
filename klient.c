#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#define SHM_SIZE 12
// Struktura komunikatu
struct msgbuf {
    long mtype; // Typ komunikatu
    int pid;  // PID procesu klienta
};

int main() {
    time_t now;
    struct tm *local;
    int msgid;
    struct msgbuf message;
    int shmID, msgID;
    key_t msg_key, shm_key;

    // Uzyskaj dostęp do kolejki komunikatów
    if ((msg_key = ftok(".", 'A')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
    }
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    if (msgID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((shm_key = ftok(".", 'B')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
    }
    shmID = shmget(shm_key, SHM_SIZE * sizeof(int), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    time(&now);
    local = localtime(&now);

    printf("[%02d:%02d:%02d] Wygenerowano klienta: %d\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());

    return 0;
}
