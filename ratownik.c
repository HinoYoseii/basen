#include "utility.c"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Blad: Brak identyfikatora basenu\n");
        exit(EXIT_FAILURE);
    }

    int pool_id = atoi(argv[1]); // Pobierz identyfikator basenu
    srand(time(NULL) + pool_id); // Opcjonalne: różnicowanie na podstawie pool_id

    struct msgbuf msg;
    int msgrID;
    key_t msg2_key;

    if ((msg2_key = ftok(".", 'R')) == -1) {
        printf("Blad ftok R (ratownik)");
        exit(EXIT_FAILURE);
    }
    if ((msgrID = msgget(msg2_key, IPC_CREAT | 0666)) == -1) {
        perror("Blad msgget msgrID (ratownik)");
        exit(EXIT_FAILURE);
    }

    // Wyświetl informację o przypisanym basenie
    printf("Ratownik [%d]: Obsługuje basen %d\n", getpid(), pool_id);

    // #define X1 3 // olimpijski
    // #define X2 4 // rekreacyjny
    // #define X3 2 // brodzik 

    // Główna pętla działania
    //olimpijski
    if(pool_id == 1){
        
        while (1) {
            continue;
        }
    }
    //rekreacyjny
    else if(pool_id == 2){
        while (1) {
            continue;
        }
    }
    //brodzik
    else if(pool_id == 3){
        while (1) {
            continue;
        }
    }
    

    return 0;
}
