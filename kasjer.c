#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#define SHM_SIZE 12
// Struktura komunikatu
struct msgbuf {
    long mtype; // Typ komunikatu
    int pid;  // PID procesu klienta
};

int main() {
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

    printf("Kasjer [%d]: Oczekiwanie na komunikaty...\n", getpid());

    // while (1) {
    //     // Odczytaj komunikat z kolejki
    //     if (msgrcv(msgid, &message, sizeof(pid_t), 1, 0) == -1) {
    //         perror("msgrcv");
    //         exit(EXIT_FAILURE);
    //     }

    //     printf("Kasjer: Otrzymano PID klienta: %d\\n", message.pid);

    //     // Symulacja obsługi klienta
    //     printf("Kasjer: Obsługa klienta %d...\\n", message.pid);
    //     sleep(2); // Symulacja czasu obsługi

    //     printf("Kasjer: Obsługa klienta %d zakończona.\\n", message.pid);
    // }

    return 0;
}
