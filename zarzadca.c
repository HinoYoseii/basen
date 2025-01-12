#include "utility.c"

int shmID, msgID, msgrID;
pid_t pid_kasjer, pid_ratownik;

void koniec(int sig);

int main() {
    srand(time(NULL));

    struct sigaction act;
    act.sa_handler = koniec;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
    
    pid_t pid;
    key_t msg_key, msg_key2, shm_key;

    // Utwórz kolejkę komunikatów
    if ((msg_key = ftok(".", 'M')) == -1) {
        printf("Blad ftok M (zarzadca)");
        exit(EXIT_FAILURE);
    }
    if ((msgID = msgget(msg_key, IPC_CREAT |  IPC_EXCL  | 0666)) == -1) {
        perror("Blad msgget msgID (zarzadca)");
        exit(EXIT_FAILURE);
    }

    printf("%sKolejka komunikatów utworzona. MSGID: %d%s\n", GREEN, msgID, RESET);

    if ((msg_key2 = ftok(".", 'R')) == -1) {
        printf("Blad ftok R (zarzadca)\n");
        exit(EXIT_FAILURE);
    }
    if ((msgrID = msgget(msg_key2, IPC_CREAT |  IPC_EXCL  | 0666)) == -1) {
        perror("Blad msgget msgrID (zarzadca)");
        exit(EXIT_FAILURE);
    }

    printf("%sKolejka komunikatów utworzona. MSGRID: %d%s\n", GREEN, msgrID, RESET);

    // Utwórz pamięć dzieloną
    if ((shm_key = ftok(".", 'S')) == -1) {
        printf("Blad ftok S (zarzadca)\n");
        exit(EXIT_FAILURE);
    }
    if ((shmID = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        perror("Blad shmget shmID (zarzadca)");
        exit(EXIT_FAILURE);
    }

    printf("%sPamięć dzielona utworzona. SHMID: %d%s\n", GREEN, shmID, RESET);
    printf("\n");

    pid_ratownik = fork();

    if (pid_ratownik == -1) {
        perror("Blad fork pid_ratownik (zarzadca)");
        exit(EXIT_FAILURE);
    } else if (pid_ratownik == 0) {
        execl("./ratownik", "ratownik", NULL);
        perror("Blad execl pid_ratownik (zarzadca)");
        exit(EXIT_FAILURE);
    }
    //uruchomienie kasjera
    pid_kasjer = fork();

    if (pid_kasjer == -1) {
        perror("Blad fork pid_kasjer (zarzadca)");
        exit(EXIT_FAILURE);
    } else if (pid_kasjer == 0) {
        execl("./kasjer", "kasjer", NULL);
        perror("Blad execl pid_kasjer (zarzadca)");
        exit(EXIT_FAILURE);
    }
    sleep(2);
    printf("\n");
    //uruchomienie klientów
    for (int i = 0; i < MAX_PROCESSES; i++) {
        pid_t pid_klient = fork();

        if (pid_klient == -1) {
            perror("Blad fork pid_klient (zarzadca)");
            exit(EXIT_FAILURE);
        } else if (pid_klient == 0) {
            // Proces potomny - uruchom klienta
            execl("./klient", "klient", NULL);
            perror("Blad execl pid_klient (zarzadca)");
            exit(EXIT_FAILURE);
        } else {
            sleep(rand() % 2 + 1);
        }
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        int status;
        pid_t finished_pid = wait(&status);
        if (finished_pid == -1) {
            perror("Błąd wait");
            exit(EXIT_FAILURE);
        }
        //printf("Proces klienta o PID %d zakończył się. Status: %d\n", finished_pid, WEXITSTATUS(status));
    }

    kill(pid_ratownik, SIGTERM);
    kill(pid_kasjer, SIGTERM);
    msgctl(msgrID, IPC_RMID, NULL);
    msgctl(msgID, IPC_RMID, NULL);
    shmctl(shmID, IPC_RMID, NULL);
    printf("MAIN: Koniec.\n");
    return 0;
}

void koniec(int sig) {
    kill(pid_ratownik, SIGTERM);
    kill(pid_kasjer, SIGTERM);
    msgctl(msgrID, IPC_RMID, NULL);
    msgctl(msgID, IPC_RMID, NULL);
    shmctl(shmID, IPC_RMID, NULL);
    printf("MAIN - funkcja koniec sygnal %d: Koniec.\n", sig);
    exit(1);
}