#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define MAX_CLIENTS 5000

// Definicje kolorów do wyświetlania w konsoli
const char *RESET = "\033[0m";
const char *RED = "\033[31m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *BLUE = "\033[34m";
const char *MAGENTA = "\033[35m";
const char *CYAN = "\033[36m";

struct msgbuf {
    long mtype; // Typ komunikatu
    int pid;    // PID procesu klienta
    int wiek;   // Wiek klienta
    time_t czas_wyjscia;    // Godzina wyjścia klienta
};

struct msgbuf_r {
    long mtype; // Typ komunikatu
    int pid;    // PID procesu klienta
    int wiek;   // Wiek klienta
    int wiek_opiekuna;  // Wiek opiekuna klienta
    char kom;   //  Infromacja zwrotna klienta
};

struct klient_dane{
    int pid;    // PID procesu klienta
    int wiek;   // Wiek klienta
    int wiek_opiekuna;  // Wiek opiekuna klienta
    time_t czas_wyjscia;    // Godzina wyjścia klienta
    int basen;  // Numer basenu na którym przebywa klient
    
};

struct shared_mem{
    int dlugosc_otwarcia;   // Długośc otwarcia basenu, podana przez użytkownika
    time_t zamkniecie;  // Godzina zamknięcia kompleksu
    time_t otwarcie;    // Godzina otwarcia kompleksu
};

struct tm* czas();  // Funkcja zwracająca godzine z momentu wywołania
void sprawdz_blad(int wynik, const char *opis); // Funkcja sprawdzająca wywołania funkcji systemowych
void sprawdz_blad_watek(int wynik, const char *opis); // Funkcja sprawdzająca poprawność wywołania funkcji systemowych dotyczących wątków
void wyswietl_klientow(int *klienci, int rozmiar);  // Funkcja do wyśwwietlania tablicy jednowymiarowej
