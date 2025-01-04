#include "utility.c"

int main() {
    srand(time(NULL));
    int msgid;
    struct msgbuf msg;
    int shmID, msgID;
    key_t msg_key, shm_key;
    SharedMemory *shared_mem;
    time_t now;
    struct tm *local;


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
    shmID = shmget(shm_key, SHM_SIZE * sizeof(int), IPC_CREAT | 0666);
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
    
    printf("[%02d:%02d:%02d] [%d] Kasjer: Oczekiwanie na komunikaty...\n\n", local->tm_hour, local->tm_min, local->tm_sec, getpid());
    sleep(3);

    while (1) { 
        if (msgrcv(msgID, &msg, sizeof(msg), -2, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        time(&now);
        local = localtime(&now);

        if (msg.mtype == 1){
            printf("[%02d:%02d:%02d] [%d] Kasjer obsługuje klienta VIP.\n", local->tm_hour, local->tm_min, local->tm_sec, msg.pid);
        }
        if (msg.mtype == 2){
            printf("[%02d:%02d:%02d] [%d] Kasjer obsługuje klienta\n", local->tm_hour, local->tm_min, local->tm_sec, msg.pid);
        }

        sleep(rand() % 3 + 1);
        
        msg.mtype = 3;
        if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
