#include "utility.c"

int main() {
    srand(time(NULL));
    struct msgbuf msg;
    int msgrID;
    key_t msg2_key;

    time_t now;
    struct tm *local;

    if ((msg2_key = ftok(".", 'R')) == -1) {
        printf("Blad ftok R(ratownik)");
        exit(EXIT_FAILURE);
    }

    if ((msgrID = msgget(msg2_key, IPC_CREAT | 0666)) == -1) {
        perror("Blad msgget msgrID (ratownik)");
        exit(EXIT_FAILURE);
    }

    local = czas();
    printf("%s[%02d:%02d:%02d  %d] Ratownik: Oczekiwanie na komunikaty...%s\n", BLUE, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);

    // while(1){
    //     
    //     local = czas();
    //    // printf("[%02d:%02d:%02d  %d] Ratownik przydziela klientowi basen.\n", local->tm_hour, local->tm_min, local->tm_sec, msg.pid);

    // }
    return 0;
}
