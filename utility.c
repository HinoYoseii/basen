#include "utility.h"

// Funkcja zwracająca obecną godzine
struct tm* czas() {
    static time_t now;
    static struct tm local_time;
    
    time(&now);
    local_time = *localtime(&now);

    return &local_time;
}

// Funkcja sprawdzająca poprawność wywołania funkcji systemowych
void sprawdz_blad(int wynik, const char *opis) {
    if (wynik == -1) {
        perror(opis);
        printf("Errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

// Funkcja sprawdzająca poprawność wywołania funkcji systemowych dotyczących wątków
void sprawdz_blad_watek(int wynik, const char *opis) {
    if (wynik != 0) {
        fprintf(stderr, "%s failed: %s\n", opis, strerror(wynik));
        exit(EXIT_FAILURE);
    }
}

// Funkcja do wyśwwietlania tablicy jednowymiarowej
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

