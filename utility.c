#include "utility.h"

struct tm* czas() {
    static time_t now;
    static struct tm local_time;
    
    time(&now);
    local_time = *localtime(&now);

    return &local_time;
}

void sprawdz_blad(int wynik, const char *opis) {
    if (wynik == -1) {
        perror(opis);
        printf("Errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void wyswietl_klientow(int *klienci, int rozmiar) {
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

