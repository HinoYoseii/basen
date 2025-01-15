#include "utility.c"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

pthread_mutex_t mutex_olimpijski, mutex_rekreacyjny, mutex_brodzik;
pthread_t t_wpuszczanie, t_wychodzenie;
struct msgbuf_r msgr;
int msgrID, pool_id;
key_t msg2_key;

// Funkcje wątków
void *olimpijski(void *arg);
void *rekreacyjny(void *arg);
void *brodzik(void *arg);
void *wychodzenie_olimpijski(void *arg);
void *wychodzenie_rekreacyjny(void *arg);
void *wychodzenie_brodzik(void *arg);
void wyswietl_klientow(int *klienci, int rozmiar);
double suma_wieku(int klienci[2][X2+1], int liczba_klientow);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Blad: Brak identyfikatora basenu\n");
        exit(EXIT_FAILURE);
    }

    pool_id = atoi(argv[1]);

    if ((msg2_key = ftok(".", 'R')) == -1) {
        perror("Blad ftok R (ratownik)");
        exit(EXIT_FAILURE);
    }
    if ((msgrID = msgget(msg2_key, IPC_CREAT | 0666)) == -1) {
        perror("Blad msgget msgrID (ratownik)");
        exit(EXIT_FAILURE);
    }

    printf("Ratownik [%d]: Obsługuje basen %d\n", getpid(), pool_id);

    if (pool_id == 1) {
        // Inicjalizacja danych
        int klienci[X1 + 1];
        for (int i = 0; i <= X1; i++) {
            klienci[i] = 0;
        }

        // Inicjalizacja mutexa
        if (pthread_mutex_init(&mutex_olimpijski, NULL) != 0) {
            perror("pthread_mutex_init - błąd inicjalizacji mutexa");
            exit(EXIT_FAILURE);
        }

        // Tworzenie wątków
        if (pthread_create(&t_wpuszczanie, NULL, &olimpijski, klienci) != 0) {
            perror("pthread_create - wątek wpuszczania");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&t_wychodzenie, NULL, &wychodzenie_olimpijski, klienci) != 0) {
            perror("pthread_create - wątek wychodzenia");
            exit(EXIT_FAILURE);
        }

        // Oczekiwanie na zakończenie wątków
        if (pthread_join(t_wpuszczanie, NULL) != 0) {
            perror("pthread_join - wątek wpuszczania");
        }
        if (pthread_join(t_wychodzenie, NULL) != 0) {
            perror("pthread_join - wątek wychodzenia");
        }

        // Niszczenie mutexa
        if (pthread_mutex_destroy(&mutex_olimpijski) != 0) {
            perror("pthread_mutex_destroy - błąd niszczenia mutexa");
            exit(EXIT_FAILURE);
        }
    }
    else if(pool_id == 2){
        int klienci[2][X2 + 1];
        for (int i = 0; i <= X2; i++) {
            klienci[0][i] = 0;
            klienci[1][i] = 0;
        }

        // Inicjalizacja mutexa
        if (pthread_mutex_init(&mutex_rekreacyjny, NULL) != 0) {
            perror("pthread_mutex_init - błąd inicjalizacji mutexa");
            exit(EXIT_FAILURE);
        }

        // Tworzenie wątków
        if (pthread_create(&t_wpuszczanie, NULL, &rekreacyjny, klienci) != 0) {
            perror("pthread_create - wątek wpuszczania");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&t_wychodzenie, NULL, &wychodzenie_rekreacyjny, klienci) != 0) {
            perror("pthread_create - wątek wychodzenia");
            exit(EXIT_FAILURE);
        }

        // Oczekiwanie na zakończenie wątków
        if (pthread_join(t_wpuszczanie, NULL) != 0) {
            perror("pthread_join - wątek wpuszczania");
        }
        if (pthread_join(t_wychodzenie, NULL) != 0) {
            perror("pthread_join - wątek wychodzenia");
        }

        // Niszczenie mutexa
        if (pthread_mutex_destroy(&mutex_rekreacyjny) != 0) {
            perror("pthread_mutex_destroy - błąd niszczenia mutexa");
            exit(EXIT_FAILURE);
        }
    }
    else if(pool_id == 3){

        int klienci[(X3 / 2) + 1];
        for (int i = 0; i <= (X3/2); i++) {
            klienci[i] = 0;
        }
        // for (int i = 0; i <= (X3/2); i++) {
        //     printf("%d - %d\n",i ,klienci[i]);
        // }

        // Inicjalizacja mutexa
        if (pthread_mutex_init(&mutex_brodzik, NULL) != 0) {
            perror("pthread_mutex_init - błąd inicjalizacji mutexa");
            exit(EXIT_FAILURE);
        }

        // Tworzenie wątków
        if (pthread_create(&t_wpuszczanie, NULL, &brodzik, klienci) != 0) {
            perror("pthread_create - wątek wpuszczania");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&t_wychodzenie, NULL, &wychodzenie_brodzik, klienci) != 0) {
            perror("pthread_create - wątek wychodzenia");
            exit(EXIT_FAILURE);
        }

        // Oczekiwanie na zakończenie wątków
        if (pthread_join(t_wpuszczanie, NULL) != 0) {
            perror("pthread_join - wątek wpuszczania");
        }
        if (pthread_join(t_wychodzenie, NULL) != 0) {
            perror("pthread_join - wątek wychodzenia");
        }

        // Niszczenie mutexa
        if (pthread_mutex_destroy(&mutex_brodzik) != 0) {
            perror("pthread_mutex_destroy - błąd niszczenia mutexa");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

void *olimpijski(void *arg) {
    int *klienci = (int *)arg;

    while (1) {
        if (msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 1, 0) == -1) {
            perror("Blad msgrcv msgrID (ratownik)");
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&mutex_olimpijski);
        msgr.mtype = msgr.pid;

        if (msgr.wiek < 18) {
            msgr.kom = 'w';
        } else if (klienci[0] >= X1) {
            msgr.kom = 'n';
        } else {
            for (int i = 1; i <= X1; i++) {
                if (klienci[i] == 0) {
                    klienci[i] = msgr.pid;
                    klienci[0]++;
                    msgr.kom = 't';
                    break;
                }
            }
            wyswietl_klientow(klienci, X1 + 1);
        }
        

        pthread_mutex_unlock(&mutex_olimpijski);

        if (msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0) == -1) {
            perror("Blad msgsnd msgrID (klient)");
        }
    }

    pthread_exit(NULL);
}

void *rekreacyjny(void *arg){
    int (*klienci)[X2 + 1] = (int (*)[X2 + 1])arg;

    while(1){
        if (msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 2, 0) == -1) {
            perror("Blad msgrcv msgrID (ratownik)");
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&mutex_rekreacyjny);
        msgr.mtype = msgr.pid;
        //printf("%d\n", msgr.wiek);
        int wiek_klienta = msgr.wiek;
        int liczba_osob = 0;
        double srednia = 0;

        for(int i = 1; i <= X2; i++){
            //printf("%d. %d\n", i ,klienci[1][i]);
            srednia += klienci[1][i];
        }

        if(msgr.wiek_opiekuna > 0){
            wiek_klienta = msgr.wiek + msgr.wiek_opiekuna;
            liczba_osob = 1;
        }

        //printf("liczba klientow: %d\n", klienci[0][0]);
        int liczba_klientow = klienci[0][0] + liczba_osob + 1;
        srednia = (srednia + wiek_klienta) / liczba_klientow;

        //printf("suma_wieku: %f\n", srednia);
        //srednia = (srednia + wiek_klienta) / (klienci[0][0] + liczba_osob);

        if (klienci[0][0] >= (X2 - liczba_osob)) {
            msgr.kom = 'n';
        } else if (srednia > 40) {
           //printf("srednia: %f\n", srednia);
            msgr.kom = 's';
        } else {
            for (int i = 1; i <= X2; i++) {
                if (klienci[0][i] == 0) {
                    klienci[0][i] = msgr.pid;
                    klienci[1][i] = wiek_klienta;
                    if(msgr.wiek_opiekuna > 0){
                        klienci[0][0] += 2;
                    }
                    else{
                        klienci[0][0]++;
                    }
                    msgr.kom = 't';

                    printf("\nStan tablicy klienci:\n");
                    printf("Liczba klientów na basenie: %d\n", klienci[0][0]);
                    for (int i = 1; i <= X2; i++) {
                        if (klienci[0][i] == 0) {
                            printf("Miejsce %d: PUSTE\n", i);
                        } else {
                            printf("Miejsce %d: PID klienta %d Wiek: %d\n", i, klienci[0][i], klienci[1][i]);
                        }
                    }
                    printf("\n");

                    break;
                }
            }
        }

        pthread_mutex_unlock(&mutex_rekreacyjny);

        if (msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0) == -1) {
            perror("Blad msgsnd msgrID (klient)");
        }
    }
    pthread_exit(NULL);

}

void *brodzik(void *arg) {
    int *klienci = (int *)arg;

    while (1) {
        if (msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 3, 0) == -1) {
            perror("Blad msgrcv msgrID (ratownik)");
            pthread_exit(NULL);
        }
        printf("huh\n");
        msgr.mtype = msgr.pid;

        pthread_mutex_lock(&mutex_brodzik);

        if (msgr.wiek > 5) {
            msgr.kom = 'w';
        } else if (klienci[0] >= X3) {
            msgr.kom = 'n';
        } else {
            for (int i = 1; i <= (X3/2 + 1); i++) {
                if (klienci[i] == 0) {
                    klienci[i] = msgr.pid;
                    klienci[0] += 2;
                    msgr.kom = 't';
                    break;
                }
            }
            wyswietl_klientow(klienci, (X3/2 + 1));
        }
        
        pthread_mutex_unlock(&mutex_brodzik);

        if (msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0) == -1) {
            perror("Blad msgsnd msgrID (klient)");
        }
    }

    pthread_exit(NULL);
}

void *wychodzenie_olimpijski(void *arg) {
    while (1) {
        int *klienci = (int *)arg;

        if (msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 4, 0) == -1) {
            perror("Blad msgrcv msgrID (ratownik)");
            pthread_exit(NULL);
        }

        // printf("Przed zablokowaniem");
        // wyswietl_klientow(klienci, X1 + 1);

        pthread_mutex_lock(&mutex_olimpijski);

        //printf("KLIENT %d\n", msgr.pid);
        for (int i = 1; i <= X1; i++) {
            if (klienci[i] == msgr.pid) {
                klienci[i] = 0;
                klienci[0]--;
                //printf("KLIENT %d ZNALEZIONY\n", msgr.pid);
                break;
            }
        }
        // printf("Po zablokowaniu");
        // wyswietl_klientow(klienci, X1 + 1);

        pthread_mutex_unlock(&mutex_olimpijski);
    }

    pthread_exit(NULL);
}

void *wychodzenie_rekreacyjny(void *arg){
    while(1){
        continue;
    }
    pthread_exit(NULL);

}


void *wychodzenie_brodzik(void *arg){
    while (1) {
        int *klienci = (int *)arg;

        if (msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 6, 0) == -1) {
            perror("Blad msgrcv msgrID (ratownik)");
            pthread_exit(NULL);
        }

        // printf("Przed zablokowaniem");
        // wyswietl_klientow(klienci, (X3/2 + 1));

        pthread_mutex_lock(&mutex_brodzik);

        //printf("KLIENT %d\n", msgr.pid);
        for (int i = 1; i <= (X3/2); i++) {
            if (klienci[i] == msgr.pid) {
                klienci[i] = 0;
                klienci[0] -= 2;
                //printf("KLIENT %d ZNALEZIONY\n", msgr.pid);
                break;
            }
        }
        // printf("Po zablokowaniu");
        // wyswietl_klientow(klienci, (X3/2 + 1));

        pthread_mutex_unlock(&mutex_brodzik);
    }

    pthread_exit(NULL);
}

void wyswietl_klientow(int *klienci, int rozmiar) {
    printf("\nStan tablicy klienci:\n");
    printf("Liczba klientów na basenie: %d\n", klienci[0]);
    for (int i = 1; i < rozmiar; i++) {
        if (klienci[i] == 0) {
            printf("Miejsce %d: PUSTE\n", i);
        } else {
            printf("Miejsce %d: PID klienta %d\n", i, klienci[i]);
        }
    }
    printf("\n");
}

double suma_wieku(int klienci[2][X2+1], int liczba_klientow) {
    double suma = 0;

    for (int i = 1; i <= liczba_klientow; i++) {
        suma += klienci[1][i];
    }

    return suma;
}

