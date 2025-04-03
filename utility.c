#include "utility.h"

struct tm* czas() {
// Funkcja zwracająca obecną godzinę
// 
// Opis:
// Funkcja pobiera bieżący czas systemowy i zwraca wskaźnik do struktury struct tm, 
// zawierającej szczegółowe informacje o lokalnym czasie.
// 
// Parametry:
// - Brak parametrów wejściowych.
// 
// Wartość zwracana:
// - Wskaźnik do struktury struct tm, która zawiera dane o lokalnym czasie.
    static time_t now;
    static struct tm local_time;
    
    time(&now);
    local_time = *localtime(&now);

    return &local_time;
}

void sprawdz_blad(int wynik, const char *opis) {
// Funkcja sprawdzająca poprawność wywołania funkcji systemowych
//
// Opis:
// Funkcja służy do sprawdzania, czy wywołanie funkcji systemowej zakończyło się sukcesem.
// Jeśli wynik operacji wynosi -1, wyświetla komunikat o błędzie (używając perror), 
// drukuje numer błędu (errno), a następnie kończy działanie programu z kodem EXIT_FAILURE.
//
// Parametry:
// - wynik: Wynik wywołania funkcji systemowej (np. wynik fork, shmget itp.).
// - opis: Opis tekstowy używany w komunikacie o błędzie.
//
// Wartość zwracana:
// - Funkcja nie zwraca wartości. W przypadku błędu kończy program.
    if (wynik == -1) {
        perror(opis);
        printf("Errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void sprawdz_blad_watek(int wynik, const char *opis) {
// Funkcja sprawdzająca poprawność wywołania funkcji systemowych dotyczących wątków
//
// Opis:
// Funkcja sprawdza, czy operacje związane z wątkami (np. pthread_create, pthread_join) 
// zakończyły się sukcesem. Jeśli wynik operacji jest różny od zera, wyświetla komunikat 
// o błędzie (używając strerror do opisu błędu) i kończy działanie programu.
//
// Parametry:
// - wynik: Wynik wywołania funkcji związanej z wątkami.
// - opis: Opis tekstowy używany w komunikacie o błędzie.
//
// Wartość zwracana:
// - Funkcja nie zwraca wartości. W przypadku błędu kończy program.
    if (wynik != 0) {
        fprintf(stderr, "%s failed: %s\n", opis, strerror(wynik));
        exit(EXIT_FAILURE);
    }
}

void wyswietl_klientow(int *klienci, int rozmiar) {
// Funkcja do wyświetlania zawartości tablicy jednowymiarowej reprezentującej klientów
//
// Opis:
// Funkcja iteruje po przekazanej tablicy klienci, która reprezentuje miejsca na basenie.
// Pierwszy element tablicy zawiera liczbę klientów, a pozostałe miejsca zawierają 
// informacje o procesach klientów (ich PID-y) lub 0, jeśli miejsce jest puste. 
// Wyświetla szczegóły w czytelnej formie.
//
// Parametry:
// - klienci: Wskaźnik na tablicę jednowymiarową reprezentującą klientów.
//      Indeks 0: Liczba klientów.
//      Indeksy od 1 do rozmiar-1: PID-y klientów lub 0, jeśli miejsce jest puste.
// - rozmiar: Rozmiar tablicy
//
// Wartość zwracana:
// - Funkcja nie zwraca wartości. Wyświetla informacje w konsoli.

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

