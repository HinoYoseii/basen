#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>

#define SHM_SIZE 12
#define MAX_PROCESSES 10
#define MAX_CLIENTS 100
#define KASJER 1
#define KASJER_VIP 2

struct msgbuf {
    long mtype; // Typ komunikatu
    int pid;  // PID procesu klienta
};

typedef struct {
    int pid;
    int age;
    int vip; // 0/1)
} Client;

typedef struct {
    int pid;
    int wiek;
    int wiek_opiekuna; 
    time_t czas_wyjscia;        // Liczba aktywnych klientów
     // Lista klientów
} SharedMemory;

struct klient{
    int pid;
    int wiek;
    int wiek_opiekuna;
    time_t czas_wyjscia;
};

struct tm* czas() {
    static time_t now;
    static struct tm local_time;

    time(&now);                     // Pobierz bieżący czas
    local_time = *localtime(&now);  // Konwertuj na czas lokalny

    return &local_time;             // Zwróć wskaźnik do struktury czasu
}