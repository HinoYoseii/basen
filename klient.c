#include "utility.c"

int main() {
    srand(time(NULL));
    time_t now;
    struct tm *local;
    struct tm *wyjscie;

    int msgID, msgrID;
    key_t msg_key, msg2_key;
    struct msgbuf msg;
    struct klient_dane klient;

    if ((msg_key = ftok(".", 'M')) == -1) {
        printf("Blad ftok A (klient)");
        exit(EXIT_FAILURE);
    }
    if ((msgID = msgget(msg_key, IPC_CREAT | 0666)) == -1) {
        perror("Blad msgget msgID (klient)");
        exit(EXIT_FAILURE);
    }

    if ((msg2_key = ftok(".", 'R')) == -1) {
        printf("Blad ftok R (klient)");
        exit(EXIT_FAILURE);
    }
    
    if ((msgrID = msgget(msg2_key, IPC_CREAT | 0666)) == -1) {
        perror("Blad msgget msgrID (klient)");
        exit(EXIT_FAILURE);
    }

    int vip = (rand() % 5 + 1 == 5) ? 1 : 0;

    local = czas();
    if(vip)
        printf("%s[%02d:%02d:%02d  %d]%s Pojawia się klient VIP.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
    else
        printf("%s[%02d:%02d:%02d  %d]%s Pojawia się klient.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
        
    klient.pid = getpid();
    klient.wiek = (rand() % 70) + 1;
    klient.wiek_opiekuna = (klient.wiek < 10) ? ((rand() % 53) + 18) : 0;

    msg.pid = getpid();
    msg.mtype = (vip ? 1 : 2);
    msg.wiek = klient.wiek;
    msg.czas_wyjscia = 0;

    if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    
    if (msgrcv(msgID, &msg, sizeof(msg), getpid(), 0) == -1) {
            perror("msgrcv klient");
            exit(EXIT_FAILURE);
    }

    local = czas();
    printf("[%02d:%02d:%02d  %d] Klient wchodzi na basen.\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());

    if(klient.wiek <= 3){
        local = czas();
        printf("[%02d:%02d:%02d  %d] Dziecko zakłada pampers do plywania. Wiek: %d\n", local->tm_hour, local->tm_min, local->tm_sec, getpid(), klient.wiek);
    }

    if((rand() % 6 + 1 == 6) ? 1 : 0){
        local = czas();
        printf("[%02d:%02d:%02d  %d] Klient zakłada czepek.\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());
    }

    klient.czas_wyjscia = msg.czas_wyjscia;
    wyjscie = localtime(&klient.czas_wyjscia);
    printf("[%d] Czas wyjścia klienta: %02d:%02d:%02d\n", getpid(), wyjscie->tm_hour, wyjscie->tm_min, wyjscie->tm_sec);

    return 0;
}
