#include "utility.c"
#include "utility.h"

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

    printf("Podaj rozmiary basenów. Wpisz liczbę a następnie kliknij ENTER.\n");
    int X[3];
    for(int i = 0; i < 3; i++){
        while(1){
            printf("[X%d]: ", i + 1);
            char term;
            if(scanf("%d%c", &X[i], &term) != 2 || term != '\n'){
                printf("Podano błędną wartość. Rozmiar basenu musi być liczbą całkowitą.\n");
                while (getchar() != '\n');
            }
            else if(i == 2 && X[i] % 2 != 0){
                printf("Podano błędną wartość. Rozmiar basenu nr.3 musi być liczbą całkowitą parzystą.\n");
                while (getchar() != '\n');
            }
            else
                break;
        }  
    }

    printf("Podaj dlugosc otwarcia basenu w sekundach a następnie kliknij ENTER.\n");
    int dlugosc_otwarcia;
    while(1){
        printf("Dlugość otwarcia: ");
        char term;
        if(scanf("%d%c", &dlugosc_otwarcia, &term) != 2 || term != '\n' || (dlugosc_otwarcia <= 0 && dlugosc_otwarcia > 28800)){
            printf("Podano błędną wartość. Długość otwarcia musi być liczbą całkowitą większa od 0 i mniejsza od 8h.\n");
            while (getchar() != '\n');
        }
        else
            break;
    }

    time_t zamkniecie;
    pid_t pid;
    key_t msg_key, msgr_key;
    
    struct tm *local;
    int dzieci = 0;

    msg_key = ftok(".", 'M');
    sprawdz_blad(msg_key, "ftok M (zarzadca)");
    msgID = msgget(msg_key, IPC_CREAT | IPC_EXCL | 0666);
    sprawdz_blad(msgID, "msgget msgID (zarzadca)");

    printf("%s\nKolejka komunikatów utworzona. MSGID: %d%s\n", CYAN, msgID, RESET);

    msgr_key = ftok(".", 'R');
    sprawdz_blad(msgr_key, "ftok R (zarzadca)");
    msgrID = msgget(msgr_key, IPC_CREAT | IPC_EXCL | 0666);
    sprawdz_blad(msgrID, "msgget msgrID (zarzadca)");

    printf("%sKolejka komunikatów utworzona. MSGRID: %d%s\n", CYAN, msgrID, RESET);
    
    while(1){
        printf("\n\n");
        local = czas();
        zamkniecie = time(NULL) + dlugosc_otwarcia;
        time_t okresowe_zamkniecie = zamkniecie - (dlugosc_otwarcia / 2);
        //time_t okresowe_otwarcie = okresowe_zamkniecie + (dlugosc_otwarcia / 10);

        char time_str[32]; 
        pid_kasjer = fork();
        sprawdz_blad(pid_kasjer, "Blad fork pid_kasjer (zarzadca)");

        if (pid_kasjer == 0) {
            sprintf(time_str, "%ld", (long)zamkniecie);
            execl("./kasjer", "kasjer", time_str, NULL);
            sprawdz_blad(-1, "Blad execl pid_kasjer (zarzadca)");
        }

        char pool_id[2];
        char pool_size[16];
        for (int i = 0; i < 3; i++) {
            pid_ratownik[i] = fork();
            sprawdz_blad(pid_ratownik[i], "Blad fork pid_ratownik (zarzadca)");

            if (pid_ratownik[i] == 0) {
                sprintf(pool_id, "%d", i + 1);
                sprintf(pool_size, "%d", X[i]);
                execl("./ratownik", "ratownik", pool_id, pool_size, NULL);
                sprawdz_blad(-1, "Blad execl pid_ratownik (zarzadca)");
            }
        }

        sleep(3);
        printf("\n");
        printf("%s[%02d:%02d:%02d]%s OTWARCIE KOMPLEKSU BASENOWEGO\n", RED, local->tm_hour, local->tm_min, local->tm_sec, RESET);

        while (time(NULL) < zamkniecie) {
            pid_t pid_klient = fork();
            sprawdz_blad(pid_klient, "Blad fork pid_klient (zarzadca)");

            if (pid_klient == 0) {
                execl("./klient", "klient", NULL);
                sprawdz_blad(-1, "Blad execl pid_klient (zarzadca)");
            } else {
                dzieci++;
                // if(time(NULL) >= okresowe_zamkniecie){
                //         for (int i = 0; i < dzieci; i++) {
                //         int status;
                //         pid_t finished_pid = wait(&status);
                //         sprawdz_blad(finished_pid, "Błąd wait");

                //         //printf("%sProces o PID %d zakończył się. Status: %d%s\n", YELLOW, finished_pid, WEXITSTATUS(status), RESET);
                //     }
                //     printf("%s[%02d:%02d:%02d]%s OKRESOWE ZAMKNIECIE KOMPLEKSU. NASTĘPUJE WYMIANA WODY\n", RED, local->tm_hour, local->tm_min, local->tm_sec, RESET);
                //     dzieci = 0;
                //     sleep(dlugosc_otwarcia / 10);
                // }
                sleep(rand() % 6 + 1);
            }
        }

        local = czas();
        for (int i = 0; i < dzieci; i++) {
            int status;
            pid_t finished_pid = wait(&status);
            sprawdz_blad(finished_pid, "Błąd wait");

            //printf("%sProces o PID %d zakończył się. Status: %d%s\n", YELLOW, finished_pid, WEXITSTATUS(status), RESET);
        }
        printf("%s[%02d:%02d:%02d]%s ZAMKNIECIE KOMPLEKSU\n", RED, local->tm_hour, local->tm_min, local->tm_sec, RESET);

        // local = czas();
        // printf("%s[%02d:%02d:%02d]%s WYMIANA WODY\n", RED, local->tm_hour, local->tm_min, local->tm_sec, RESET);
        
        for (int i = 0; i < 3; i++) {
            sprawdz_blad(kill(pid_ratownik[i], SIGTERM), "Błąd kill pid_ratownik (zarzadca)");
        }
        sprawdz_blad(kill(pid_kasjer, SIGTERM), "Błąd kill pid_kasjer (zarzadca)");

        sleep(dlugosc_otwarcia * 2);
    }

    for (int i = 0; i < 3; i++) {
        sprawdz_blad(kill(pid_ratownik[i], SIGTERM), "Błąd kill pid_ratownik (zarzadca)");
    }
    sprawdz_blad(kill(pid_kasjer, SIGTERM), "Błąd kill pid_kasjer (zarzadca)");
    sprawdz_blad(msgctl(msgrID, IPC_RMID, NULL), "Błąd msgctl msgrID (zarzadca)");
    sprawdz_blad(msgctl(msgID, IPC_RMID, NULL), "Błąd msgctl msgID (zarzadca)");

    printf("ZARZADCA: Koniec.\n");
    return 0;
}


void koniec(int sig) {
    for (int i = 0; i < 3; i++) {
        sprawdz_blad(kill(pid_ratownik[i], SIGTERM), "Błąd kill pid_ratownik (zarzadca)");
    }
    sprawdz_blad(kill(pid_kasjer, SIGTERM), "Błąd kill pid_kasjer (zarzadca)");
    sprawdz_blad(msgctl(msgrID, IPC_RMID, NULL), "Błąd msgctl msgrID (zarzadca)");
    sprawdz_blad(msgctl(msgID, IPC_RMID, NULL), "Błąd msgctl msgID (zarzadca)");

    printf("ZARZADCA - funkcja koniec sygnal %d: Koniec.\n", sig);
    exit(1);
}