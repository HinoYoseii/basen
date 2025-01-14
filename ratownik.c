#include "utility.c"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Blad: Brak identyfikatora basenu\n");
        exit(EXIT_FAILURE);
    }

    int pool_id = atoi(argv[1]); // Pobierz identyfikator basenu
    srand(time(NULL) + pool_id); // Opcjonalne: różnicowanie na podstawie pool_id

    struct msgbuf_r msgr;
    int msgrID;
    key_t msg2_key;
    struct tm *local;

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
        int klienci[X1 + 1];
        for(int i = 0; i <= X1; i++){
            klienci[i] = 0;
        }

        while (1) {
            if (msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), -2, 0) == -1) {
                perror("Blad msgrcv msgrID (ratownik)");
                exit(EXIT_FAILURE);
            }
            if(msgr.mtype == 1){
                for(int i = 1; i <= X1; i++){
                    if(msgr.mtype == klienci[i]){
                        klienci[0]--;
                        klienci[i] = 0;
                        break;
                    }
                }
            }
            else{
                 msgr.mtype = msgr.pid;

                if(msgr.wiek >= 18){
                    if(klienci[0] == 3){
                        msgr.kom = 'n';
                        if (msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0) == -1) {
                            perror("Blad msgsnd msgrID (klient)");
                            exit(EXIT_FAILURE);
                        }

                        printf("%s[%d]%s Nie ma miejsca na basenie olimpijskim.\n", YELLOW, msgr.pid, RESET);
                    }
                    else{
                        for(int i = 1; i <= X1; i++){
                            if(klienci[i] == 0){
                                klienci[0]++;
                                klienci[i] = msgr.pid;
                                
                                msgr.kom = 't';
                                if (msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0) == -1) {
                                    perror("Blad msgsnd msgrID (klient)");
                                    exit(EXIT_FAILURE);
                                }

                                local = czas();
                                printf("%s[%02d:%02d:%02d  %d]%s Klient wchodzi do basenu olimpijskiego.\n", MAGENTA, local->tm_hour, local->tm_min, local->tm_sec, msgr.pid, RESET);
                                break;
                            }
                        }
                    }
                    
                }
                else{
                    msgr.kom = 'w';
                    if (msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0) == -1) {
                        perror("Blad msgsnd msgrID (klient)");
                        exit(EXIT_FAILURE);
                    }

                    printf("%s[%d]%s Kapiel w basenie olimpijskim od 18 roku życia. Wiek: %d\n", YELLOW, msgr.pid, RESET, msgr.wiek);
                }
            }
            
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
