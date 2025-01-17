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