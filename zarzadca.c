#include "utility.c"

int msgID, msgrID;
pid_t pid_kasjer, pid_ratownik[3];

void koniec(int sig);
void clean();

int main() {
    srand(time(NULL));
    struct sigaction act;
    act.sa_handler = koniec;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
    
    pid_t pid;
    key_t msg_key, msg_key2;
    time_t zamkniecie = time(NULL) + DLUGOSC_OTWARCIA;

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

    // // Utwórz pamięć dzieloną
    // key_t shm_key;
    // int shm_ID;
    // if ((shm_key = ftok(".", 'S')) == -1) {
    //     printf("Blad ftok S (zarzadca)\n");
    //     exit(EXIT_FAILURE);
    // }
    // if ((shmID = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | IPC_EXCL | 0666)) == -1) {
    //     perror("Blad shmget shmID (zarzadca)");
    //     exit(EXIT_FAILURE);
    // }

    // printf("%sPamięć dzielona utworzona. SHMID: %d%s\n", GREEN, shmID, RESET);
    // printf("\n");

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

    char pool_id[10];
    for (int i = 0; i < 3; i++) {
        pid_ratownik[i] = fork();

        if (pid_ratownik[i] == -1) {
            perror("Blad fork pid_ratownik (zarzadca)");
            exit(EXIT_FAILURE);
        } else if (pid_ratownik[i] == 0) {
            // Konwertuj numer basenu na string i przekaż jako argument do execl
            sprintf(pool_id, "%d", i + 1);
            execl("./ratownik", "ratownik", pool_id, NULL);

            // Jeśli execl nie zadziała, zgłoś błąd
            perror("Blad execl pid_ratownik (zarzadca)");
            exit(EXIT_FAILURE);
        }
    }
    sleep(3);
    printf("\n");
    int dzieci = 0;
    //uruchomienie klientów
    while (time(NULL) < zamkniecie) {
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
            // continue;
            dzieci++;
            sleep(rand() % 6 + 1);
        }
    }
    // for (int i = 0; i < 5; i++) {
    //     pid_t pid_klient = fork();
    
    //     if (pid_klient == -1) {
    //         perror("Blad fork pid_klient (zarzadca)");
    //         exit(EXIT_FAILURE);
    //     } else if (pid_klient == 0) {
    //         // Proces potomny - uruchom klienta
    //         execl("./klient", "klient", NULL);
    //         perror("Blad execl pid_klient (zarzadca)");
    //         exit(EXIT_FAILURE);
            
    //     } else {
    //         // continue;
    //         dzieci++;
    //         sleep(rand() % 15 + 1);
    //     }
    // }
    printf("dzieci: %d\n", dzieci);
    for (int i = 0; i < dzieci; i++) {
        int status;
        pid_t finished_pid = wait(&status);
        if (finished_pid == -1) {
            perror("Błąd wait");
            exit(EXIT_FAILURE);
        }
        //printf("%sProces o PID %d zakończył się. Status: %d%s\n",YELLOW, finished_pid, WEXITSTATUS(status), RESET);
    }

    for(int i = 0; i < 3; i++){
        kill(pid_ratownik[i], SIGTERM);
    }
    kill(pid_kasjer, SIGTERM);
    msgctl(msgrID, IPC_RMID, NULL);
    msgctl(msgID, IPC_RMID, NULL);
    //shmctl(shmID, IPC_RMID, NULL);
    printf("MAIN: Koniec.\n");
    return 0;
}


void koniec(int sig) {
    for(int i = 0; i < 3; i++){
        kill(pid_ratownik[i], SIGTERM);
    }
    kill(pid_kasjer, SIGTERM);
    msgctl(msgrID, IPC_RMID, NULL);
    msgctl(msgID, IPC_RMID, NULL);
    //shmctl(shmID, IPC_RMID, NULL);
    printf("MAIN - funkcja koniec sygnal %d: Koniec.\n", sig);
    exit(1);
}