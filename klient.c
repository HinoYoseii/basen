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
    if ((msg_key = ftok(".", 'M')) == -1) {
        printf("Blad ftok A(main)\n");
        exit(1);
    }
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    if (msgID == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((shm_key = ftok(".", 'S')) == -1) {
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
    int vip = rand()%2;
    printf("[%02d:%02d:%02d] Wygenerowano klienta: %d VIP: %d \n", local->tm_hour, local->tm_min, local->tm_sec, getpid(), vip);
    
    
     // Typ komunikatu
    msg.pid = getpid();
    msg.mtype = 2;

    if(vip){
        msg.mtype = 1;
    }

    if (msgsnd(msgID, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}
