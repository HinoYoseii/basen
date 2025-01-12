#pragma once

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
#include <pthread.h>
#include <stdbool.h>

#define SHM_SIZE 12
#define MAX_PROCESSES 10
#define MAX_CLIENTS 100
#define X1 3 // olimpijski
#define X2 4 // rekreacyjny
#define X3 2 // brodzik

const char *RESET = "\033[0m";
const char *RED = "\033[31m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *BLUE = "\033[34m";
const char *MAGENTA = "\033[35m";

struct msgbuf {
    long mtype; // Typ komunikatu
    int pid;  // PID procesu klienta
    int wiek;
    time_t czas_wyjscia;
};

struct msgbuf_r {
    long mtype; // Typ komunikatu
    int pid;  // PID procesu klienta
    int wiek;
    int wiek_opiekuna;
};

struct klient_dane{
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
