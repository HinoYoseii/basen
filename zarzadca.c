#include "utility.c"

int msgID, msgrID, shmID;   // ID do kolejek i pamięci
int dzieci = 0; // Licznik procesów potomnych klientów
pid_t pid_kasjer, pid_ratownik[3];  // Pid do zamknięcia procesów
pthread_t czyszczenie;
pthread_mutex_t mutex_dzieci;

volatile sig_atomic_t zakoncz_watek = 0; // flaga do zakończenia wątku czyszczenia

void *czyszczenie_procesow();
void koniec(int sig);

int main() {
    srand(time(NULL));

    // Obsługa sygnału crtl + c
    struct sigaction act;
    act.sa_handler = koniec;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);

    key_t msg_key, msgr_key, shm_key;   // Klucze do kolejek i pamięci
    struct tm *local;   // Wskaźnik do wyświetlania obecnego czasu
    struct shared_mem *shared_data; // Wskaźnik na strukture pamięci współdzielonej

    // Dane wprowadzane przez uzytkownika
    printf("Podaj rozmiary basenów. Wpisz liczbę a następnie kliknij ENTER.\n");
    int X[3];
    for (int i = 0; i < 3; i++) {
        while (1) {
            printf("[X%d]: ", i + 1);
            char term;
            if (scanf("%d%c", &X[i], &term) != 2 || term != '\n' || X[i] <= 0) {
                printf("%sPodano błędną wartość. Rozmiar basenu musi być liczbą całkowitą dodatnią.%s\n", RED, RESET);
                while (getchar() != '\n'); // Czyszczenie bufora
            } else if (i == 2 && X[i] % 2 != 0) {
                printf("%sPodano błędną wartość. Rozmiar basenu nr.3 musi być liczbą całkowitą parzystą.%s\n", RED, RESET);
                while (getchar() != '\n'); // Czyszczenie bufora
            } else {
                printf("%sZapisano dane ;)%s\n", GREEN, RESET);
                break; // Wyjście z pętli po poprawnym wprowadzeniu
            }
        }
    }


    printf("Podaj dlugosc otwarcia basenu w sekundach (od 60 sek. do 8 godzin) a następnie kliknij ENTER.\n");
    int dlugosc_otwarcia;
    while(1){
        printf("Dlugość otwarcia: ");
        char term;
        if(scanf("%d%c", &dlugosc_otwarcia, &term) != 2 || term != '\n' || (dlugosc_otwarcia < 60 || dlugosc_otwarcia > 28800)){
            printf("%sPodano błędną wartość. Długość otwarcia musi być liczbą całkowitą większą bądź równą 60 sekund i mniejszą od 8h.%s\n", RED, RESET);
            while (getchar() != '\n'); // Czyszczenie bufora
        }
        else{
            printf("%sZapisano dane ;)%s\n", GREEN, RESET);
            break; // Wyjście z pętli po poprawnym wprowadzeniu
        }
    }

    // Utworzenie kolejki komunikatów klient <-> kasjer
    msg_key = ftok(".", 'M');
    sprawdz_blad(msg_key, "ftok M (zarzadca)");
    msgID = msgget(msg_key, IPC_CREAT | IPC_EXCL | 0666);
    sprawdz_blad(msgID, "msgget msgID (zarzadca)");

    printf("%s\nKolejka komunikatów utworzona. MSGID: %d%s\n", CYAN, msgID, RESET);

    // Utworzenie kolejki komunikatów klient <-> ratownik
    msgr_key = ftok(".", 'R');
    sprawdz_blad(msgr_key, "ftok R (zarzadca)");
    msgrID = msgget(msgr_key, IPC_CREAT | IPC_EXCL | 0666);
    sprawdz_blad(msgrID, "msgget msgrID (zarzadca)");

    printf("%sKolejka komunikatów utworzona. MSGRID: %d%s\n", CYAN, msgrID, RESET);

    // Utworzenie fragmentu pamięci współdzielonej
    shm_key = ftok(".", 'S');
    sprawdz_blad(shm_key, "ftok S (zarzadca)");
    shmID = shmget(shm_key, sizeof(struct shared_mem), IPC_CREAT | IPC_EXCL | 0666);
    sprawdz_blad(shmID, "shmget shmID (zarzadca)");
    shared_data = shmat(shmID, NULL, 0);
    // Przypisanie zmiennej w pamięci współdzielonej
    shared_data->dlugosc_otwarcia = dlugosc_otwarcia;
    shared_data->otwarcie = time(NULL);
    shared_data->zamkniecie = shared_data->otwarcie + dlugosc_otwarcia;

    printf("%sPamięć współdzielona utworzona. SHMID: %d%s\n\n", CYAN, shmID, RESET);

    // Uruchomienie kasjera
    pid_kasjer = fork();
    sprawdz_blad(pid_kasjer, "Blad fork pid_kasjer (zarzadca)");

    if (pid_kasjer == 0) {
        execl("./kasjer", "kasjer", NULL);
        sprawdz_blad(-1, "Blad execl pid_kasjer (zarzadca)");
    }

    // Uruchomienie ratowników
    for (int i = 0; i < 3; i++) {
        pid_ratownik[i] = fork();
        sprawdz_blad(pid_ratownik[i], "Blad fork pid_ratownik (zarzadca)");

        if (pid_ratownik[i] == 0) {
            char pool_id[2];
            char pool_size[16];
            sprintf(pool_id, "%d", i + 1);
            sprintf(pool_size, "%d", X[i]);
            execl("./ratownik", "ratownik", pool_id, pool_size, NULL);
            sprawdz_blad(-1, "Blad execl pid_ratownik (zarzadca)");
        }
    }

    // Inicjalizacja mutexa i wątku do czyszczenia procesów zombie
    sprawdz_blad_watek(pthread_mutex_init(&mutex_dzieci, NULL), "pthread_mutex_init ratownik 1 - mutex olimpijski");
    sprawdz_blad_watek(pthread_create(&czyszczenie, NULL, &czyszczenie_procesow, NULL), "pthread_create ratownik 1 - wątek t_wpuszczanie");

    sleep(3); // Dla estetyki w konsoli, można zakomentować

    while (time(NULL) < shared_data->zamkniecie && dzieci <= MAX_CLIENTS) {
        pid_t pid_klient = fork();
        sprawdz_blad(pid_klient, "Blad fork pid_klient (zarzadca)");

        if (pid_klient == 0) {
            execl("./klient", "klient", NULL);
            sprawdz_blad(-1, "Blad execl pid_klient (zarzadca)");
        } else {
            pthread_mutex_lock(&mutex_dzieci);
            dzieci++;
            pthread_mutex_unlock(&mutex_dzieci);
            sleep(rand() % 5 + 1);
        }
    }

    // Oczekiwaie na zakończenie wątku czyszczenia i zniszczenie mutexa
    zakoncz_watek = 1;
    sprawdz_blad_watek(pthread_join(czyszczenie, NULL), "pthread_join zarzadca - wątek czyszczenie");
    sprawdz_blad_watek(pthread_mutex_destroy(&mutex_dzieci), "pthread_mutex_destroy zarzadca - mutex dzieci");

    // Oczekiwanie na zakończenie reszty klientów jeżeli jest taka potrzeba
    local = czas();
    for (int i = 0; i < dzieci; i++) {
        int status;
        pid_t finished_pid = wait(&status);
        sprawdz_blad(finished_pid, "Błąd wait");

        //printf("%sProces o PID %d zakończył się. Status: %d%s\n", YELLOW, finished_pid, WEXITSTATUS(status), RESET);
    }

    printf("%s[%02d:%02d:%02d]%s ZAMKNIĘCIE KOMPLEKSU\n", RED, local->tm_hour, local->tm_min, local->tm_sec, RESET);

    for (int i = 0; i < 3; i++) {
        sprawdz_blad(kill(pid_ratownik[i], SIGTERM), "Błąd kill pid_ratownik (zarzadca) - koniec symulacji");
    }
    sprawdz_blad(kill(pid_kasjer, SIGTERM), "Błąd kill pid_kasjer (zarzadca) - koniec symulacji");
    sprawdz_blad(msgctl(msgrID, IPC_RMID, NULL), "Błąd msgctl msgrID (zarzadca) - koniec symulacji");
    sprawdz_blad(msgctl(msgID, IPC_RMID, NULL), "Błąd msgctl msgID (zarzadca) - koniec symulacji");
    sprawdz_blad(shmctl(shmID, IPC_RMID, NULL), "Błąd shmctl IPC_RMID (zarzadca) - koniec symulacji");
    printf("ZARZADCA: Koniec.\n");
    return 0;

}
void koniec(int sig) {
    for (int i = 0; i < 3; i++) {
        sprawdz_blad(kill(pid_ratownik[i], SIGTERM), "Błąd kill pid_ratownik (zarzadca) - crtl + c");
    }
    sprawdz_blad(kill(pid_kasjer, SIGTERM), "Błąd kill pid_kasjer (zarzadca) - crtl + c");
    sprawdz_blad(msgctl(msgrID, IPC_RMID, NULL), "Błąd msgctl msgrID (zarzadca) - crtl + c");
    sprawdz_blad(msgctl(msgID, IPC_RMID, NULL), "Błąd msgctl msgID (zarzadca) - crtl + c");
    sprawdz_blad(shmctl(shmID, IPC_RMID, NULL), "Błąd shmctl IPC_RMID (zarzadca) - crtl + c");

    printf("ZARZADCA - funkcja koniec sygnal %d: Koniec.\n", sig);
    exit(1);
}

void *czyszczenie_procesow(void *arg) {
    // czyszczenie procesów zombie
    while (!zakoncz_watek) {
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);
        if (finished_pid > 0) {
            pthread_mutex_lock(&mutex_dzieci);
            dzieci--;
            pthread_mutex_unlock(&mutex_dzieci);
            // printf("Proces klienta o PID %d zakończony. Status: %d\n", finished_pid, WEXITSTATUS(status));
        } else if (finished_pid == 0) {
            // Brak zakończonych procesów odczekuje chwile
            usleep(100000); // 100ms
        }
    }
    return NULL;
}