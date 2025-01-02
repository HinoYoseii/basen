/* zarzadca.c */
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h> 
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#define SHM_SIZE 12
#define MAX_PROCESSES 10

// Struktura komunikatu
struct msgbuf {
    long mtype; // Typ komunikatu
    int pid;  // PID procesu klienta
};

int shmID, msgID;
int *shared_mem;

void koniec(int sig) {
    msgctl(msgID, IPC_RMID, NULL);
    shmctl(shmID, IPC_RMID, NULL);
    printf("MAIN - funkcja koniec sygnal %d: Koniec.\n", sig);
    exit(1);
}


int main() {
    
    struct sigaction act;
    act.sa_handler = koniec;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);

    key_t msg_key, shm_key;

    // Utwórz kolejkę komunikatów
    if ((msg_key = ftok(".", 'A')) == -1) {
        printf("Blad ftok A(main)\n");
    }
    msgID = msgget(msg_key, IPC_CREAT |  IPC_EXCL  | 0666);
    if (msgID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Kolejka komunikatów utworzona. MSGID: %d\n", msgID);

    // Utwórz pamięć dzieloną
    if ((shm_key = ftok(".", 'B')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
    }
    shmID = shmget(shm_key, SHM_SIZE * sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (shmID == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Mapuj pamięć dzieloną
    shared_mem = (int *)shmat(shmID, NULL, 0);
    printf("Pamięć dzielona utworzona. SHMID: %d\n", shmID);

    // Inicjalizacja pamięci dzielonej
    // memset(shared_mem, 0, SHM_SIZE);

    //uruchomienie kasjera
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Proces potomny - uruchom `kasjer`
        execl("./kasjer", "kasjer", NULL);
        perror("execl");
        exit(EXIT_FAILURE); // Jeśli execl się nie powiedzie
    }

    // Inicjalizuj generator liczb losowych
    srand(time(NULL));

    for (int i = 0; i < MAX_PROCESSES; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proces potomny - uruchom klienta
            execl("./klient", "klient", NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        } else {
            // Proces macierzysty - odczekaj losowy czas (1-5 sekund)
            sleep(rand() % 5 + 1);
        }
    }

    //printf("Zarządca zakończył tworzenie procesów.\n");

    // Odłącz pamięć dzieloną
    // if (shmdt(shared_mem) == -1) {
    //     perror("shmdt");
    // }

    //for (i = 0; i < 2 * P; i++)
    //    wait(NULL);

    msgctl(msgID, IPC_RMID, NULL);
    shmctl(shmID, IPC_RMID, NULL);
    printf("MAIN: Koniec.\n");
    return 0;
}
