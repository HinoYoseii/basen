#include "utility.c"

int msgID, msgrID, shmID;   // ID do kolejek i pamięci
pid_t pid_kasjer, pid_ratownik[3];  // Pid do zamknięcia procesów

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

    //sleep(3); // Dla estetyki w konsoli, można zakomentować

    int dzieci = 0; // Licznik procesów potomnych klientów
    while (time(NULL) < shared_data->zamkniecie && dzieci <= MAX_CLIENTS) {
        pid_t pid_klient = fork();
        sprawdz_blad(pid_klient, "Blad fork pid_klient (zarzadca)");

        if (pid_klient == 0) {
            execl("./klient", "klient", NULL);
            sprawdz_blad(-1, "Blad execl pid_klient (zarzadca)");
        } else {
            dzieci++;
            sleep(rand() % 5 + 1);
        }
    }

    // Oczekiwanie na zakończenie klientów
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