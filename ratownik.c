#include "utility.c"

pthread_mutex_t mutex_olimpijski, mutex_rekreacyjny, mutex_brodzik; // Mutexy do blokowania poszczególych basenów
pthread_t t_wpuszczanie, t_wychodzenie, t_sygnaly;                  // Identifykatory wątków w każdym z procesów
key_t msgr_key, shm_key;                                            // Klucze do kolejki komunikatów i pamięci współdzielonej
int msgrID, shmID;                                                  // ID kolejki komunikatów i pamięci współdzielonej
int pool_id, pool_size, dlugosc_otwarcia, czynny = 1;               // zmienne do obsługi basenów
time_t zamkniecie;
struct msgbuf_r msgr;                                               // Bufor do komunikatu
struct tm *local;                                                   // Wskaźnik do wyświetlania obecnego czasu
struct shared_mem *shared_data;                                     // Wskaźnik na strukture pamięci współdzielonej

void *olimpijski(void *arg);                                        // Funkcja obsługująca klientów wchodzących do basenu olimpijskiego
void *rekreacyjny(void *arg);                                       // Funkcja obsługująca klientów wchodzących do basenu rekreacyjnego
void *brodzik(void *arg);                                           // Funkcja obsługująca klientów wchodzących do brodzika
void *wychodzenie_olimpijski(void *arg);                            // Funkcja obsługująca klientów wychodzących z basenu olimpijskiego
void *wychodzenie_rekreacyjny(void *arg);                           // Funkcja obsługująca klientów wychodzących z basenu rekreacyjnego
void *wychodzenie_brodzik(void *arg);                               // Funkcja obsługująca klientów wychodzących z brodzika
void* sygnal(void *arg);                                            // Funkcja do obsługi wysyłania sygnałów

int main(int argc, char *argv[]) {
    srand(getpid());
    // Pobranie id basenu i jego rozmiaru
    if (argc < 3) {
        fprintf(stderr, "Blad: Brak identyfikatora i rozmiaru basenu\n");
        exit(EXIT_FAILURE);
    }

    pool_id = atoi(argv[1]);
    pool_size = atoi(argv[2]);
    //printf("Wartość X%d: %d\n", pool_id, pool_size);

    // Dołączenie do kolejki komunikatów klient <-> ratownik
    msgr_key = ftok(".", 'R');
    sprawdz_blad(msgr_key, "ftok R (klient)");
    msgrID = msgget(msgr_key, IPC_CREAT | 0666);
    sprawdz_blad(msgrID, "msgget msgrID (zarzadca)");

    // Dołączenie do fragmentu pamięci współdzielonej
    shm_key = ftok(".", 'S');
    sprawdz_blad(shm_key, "ftok S (zarzadca)");
    shmID = shmget(shm_key, sizeof(struct shared_mem), IPC_CREAT | 0666);
    sprawdz_blad(shmID, "shmget shmID (zarzadca)");
    // Pobranie długości otwarcia basenu z pamięci współdzielonej
    shared_data = shmat(shmID, NULL, 0);
    dlugosc_otwarcia = shared_data->dlugosc_otwarcia;
    zamkniecie = shared_data->zamkniecie;

    printf("%sRatownik [%d]%s Obsługuje basen %d o rozmiarze %d\n", MAGENTA, getpid(), RESET, pool_id, pool_size);

    if (pool_id == 1) {
        // Inicjalizacja tablicy do basenu, rozmiar basenu + 1 na przechowanie liczby klientów
        int klienci[pool_size + 1];
        for (int i = 0; i <= pool_size; i++) {
            klienci[i] = 0;
        }
        // Inicjalizacja mutexa
        sprawdz_blad_watek(pthread_mutex_init(&mutex_olimpijski, NULL), "pthread_mutex_init ratownik 1 - mutex olimpijski");

        // Tworzenie wątków
        sprawdz_blad_watek(pthread_create(&t_wpuszczanie, NULL, &olimpijski, klienci), "pthread_create ratownik 1 - wątek t_wpuszczanie");
        sprawdz_blad_watek(pthread_create(&t_wychodzenie, NULL, &wychodzenie_olimpijski, klienci), "pthread_create ratownik 1 - wątek t_wychodzenie_olimpijski");
        sprawdz_blad_watek(pthread_create(&t_sygnaly, NULL, &sygnal, klienci), "pthread_create ratownik 1 - wątek t_sygnaly");

        // Oczekiwanie na zakończenie wątków
        sprawdz_blad_watek(pthread_join(t_wpuszczanie, NULL), "pthread_join ratownik 1 - wątek t_wpuszczanie");
        sprawdz_blad_watek(pthread_join(t_wychodzenie, NULL), "pthread_join ratownik 1 - wątek t_wychodzenie");
        sprawdz_blad_watek(pthread_join(t_sygnaly, NULL), "pthread_join ratownik 1 - wątek t_sygnaly");

        // Niszczenie mutexa
        sprawdz_blad_watek(pthread_mutex_destroy(&mutex_olimpijski), "pthread_mutex_destroy ratownik 1 - mutex olimpijski");
    }
    else if(pool_id == 2){
        // Inicjalizacja tablicy do basenu, rozmiar basenu + 1 na przechowanie liczby klientów
        int klienci[2][pool_size + 1];
        for (int i = 0; i <= pool_size; i++) {
            klienci[0][i] = 0;
            klienci[1][i] = 0;
        }
        // Inicjalizacja mutexa
        sprawdz_blad_watek(pthread_mutex_init(&mutex_rekreacyjny, NULL), "pthread_mutex_init ratownik 2 - mutex rekreacyjny");

        // Tworzenie wątków
        sprawdz_blad_watek(pthread_create(&t_wpuszczanie, NULL, &rekreacyjny, klienci), "pthread_create ratownik 2 - wątek t_wpuszczanie");
        sprawdz_blad_watek(pthread_create(&t_wychodzenie, NULL, &wychodzenie_rekreacyjny, klienci), "pthread_create ratownik 2 - wątek t_wychodzenie_rekreacyjny");
        sprawdz_blad_watek(pthread_create(&t_sygnaly, NULL, &sygnal, klienci), "pthread_create ratownik 2 - wątek t_sygnaly");

        // Oczekiwanie na zakończenie wątków
        sprawdz_blad_watek(pthread_join(t_wpuszczanie, NULL), "pthread_join ratownik 2 - wątek t_wpuszczanie");
        sprawdz_blad_watek(pthread_join(t_wychodzenie, NULL), "pthread_join ratownik 2 - wątek t_wychodzenie");
        sprawdz_blad_watek(pthread_join(t_sygnaly, NULL), "pthread_join ratownik 2 - wątek t_sygnaly");

        // Niszczenie mutexa
        sprawdz_blad_watek(pthread_mutex_destroy(&mutex_rekreacyjny), "pthread_mutex_destroy ratownik 2 - mutex rekreacyjny");
    }
    else if(pool_id == 3){
        // Inicjalizacja tablicy do basenu rozmiar basenu/2 + 1 na przechowanie liczby klientów, zawsze wchodza dwie osoby dziecko + opiekun
        int klienci[(pool_size / 2) + 1];
        for (int i = 0; i <= (pool_size/2); i++) {
            klienci[i] = 0;
        }
        // Inicjalizacja mutexa
        sprawdz_blad_watek(pthread_mutex_init(&mutex_brodzik, NULL), "pthread_mutex_init ratownik 3 - mutex brodzik");

        // Tworzenie wątków
        sprawdz_blad_watek(pthread_create(&t_wpuszczanie, NULL, &brodzik, klienci), "pthread_create ratownik 3 - wątek t_wpuszczanie");
        sprawdz_blad_watek(pthread_create(&t_wychodzenie, NULL, &wychodzenie_brodzik, klienci), "pthread_create ratownik 3 - wątek t_wychodzenie_brodzik");
        sprawdz_blad_watek(pthread_create(&t_sygnaly, NULL, &sygnal, klienci), "pthread_create ratownik 3 - wątek t_sygnaly");

        // Oczekiwanie na zakończenie wątków
        sprawdz_blad_watek(pthread_join(t_wpuszczanie, NULL), "pthread_join ratownik 3 - wątek t_wpuszczanie");
        sprawdz_blad_watek(pthread_join(t_wychodzenie, NULL), "pthread_join ratownik 3 - wątek t_wychodzenie");
        sprawdz_blad_watek(pthread_join(t_sygnaly, NULL), "pthread_join ratownik 3 - wątek t_sygnaly");

        // Niszczenie mutexa
        sprawdz_blad_watek(pthread_mutex_destroy(&mutex_brodzik), "pthread_mutex_destroy ratownik 3 - mutex brodzik");
    }

    // Odłączenie pamięci
    sprawdz_blad(shmdt(shared_data), "Błąd shmdt shared_data (ratownik) - odłączanie pamięci.");
    return 0;
}

void *olimpijski(void *arg) {
    int *klienci = (int *)arg;

    while (1) {
        // Odbieranie komunikatu od klienta
        sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 1, 0), "Blad msgrcv msgrID (ratownik 1) - odebranie komunikatu od klienta");

        // Blokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_lock(&mutex_olimpijski), "Blad pthread_mutex_lock (ratownik 1) - mutex olimpijski");

        // Ustawienie typu komunikatu na pid obsługiwanego klienta
        msgr.mtype = msgr.pid;

        // Sprawdzenie warunków regulaminu
        if(!czynny){
            msgr.kom = 'c';
        }
        else if (msgr.wiek < 18) {
            msgr.kom = 'w';
        } 
        else if (klienci[0] >= pool_size) {
            msgr.kom = 'n';
        } 
        else {
            // Warunki spełnione, następuje dodanie klienta do tablicy klienci
            for (int i = 1; i <= pool_size; i++) {
                if (klienci[i] == 0) {
                    klienci[i] = msgr.pid;
                    klienci[0]++;
                    msgr.kom = 't';
                    break;
                }
            }
            printf("\nStan tablicy klienci po dodaniu klienta OLIMPIJSKI:\n");
            wyswietl_klientow(klienci, pool_size + 1);
        }

        // Odblokowuje mutex
        sprawdz_blad_watek(pthread_mutex_unlock(&mutex_olimpijski), "Blad pthread_mutex_unlock (ratownik 1) - mutex olimpijski");
        
        // Wysyła komunikat do obsłużonego klienta
        sprawdz_blad(msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0), "Blad msgsnd msgrID (ratownik 1) - wysyłanie komunikatu do klienta");
    }
    // Zakończenie wątku  
    pthread_exit(NULL);
}

void *rekreacyjny(void *arg){
    int (*klienci)[pool_size + 1] = (int (*)[pool_size + 1])arg;

    while(1){
        // Odbieranie komunikatu od klienta
        sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 2, 0), "Blad msgrcv msgrID (ratownik 2) - odebranie komunikatu od klienta");

        // Blokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_lock(&mutex_rekreacyjny), "Blad pthread_mutex_lock (ratownik 2) - mutex rekreacyjny");

        // Ustawienie typu komunikatu na pid obsługiwanego klienta
        msgr.mtype = msgr.pid;

        // Zmienne do obliczeń
        int wiek_klienta = msgr.wiek;
        int liczba_osob = 0;
        double srednia = 0;

        // Suma wieków klientów obecnie przebywających w basenie
        for(int i = 1; i <= pool_size; i++){
            //printf("%d. %d\n", i ,klienci[1][i]);
            srednia += klienci[1][i];
        }

        // Jeżeli klient (dziecko) ma opiekuna to dodaje jego wiek do wieku klienta i zwiększa liczbe osób o 1
        if(msgr.wiek_opiekuna > 0){
            wiek_klienta = msgr.wiek + msgr.wiek_opiekuna;
            liczba_osob = 1;
        }
        
        // Sumuje liczbe klientów w basenie dodaje klienta i opiekuna jeżeli jest
        int liczba_klientow = klienci[0][0] + liczba_osob + 1;

        // Oblicza średnią z poprzedniej sumy, zsumowanego wieku klienta oraz zsumowanej liczby klientów
        // Sprawdza czy po wejściu danego klienta do basenu średnia zostanie przekroczona, nie czy aktualnie jest przekroczona
        srednia = (srednia + wiek_klienta) / liczba_klientow;
        //printf("srednia: %f\n", srednia); // Do sprawdzenia

        // Sprawdzenie warunków regulaminu
        if(!czynny){
            msgr.kom = 'c';
        }
        // Sprawdza czy klient lub klient i jego opiekun zmieszczą się w basenie
        else if (klienci[0][0] >= (pool_size - liczba_osob)) {
            msgr.kom = 'n';
        } else if (srednia > 40) {
            msgr.kom = 's';
        } else {
            // Warunki spełnione, następuje dodanie klienta do tablicy klienci
            for (int i = 1; i <= pool_size; i++) {
                if (klienci[0][i] == 0) {
                    klienci[0][i] = msgr.pid;
                    klienci[1][i] = wiek_klienta;
                    // Jeżeli jest opiekun to dodaje do basenu dwie osoby, inaczej jedną
                    if(msgr.wiek_opiekuna > 0){
                        klienci[0][0] += 2;
                    }
                    else{
                        klienci[0][0]++;
                    }
                    msgr.kom = 't';

                    printf("\nStan tablicy klienci po dodaniu klienta REKREACYJNY:\n");
                    printf("Liczba klientów na basenie: %d\n", klienci[0][0]);
                    for (int i = 1; i <= pool_size; i++) {
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
        // Odblokowuje mutex
        sprawdz_blad_watek(pthread_mutex_unlock(&mutex_rekreacyjny), "Blad pthread_mutex_unlock (ratownik 2) - mutex rekreacyjny");

        // Wysyła komunikat do obsłużonego klienta
        sprawdz_blad(msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0), "Blad msgsnd msgrID (ratownik 2) - wysyłanie komunikatu do klienta");
    }
    // Zakończenie wątku  
    pthread_exit(NULL);
}

void *brodzik(void *arg) {
    int *klienci = (int *)arg;

    while (1) {
        // Odbieranie komunikatu od klienta
        sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 3, 0), "Blad msgrcv msgrID (ratownik 3) - odebranie komunikatu od klienta");
        
        // Blokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_lock(&mutex_brodzik), "Blad pthread_mutex_unlock (ratownik 3) - mutex brodzik");

        // Ustawienie typu komunikatu na pid obsługiwanego klienta
        msgr.mtype = msgr.pid;
        
        // Sprawdzenie warunków regulaminu
        if(!czynny){
            msgr.kom = 'c';
        }
        else if (msgr.wiek > 5) {
            msgr.kom = 'w';
        } else if (klienci[0] >= pool_size) {
            msgr.kom = 'n';
        } else {
            // Warunki spełnione, następuje dodanie klienta do tablicy klienci
            for (int i = 1; i <= (pool_size / 2 + 1); i++) {
                if (klienci[i] == 0) {
                    klienci[i] = msgr.pid;
                    klienci[0] += 2; // Zawsze dodaje dwie osoby do basenu ponieważ zawsze wchodzi dziecko z opiekunem
                    msgr.kom = 't';

                    printf("\nStan tablicy klienci po dodaniu klienta BRODZIK:\n");
                    wyswietl_klientow(klienci, (pool_size / 2 + 1));

                    break;
                }
            }
        }
        // Odblokowuje mutex
        sprawdz_blad_watek(pthread_mutex_unlock(&mutex_brodzik), "Blad pthread_mutex_unlock (ratownik 3) - mutex brodzik");

        // Wysyła komunikat do obsłużonego klienta
        sprawdz_blad(msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0), "Blad msgsnd msgrID (ratownik 3) - wysyłanie komunikatu do klienta");
    }
    // Zakończenie wątku  
    pthread_exit(NULL);
}

void *wychodzenie_olimpijski(void *arg) {
    while (1) {
        int *klienci = (int *)arg;

        // Odbieranie komunikatu od klienta
        sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 4, 0), "Blad msgrcv msgrID (ratownik 1) - odebranie komunikatu wychodzenia od klienta");

        // Blokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_lock(&mutex_olimpijski), "Blad pthread_mutex_lock (ratownik 1) - mutex olimpijski wychodzenie");

        // Szukanie klienta o podanim pid i usuwanie go z tablicy
        for (int i = 1; i <= pool_size; i++) {
            if (klienci[i] == msgr.pid) {
                klienci[i] = 0;
                klienci[0]--;

                printf("\nStan tablicy klienci po usunięciu klienta OLIMPIJSKI:\n");
                wyswietl_klientow(klienci, pool_size + 1);

                break;
            }
        }

        // Odblokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_unlock(&mutex_olimpijski), "Blad pthread_mutex_unlock (ratownik 1) - mutex olimpijski wychodzenie");
    }

    pthread_exit(NULL);
}

void *wychodzenie_rekreacyjny(void *arg){
    while(1){
        int (*klienci)[pool_size + 1] = (int (*)[pool_size + 1])arg;

        // Odbieranie komunikatu od klienta
        sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 5, 0), "Blad msgrcv msgrID (ratownik 2) - odebranie komunikatu wychodzenia od klienta");

        // Blokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_lock(&mutex_rekreacyjny), "Blad pthread_mutex_lock (ratownik 2) - mutex rekreacyjny wychodzenie");

        // Szukanie klienta o podanim pid i usuwanie go z tablicy
        for(int i = 1; i <= pool_size; i++){
            if(klienci[0][i] == msgr.pid){
                klienci[0][i] = 0;
                klienci[1][i] = 0;
                if(msgr.wiek_opiekuna > 0){
                    klienci[0][0] -= 2; // Jeżeli jest opiekun to usuwa dwie osoby inaczej jedną
                }
                else{
                    klienci[0][0]--;
                }

                printf("\nStan tablicy klienci po usunięciu klienta REKREACYJNY:\n");
                printf("Liczba klientów na basenie: %d\n", klienci[0][0]);
                for (int i = 1; i <= pool_size; i++) {
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
        // Odblokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_unlock(&mutex_rekreacyjny), "Blad pthread_mutex_unlock (ratownik 2) - mutex rekreacyjny wychodzenie");
    }
    // Zakończenie wątku 
    pthread_exit(NULL);
}

void *wychodzenie_brodzik(void *arg){
    while (1) {
        int *klienci = (int *)arg;

        // Odbieranie komunikatu od klienta
        sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), 6, 0), "Blad msgrcv msgrID (ratownik 3) - odebranie komunikatu wychodzenia od klienta");

        // Blokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_lock(&mutex_brodzik), "Blad pthread_mutex_lock (ratownik 3) - mutex brodzik wychodzenie");

        // Szukanie klienta o podanim pid i usuwanie go z tablicy
        for (int i = 1; i <= (pool_size/2); i++) {
            if (klienci[i] == msgr.pid) {
                klienci[i] = 0;
                klienci[0] -= 2;    // zawsze usuwa dwie osoby klienta i opiekuna

                printf("\nStan tablicy klienci po usunięciu klienta BRODZIK:\n");
                wyswietl_klientow(klienci, (pool_size/2 + 1));

                break;
            }
        }  
        // Odblokowanie mutexa
        sprawdz_blad_watek(pthread_mutex_unlock(&mutex_brodzik), "Blad pthread_mutex_unlock (ratownik 3) - mutex brodzik wychodzenie");
    }
    // Zakończenie wątku 
    pthread_exit(NULL);
}

void* sygnal(void *arg){
    int *klienci = (int *)arg;

    // Przypisanie mutexa do blokowania w zależnosci od obsługiwanego basenu
    pthread_mutex_t *mutex = (pool_id == 1) ? &mutex_olimpijski : (pool_id == 2) ? &mutex_rekreacyjny : &mutex_brodzik;

    // Przypisanie rozmiaru basenu w zależności od obsługiwanego basenu
    int rozmiar = (pool_id == 1 ? pool_size : (pool_id == 2 ? pool_size : (pool_size/2)));

    // Inicjalizacja tablicy do przechowania klientów opuszczających basen
    int byli_klienci[rozmiar];
    for(int i = 0; i < rozmiar; i++)
        byli_klienci[i] = 0;
    
    // Przypisanie godziny wysyłania sygnałów w trakcie otwarcia kompleksu
    time_t send_signal1 = shared_data->otwarcie + rand() % (shared_data->dlugosc_otwarcia / 4) + 10;
    time_t send_signal2 = send_signal1 + rand() % (shared_data->dlugosc_otwarcia / 4) + 5;

    // Wyświetlenie godziny planowanego wysłania sygnału
    struct tm *sygnal_godzina;
    sygnal_godzina = localtime(&send_signal1);
    printf("%s[%02d:%02d:%02d %d]%s Wyslanie sygnalu 1.\n", CYAN, sygnal_godzina->tm_hour, sygnal_godzina->tm_min, sygnal_godzina->tm_sec, pool_id, RESET);
    sygnal_godzina = localtime(&send_signal2);
    printf("%s[%02d:%02d:%02d %d]%s Wyslanie sygnalu 2.\n", CYAN, sygnal_godzina->tm_hour, sygnal_godzina->tm_min, sygnal_godzina->tm_sec, pool_id, RESET);
    
    // Czeka na godzine wysłania sygnału
    while (time(NULL) < send_signal1) {
        sleep(1);
    }

    // Blokowanie mutexa
    sprawdz_blad_watek(pthread_mutex_lock(mutex), "Blad pthread_mutex_lock - mutex sygnał");
    
    local = czas();
    printf("%s[%02d:%02d:%02d  %d]%s RATOWNIK WYSYŁA SYGNAŁ NA WYJŚCIE Z BASENU NR.%d.\n", RED, local->tm_hour, local->tm_min, local->tm_sec, pool_id, RESET, pool_id);

    // Zmiana statusu dostępności basenu
    czynny = 0;

    // Jeżeli obsługiwany basen to basen rekreacyjny to usuwany jest zarówno pid klienta jak i wiek
    if(pool_id == 2){
        int (*klienci)[pool_size + 1] = (int (*)[pool_size + 1])arg;
        klienci[0][0] = 0;

        for (int i = 1; i <= rozmiar; i++) {
            if(klienci[0][i]){
                //printf("Wysyłanie SIGUSR1 do PID %d\n", klienci[0][i]);   // Do testowania
                // Wysyła sygnał do klienta
                kill(klienci[0][i], SIGUSR1);
                // Zapisuje klientów którzy opuszczają basen
                byli_klienci[i - 1] = klienci[0][i];
                klienci[0][i] = 0;
                klienci[1][i] = 0;
            }
        }

        printf("\nStan tablicy klienci po wysłaniu sygnału REKREACYJNY:\n");
        printf("Liczba klientów na basenie: %d\n", klienci[0][0]);
        for (int i = 1; i <= pool_size; i++) {
            if (klienci[0][i] == 0) {
                printf("Miejsce %d: PUSTE\n", i);
            } else {
                printf("Miejsce %d: PID klienta %d Wiek: %d\n", i, klienci[0][i], klienci[1][i]);
            }
        }
        printf("\n");
    }
    else{
        // Usuwa pidy klientów z tablicy i zeruje liczbe klientów
        for (int i = 1; i <= rozmiar; i++) {
            if(klienci[i]){
                //printf("Wysyłanie SIGUSR1 do PID %d\n", klienci[i]);  // Do testowania
                // Wysyła sygnał 1 do klienta
                kill(klienci[i], SIGUSR1);
                // Zapisuje klientów którzy opuszczają basen
                byli_klienci[i - 1] = klienci[i];
                klienci[i] = 0;
                klienci[0]--;
            }
        }
        if(pool_id == 1){
            printf("\nStan tablicy klienci po wysłaniu sygnału 1 OLIMPIJSKI:\n");
            wyswietl_klientow(klienci, pool_size + 1);
        }
        else{
            printf("\nStan tablicy klienci po wysłaniu sygnału 1 BRODZIK:\n");
            wyswietl_klientow(klienci, (pool_size / 2 + 1));
        }  
    }

    // Odblokowanie mutexa
    sprawdz_blad_watek(pthread_mutex_unlock(mutex), "Blad pthread_mutex_unlock - mutex sygnał");

    // Czeka na godzine wysłania drugiego sygnału
    while (time(NULL) < send_signal2) {
        sleep(1);
    }

    local = czas();
    printf("%s[%02d:%02d:%02d  %d]%s RATOWNIK WYSYŁA SYGNAŁ NA POWRÓT DO BASENU NR.%d.\n", RED, local->tm_hour, local->tm_min, local->tm_sec, pool_id, RESET, pool_id);

    // Zmiana statusu dostępności basenu
    czynny = 1;
    
    // Wysyła sygnał 2 do klienta
    for (int i = 0; i < rozmiar; i++) {
        if(byli_klienci[i]){
            //printf("Wysyłanie SIGUSR2 do PID %d\n", byli_klienci[i]); // Do testowania
            kill(byli_klienci[i], SIGUSR2);
        }
    }
    // Zakończenie wątku 
    pthread_exit(NULL);
}
