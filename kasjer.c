#include "utility.c"

int main(int argc, char *argv[]) {
    srand(time(NULL));

    key_t msg_key, shm_key;  // Klucz kolejki komunikatów
    int msgID, shmID;  // ID kolejki komunikatów
    time_t zamkniecie; // godzina zamknięcia basenu
    struct msgbuf msg;  // Bufor do kolejki komunikatów
    struct tm *local;   // Wskaźnik do wyświetlania obecnego czasu
    struct shared_mem *shared_data;

    // Dołączenie do kolejki komunikatów klient <-> kasjer
    msg_key = ftok(".", 'M');
    sprawdz_blad(msg_key, "ftok M (klient)");
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    sprawdz_blad(msgID, "msgget msgID (kasjer)");

    shm_key = ftok(".", 'S');
    sprawdz_blad(shm_key, "ftok S (zarzadca)");
    shmID = shmget(shm_key, sizeof(struct shared_mem), IPC_CREAT | 0666);
    sprawdz_blad(shmID, "shmget shmID (zarzadca)");
    shared_data = shmat(shmID, NULL, 0);
    zamkniecie = time(NULL) + shared_data->dlugosc_otwarcia;

    printf("%sKasjer [%d]%s Oczekiwanie na komunikaty...\n", BLUE, getpid(), RESET);

    while (1) { 
        // Odbieranie komunikatów typu 1 i 2 od klienta
        sprawdz_blad(msgrcv(msgID, &msg, sizeof(msg) - sizeof(long), -2, 0), "Blad msgrcv msgID (kasjer) - odebranie komunikatu od klienta");

        // Sprawdzenie typu klienta
        local = czas();
        if (msg.mtype == 1){
            printf("%s[%02d:%02d:%02d  %d]%s Kasjer obsługuje klienta VIP.\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET);
        }
        if (msg.mtype == 2){
            printf("%s[%02d:%02d:%02d  %d]%s Kasjer obsługuje klienta.\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET);
        }
        
        // Sprawdzenie wieku klienta i płacenie za bilet
        local = czas();
        if(msg.wiek < 10){
            printf("%s[%02d:%02d:%02d  %d]%s Opiekun płaci za bilet. Dziecko nie płaci za bilet. Wiek: %d\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET, msg.wiek);
        }
        else{
            printf("%s[%02d:%02d:%02d  %d]%s Klient płaci za bilet.\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET);
        }
        
        // Ustawienie czasu wyjścia klienta
        msg.czas_wyjscia = time(NULL) + 20;

        if (msg.czas_wyjscia > zamkniecie){
            msg.czas_wyjscia = zamkniecie - 5;
        }

        // Ustawienie typu komunikatu na pid obsługiwanego klienta
        msg.mtype = msg.pid;

        // Wysłanie komunikatu do klienta
        sprawdz_blad(msgsnd(msgID, &msg, sizeof(msg) - sizeof(long), 0), "Blad msgrcv msgID (kasjer) - wysłanie komunikatu do klienta");
    }

    return 0;
}