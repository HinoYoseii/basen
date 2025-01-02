#include "utility.c"

int main() {
    int msgid;
    struct msgbuf message;
    int shmID, msgID;
    key_t msg_key, shm_key;
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

    // Uzyskaj dostęp do pamięci dzielonej
    if ((shm_key = ftok(".", 'B')) == -1) {
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

    printf("Kasjer [%d]: Oczekiwanie na komunikaty...\n", getpid());
    sleep(5);

    struct msgbuf msg;
    
    while (1) {
        if (msgrcv(msgID, &message, sizeof(int), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // Print the PID received from the client
        printf("Vendor received PID: %d from client.\n", message.pid);
    }

    // Nigdy tu nie dojdzie, ponieważ pętla jest nieskończona
    return 0;
}
