#include "utility.c"
// Struktura komunikatu

int main() {
    srand(time(NULL));
    time_t now;
    struct tm *local;
    struct msgbuf message;
    int shmID, msgID;
    key_t msg_key, shm_key;
    struct msgbuf msg;

    SharedMemory *shared_mem;

    // Uzyskaj dostęp do kolejki komunikatów
    if ((msg_key = ftok(".", 'A')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
    }
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    if (msgID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((shm_key = ftok(".", 'B')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
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

    time(&now);
    local = localtime(&now);

    printf("[%02d:%02d:%02d] Wygenerowano klienta: %d\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());
    
    msg.mtype = 1; // Typ komunikatu
    msg.pid = getpid();
    
    if (msgsnd(msgID, &msg, sizeof(int), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    shared_mem->count = 0;
    Client klient;

    klient.pid = getpid();
    klient.age = rand() % 1 + 100;
    klient.vip = rand() % 2;

    shared_mem->clients[shared_mem->count] = klient;
    shared_mem->count++;

    return 0;
}
