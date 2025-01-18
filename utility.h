#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/shm.h>

#define MAX_CLIENTS 20

const char *RESET = "\033[0m";
const char *RED = "\033[31m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *BLUE = "\033[34m";
const char *MAGENTA = "\033[35m";
const char *CYAN = "\033[36m";

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
    char kom;
};

struct klient_dane{
    int pid;
    int wiek;
    int wiek_opiekuna;
    time_t czas_wyjscia;
    int basen;
};

struct shared_mem{
    int dlugosc_otwarcia;
};

struct tm* czas();
void sprawdz_blad(int wynik, const char *opis);