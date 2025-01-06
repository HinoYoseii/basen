#include "utility.c"
// Struktura komunikatu

int main() {
    srand(time(NULL));
    time_t now;
    struct tm *local;
    struct tm *wyjscie;
    struct msgbuf message;
    int shmID, msgID, msgrID, shmtID;
    key_t msg_key, msg2_key, shm_key, shmt_key;
    struct msgbuf msg;
    struct klient gen_klient;
    char godzina[9];

    SharedMemory *shared_mem;

    // Uzyskaj dostęp do kolejki komunikatów
    if ((msg_key = ftok(".", 'M')) == -1) {
        printf("Blad ftok A(klient)\n");
        exit(EXIT_FAILURE);
    }
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    if (msgID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((msg2_key = ftok(".", 'R')) == -1) {
        printf("Blad ftok R(klient)\n");
        exit(EXIT_FAILURE);
    }
    msgrID = msgget(msg2_key, IPC_CREAT | 0666);
    if (msgrID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((shm_key = ftok(".", 'S')) == -1) {
        printf("Blad ftok A(klient)\n");
        exit(EXIT_FAILURE);
    }
    shmID = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget klient");
        exit(EXIT_FAILURE);
    }

    shared_mem = (SharedMemory *)shmat(shmID, NULL, 0);
    if (shared_mem == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    if ((shmt_key = ftok(".", 'T')) == -1) {
        printf("Blad ftok T(main)\n");
        exit(1);
    }
    shmtID = shmget(shmt_key, sizeof(int), IPC_CREAT | 0666);
    if (shmtID == -1) {
        perror("shmget zarzadca");
        exit(EXIT_FAILURE);
    }

    char* shm_czas_adres = (char*)shmat(shmtID, NULL, 0);
    if (shm_czas_adres == (char*)(-1))
    {
        perror("shmat - problem z dolaczeniem pamieci do obslugi czasu");
        exit(EXIT_FAILURE);
    }


    int vip = (rand() % 20 + 1 == 20) ? 1 : 0;

    godz_sym(*((int *)shm_czas_adres), godzina);
    local = czas();
    if(vip)
        printf("[%s  %d] Pojawia się klient VIP.\n", godzina, getpid());
    else
        printf("[%s  %d] Pojawia się klient.\n", godzina, getpid());
    
     // Typ komunikatu
    msg.pid = getpid();
    msg.mtype = (vip ? 1 : 2);
    gen_klient.pid = getpid();
    gen_klient.wiek = (rand() % 70) + 1;
    gen_klient.wiek_opiekuna = (gen_klient.wiek < 10) ? ((rand() % 53) + 18) : 0;
    gen_klient.czas_wyjscia = time(NULL);

    if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    
    memcpy(shared_mem, &gen_klient, sizeof(struct klient));

    if (msgrcv(msgID, &msg, sizeof(msg), 3, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
    }

    memcpy(&gen_klient, shared_mem, sizeof(struct klient));

    local = czas();
    printf("[%02d:%02d:%02d  %d] Klient wchodzi na basen.\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());

    if(gen_klient.wiek <= 3){
        local = czas();
        printf("[%02d:%02d:%02d  %d] Dziecko zakłada pampers do plywania. Wiek: %d\n", local->tm_hour, local->tm_min, local->tm_sec, getpid(), gen_klient.wiek);
    }

    if((rand() % 6 + 1 == 6) ? 1 : 0){
        local = czas();
        printf("[%02d:%02d:%02d  %d] Klient zakłada czepek.\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());
    }

    wyjscie = localtime(&gen_klient.czas_wyjscia);
    printf("[%d] Czas wyjścia klienta: %02d:%02d:%02d\n", getpid(), wyjscie->tm_hour, wyjscie->tm_min, wyjscie->tm_sec);
    msg.mtype = 1;
    if (msgsnd(msgrID, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    return 0;
}
