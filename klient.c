#include "utility.c"
// Struktura komunikatu

int main() {
    srand(time(NULL));
    time_t now;
    struct tm *local;
    struct msgbuf message;
    int shmID, msgID, msgrID;
    key_t msg_key, msg2_key, shm_key;
    struct msgbuf msg;

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
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_mem = (SharedMemory *)shmat(shmID, NULL, 0);
    if (shared_mem == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int vip = (rand() % 20 + 1 == 20) ? 1 : 0;

    time(&now);
    local = localtime(&now);

    if(vip)
        printf("[%02d:%02d:%02d] [%d] Pojawia się klient VIP.\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());
    else
        printf("[%02d:%02d:%02d] [%d] Pojawia się klient.\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());
    
     // Typ komunikatu
    msg.pid = getpid();
    msg.mtype = (vip ? 1 : 2);

    if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgID, &msg, sizeof(msg), 3, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
    }

    time(&now);
    local = localtime(&now);

    printf("[%02d:%02d:%02d] [%d] Klient wchodzi na basen.\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());

    msg.mtype = 1;
    if (msgsnd(msgrID, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}