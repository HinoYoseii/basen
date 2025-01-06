#include "utility.c"

int main() {
    srand(time(NULL));
    int msgid;
    struct msgbuf msg;
    int shmID, msgID, shmtID;
    key_t msg_key, shm_key, shmt_key;
    SharedMemory *shared_mem;
    time_t now;
    struct klient gen_klient;
    char godzina[9];

    // Uzyskaj dostęp do kolejki komunikatów
    if ((msg_key = ftok(".", 'M')) == -1) {
        printf("Blad ftok M(kasjer)\n");
        exit(1);
    }
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    if (msgID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Uzyskaj dostęp do pamięci dzielonej
    if ((shm_key = ftok(".", 'S')) == -1) {
        printf("Blad ftok S(kasjer)\n");
        exit(1);
    }
    shmID = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget kasjer");
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

    // Zmieniamy sposób pobierania czasu na wykorzystanie godz_sym
    godz_sym(*((int *)shm_czas_adres), godzina);
    printf("[%s  %d] Kasjer: Oczekiwanie na komunikaty...\n\n", godzina, getpid());
    sleep(3);

    while (1) { 
        // sleep(rand() % 3 + 1);

        if (msgrcv(msgID, &msg, sizeof(msg), -2, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // Zmieniamy sposób pobierania czasu na wykorzystanie godz_sym
        godz_sym(*((int *)shm_czas_adres), godzina);

        if (msg.mtype == 1) {
            printf("[%s  %d] Kasjer obsługuje klienta VIP.\n", godzina, msg.pid);
        }
        if (msg.mtype == 2) {
            printf("[%s  %d] Kasjer obsługuje klienta.\n", godzina, msg.pid);
        }

        memcpy(&gen_klient, shared_mem, sizeof(struct klient));

        // Zmieniamy sposób pobierania czasu na wykorzystanie godz_sym
        godz_sym(*((int *)shm_czas_adres), godzina);
        if (gen_klient.wiek < 10) {
            printf("[%s  %d] Opiekun płaci za bilet. Dziecko nie płaci za bilet. Wiek: %d\n", godzina, msg.pid, gen_klient.wiek);
        } else {
            printf("[%s  %d] Klient płaci za bilet.\n", godzina, msg.pid);
        }

        // Zmieniamy sposób pobierania czasu na wykorzystanie godz_sym
        godz_sym(*((int *)shm_czas_adres), godzina);
        gen_klient.czas_wyjscia = (*((int *)shm_czas_adres)) + 3600;
        memcpy(shared_mem, &gen_klient, sizeof(struct klient));

        msg.mtype = 3;
        if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
