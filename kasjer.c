#include "utility.c"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Brak podanego czasu!\n");
        exit(EXIT_FAILURE);
    }

    time_t zamkniecie = (time_t)strtol(argv[1], NULL, 10);
    srand(time(NULL));

    struct msgbuf msg;
    int msgID;
    key_t msg_key;
    time_t now;
    struct tm *local;

    msg_key = ftok(".", 'M');
    sprawdz_blad(msg_key, "ftok M (klient)");
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    sprawdz_blad(msgID, "msgget msgID (zarzadca)");

    local = czas();
    printf("%sKasjer [%d]%s Oczekiwanie na komunikaty...\n", BLUE, getpid(), RESET);

    while (time(NULL) < zamkniecie) { 
        if (msgrcv(msgID, &msg, sizeof(msg), -2, 0) == -1) {
            perror("Blad msgrcv msgID (kasjer)");
            exit(EXIT_FAILURE);
        }

        local = czas();
        if (msg.mtype == 1){
            printf("%s[%02d:%02d:%02d  %d]%s Kasjer obsługuje klienta VIP.\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET);
        }
        if (msg.mtype == 2){
            printf("%s[%02d:%02d:%02d  %d]%s Kasjer obsługuje klienta.\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET);
        }
        
        local = czas();
        if(msg.wiek < 10){
            printf("%s[%02d:%02d:%02d  %d]%s Opiekun płaci za bilet. Dziecko nie płaci za bilet. Wiek: %d\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET, msg.wiek);
        }
        else{
            printf("%s[%02d:%02d:%02d  %d]%s Klient płaci za bilet.\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, msg.pid, RESET);
        }
        
        msg.czas_wyjscia = time(NULL) + 20;

        if (msg.czas_wyjscia > zamkniecie){
            msg.czas_wyjscia = zamkniecie;
        }

        msg.mtype = msg.pid;

        if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
            perror("Blad msgsnd msgID (kasjer)");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}