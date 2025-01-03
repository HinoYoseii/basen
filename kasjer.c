#include "utility.c"

int main() {
    int msgid;
    struct msgbuf msg;
    int shmID, msgID;
    key_t msg_key, shm_key;
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

    // Uzyskaj dostęp do pamięci dzielonej
    if ((shm_key = ftok(".", 'S')) == -1) {
        printf("Blad ftok A(main)\n");
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

    printf("Kasjer [%d]: Oczekiwanie na komunikaty...\n\n", getpid());
    sleep(10);
    //printf("huh\n");

    while (1) {
        //printf("lol\n");
        if (msgrcv(msgID, &msg, sizeof(msg), -2, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        if (msg.mtype == 1){
            printf("[VIP] Kasjer przyjął klienta PID: %d\n", msg.pid);
        }
        if (msg.mtype == 2){
            printf("Kasjer przyjął klienta PID: %d\n", msg.pid);
        }
        

    }
    return 0;
}
