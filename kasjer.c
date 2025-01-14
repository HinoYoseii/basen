#include "utility.c"

int main() {
    srand(time(NULL));

    struct msgbuf msg;
    int msgID;
    key_t msg_key;
    time_t now;
    struct tm *local;

    if ((msg_key = ftok(".", 'M')) == -1) {
        printf("Blad ftok M (kasjer)");
        exit(EXIT_FAILURE);
    }
    if ((msgID = msgget(msg_key, IPC_CREAT | 0666)) == -1) {
        perror("Blad msgget msgID (kasjer)");
        exit(EXIT_FAILURE);
    }

    local = czas();
    printf("%s[%02d:%02d:%02d  %d] Kasjer: Oczekiwanie na komunikaty...%s\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
    sleep(5);

    while (1) { 
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
        msg.mtype = msg.pid;

        if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
            perror("Blad msgsnd msgID (kasjer)");
            exit(EXIT_FAILURE);
        }

        sleep(rand() % 2 + 1);
    }
    return 0;
}