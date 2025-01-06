#include "utility.c"
#define SEKUNDA 5000

int shmID, msgID, msgrID, shmtID;
pid_t pid_kasjer, pid_ratownik;

volatile bool stop_time;
pthread_t t_czasomierz;
char* shm_czas_adres;

void koniec(int sig);
void *czasomierz();

int main() {
    srand(time(NULL));
    stop_time = false;

    struct sigaction act;
    act.sa_handler = koniec;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
    
    pid_t pid;
    key_t msg_key, msg_key2, shm_key, shmt_key;

    // Utwórz kolejkę komunikatów
    if ((msg_key = ftok(".", 'M')) == -1) {
        printf("Blad ftok A(main)\n");
    }
    msgID = msgget(msg_key, IPC_CREAT |  IPC_EXCL  | 0666);
    if (msgID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Kolejka komunikatów utworzona. MSGID: %d\n", msgID);

    if ((msg_key2 = ftok(".", 'R')) == -1) {
        printf("Blad ftok A(main)\n");
    }
    msgrID = msgget(msg_key2, IPC_CREAT |  IPC_EXCL  | 0666);
    if (msgrID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Kolejka komunikatów utworzona. MSGRID: %d\n", msgrID);

    // Utwórz pamięć dzieloną
    if ((shm_key = ftok(".", 'S')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
    }
    shmID = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | IPC_EXCL | 0666);
    if (shmID == -1) {
        perror("shmget zarzadca");
        exit(EXIT_FAILURE);
    }

    printf("Pamięć dzielona utworzona. SHMID: %d\n", shmID);

    if ((shmt_key = ftok(".", 'T')) == -1) {
        printf("Blad ftok T(main)\n");
        exit(1);
    }
    shmtID = shmget(shmt_key, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (shmtID == -1) {
        perror("shmget zarzadca");
        exit(EXIT_FAILURE);
    }

    printf("Pamięć dzielona utworzona. SHMTID: %d\n\n", shmID);

    shm_czas_adres = (char*)shmat(shmtID, NULL, 0);
    if (shm_czas_adres == (char*)(-1))
    {
        perror("shmat - problem z dolaczeniem pamieci do obslugi czasu");
        exit(EXIT_FAILURE);
    }

    *shm_czas_adres = 0;
    pthread_create(&t_czasomierz, NULL, &czasomierz, NULL);

    pid_ratownik = fork();

    if (pid_ratownik == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid_ratownik == 0) {
        execl("./ratownik", "ratownik", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
    sleep(2);

    //uruchomienie kasjera
    pid_kasjer = fork();

    if (pid_kasjer == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid_kasjer == 0) {
        execl("./kasjer", "kasjer", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
    sleep(2);

    //uruchomienie klientów
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
            sleep(rand() % 3 + 1);
        }
    }

    //printf("Zarządca zakończył tworzenie procesów.\n");

    // Odłącz pamięć dzieloną
    // if (shmdt(shared_mem) == -1) {
    //     perror("shmdt");
    // }

    //for (i = 0; i < 2 * P; i++)
    //    wait(NULL);
    stop_time = true;
    pthread_join(t_czasomierz, NULL);
    kill(pid_kasjer, SIGTERM);
    msgctl(msgrID, IPC_RMID, NULL);
    msgctl(msgID, IPC_RMID, NULL);
    shmctl(shmID, IPC_RMID, NULL);
    shmctl(shmtID, IPC_RMID, NULL);
    printf("MAIN: Koniec.\n");
    return 0;
}


void koniec(int sig) {
    stop_time = true;
    kill(pid_kasjer, SIGTERM);
    msgctl(msgrID, IPC_RMID, NULL);
    msgctl(msgID, IPC_RMID, NULL);
    shmctl(shmID, IPC_RMID, NULL);
    shmctl(shmtID, IPC_RMID, NULL);
    printf("MAIN - funkcja koniec sygnal %d: Koniec.\n", sig);
    exit(1);
}

void *czasomierz()
{
    int *jaki_czas = (int *)shm_czas_adres;
    while (*jaki_czas < 44100 && !stop_time)
    {
        usleep(SEKUNDA);
        (*jaki_czas)++;
    }

    // kill(pid_kasjer, SIGINT);
    return 0;
}
