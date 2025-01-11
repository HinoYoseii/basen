#include "utility.c"

int main() {
    srand(time(NULL));
    int msgid;
    struct msgbuf msg;
    int shmID, msgrID, shmtID;
    key_t msg2_key, shm_key, shmt_key;
    SharedMemory *shared_mem;
    time_t now;
    struct tm *local;
    char godzina[9];

    // Uzyskaj dostęp do kolejki komunikatów
    if ((msg2_key = ftok(".", 'R')) == -1) {
        printf("Blad ftok R(ratownik)\n");
        exit(1);
    }
    msgrID = msgget(msg2_key, IPC_CREAT | 0666);
    if (msgrID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Uzyskaj dostęp do pamięci dzielonej
    if ((shm_key = ftok(".", 'S')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
    }
    shmID = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget ratownik");
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
    if (shm_czas_adres == (char*)(-1)) {
        perror("shmat - problem z dolaczeniem pamieci do obslugi czasu");
        exit(EXIT_FAILURE);
    }

    local = czas();
    printf("[%02d:%02d:%02d  %d] Ratownik: Oczekiwanie na komunikaty...\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());

    while(1){
        if (msgrcv(msgrID, &msg, sizeof(msg), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        godz_sym(*((int *)shm_czas_adres), godzina);
        printf("[%s  %d] Ratownik przydziela klientowi basen.\n", godzina, msg.pid);
        
    }
    return 0;
}
