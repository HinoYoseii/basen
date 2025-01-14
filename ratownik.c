#include "utility.c"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

pthread_mutex_t mutex;
pthread_t t_wpuszczanie, t_wychodzenie;
struct msgbuf_r msgr;
int msgrID, pool_id;
key_t msg2_key;

// Funkcje wątków
void *olimpijski(void *arg);
void *wychodzenie_klientow(void *arg);
void wyswietl_klientow(int *klienci, int rozmiar);

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
        if (pthread_mutex_init(&mutex, NULL) != 0) {
            perror("pthread_mutex_init - błąd inicjalizacji mutexa");
            exit(EXIT_FAILURE);
        }

        // Tworzenie wątków
        if (pthread_create(&t_wpuszczanie, NULL, &olimpijski, klienci) != 0) {
            perror("pthread_create - wątek wpuszczania");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&t_wychodzenie, NULL, &wychodzenie_klientow, klienci) != 0) {
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
        if (pthread_mutex_destroy(&mutex) != 0) {
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

        pthread_mutex_lock(&mutex);
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
            //wyswietl_klientow(klienci, X1 + 1);
        }
        

        pthread_mutex_unlock(&mutex);

        if (msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0) == -1) {
            perror("Blad msgsnd msgrID (klient)");
        }
    }

    pthread_exit(NULL);
}

void *wychodzenie_klientow(void *arg) {
    int *klienci = (int *)arg;

    while (1) {
        if (msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 2, 0) == -1) {
            perror("Blad msgrcv msgrID (ratownik)");
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&mutex);

        for (int i = 1; i <= X1; i++) {
            if (klienci[i] == msgr.pid) {
                klienci[i] = 0;
                klienci[0]--;
                break;
            }
        }
        //wyswietl_klientow(klienci, X1 + 1);

        pthread_mutex_unlock(&mutex);
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

