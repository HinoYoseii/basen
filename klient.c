#include "utility.c"

struct klient_dane klient;  // Struktura klienta
int ban;    // Numer basenu do wykluczenia z losowania dla klienta

void signal_handler(int signal_num);    // Funkcja obsługująca sygnały wysłane od ratownika

int main() {
    srand(getpid());

    // Przypisanie funkcji do obsługi sygnałów
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    struct tm *local;   // Wskaźnik do wyświetlania obecnego czasu
    struct tm *wyjscie; // Wskaźnik do wyświetlania czasu wyjścia klienta

    key_t msg_key, msgr_key; // Klucze do kolejej komunikatów
    int msgID, msgrID;  // ID kolejek komunikatów
    int nr_basenu, vip, spanie; // Zmienne do losowania basenów i statusu vip, zmienna do usypiania klientów między losowaniami
    struct msgbuf msg;  // Bufor komunikatu do kasjera
    struct msgbuf_r msgr;   // Bufor komunikatu do ratownika

    // Dołączenie do kolejki komunikatów klient <-> kasjer
    msg_key = ftok(".", 'M');
    sprawdz_blad(msg_key, "ftok M (klient)");
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    sprawdz_blad(msgID, "msgget msgID (klient)");

    // Dołączenie do kolejki komunikatów klient <-> ratownik
    msgr_key = ftok(".", 'R');
    sprawdz_blad(msgr_key, "ftok R (klient)");
    msgrID = msgget(msgr_key, IPC_CREAT | 0666);
    sprawdz_blad(msgrID, "msgget msgrID (klient)");

    // Losowanie statusu vip i wyświetlenie komunikatu o pojawieniu się klienta
    vip = (rand() % 5 + 1 == 5) ? 1 : 2;
    local = czas();
    if(vip == 1)
        printf("%s[%02d:%02d:%02d  %d]%s Pojawia się klient VIP.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
    else
        printf("%s[%02d:%02d:%02d  %d]%s Pojawia się klient.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
    
    // Przypisanie strukturze klienta danych
    klient.pid = getpid();  // PID procesu
    klient.wiek = (rand() % 70) + 1;    // Wiek losowany od 1 do 70
    //klient.wiek = 20; // Możliwośc ustawienia dokładnego wieku klienta do testowania
    klient.wiek_opiekuna = (klient.wiek < 10) ? ((rand() % 53) + 18) : 0;   // Jeżei klient ma mniej niż 10 lat to losuje wiek opiekuna od 18 do 70
    klient.basen = 0; // Basen na którym jest klient, początkowo 0 (jest na kompleksie basenowym)

    // Przygotowanie komunikatu do wysłania do kasjera
    msg.mtype = vip;   // Status vip staje się typem komunikatu
    msg.pid = klient.pid;
    msg.wiek = klient.wiek;
    msg.czas_wyjscia = 0;

    // Wysłanie komunikatu do kasjera
    sprawdz_blad(msgsnd(msgID, &msg, sizeof(msg) - sizeof(long), 0), "Blad msgsnd msgID (klient) - wysyłanie komunikatu do kasjera o wejście na basen");
    // Odebranie komunikatu od kasjera
    sprawdz_blad(msgrcv(msgID, &msg, sizeof(msg) - sizeof(long), klient.pid, 0), "Blad msgrcv msgID (klient) - odbieranie komunikatu od kasjera o wejście na basen");

    // Przypisanie klientowi godziny wyjścia odebranej z komunikatu od kasjera
    klient.czas_wyjscia = msg.czas_wyjscia;

    local = czas();
    printf("%s[%02d:%02d:%02d  %d]%s Klient wchodzi na kompleks basenowy.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, klient.pid, RESET);

    // Jeżeli klient ma 3 lata lub mniej to zakłada pampers :)
    if(klient.wiek <= 3){
        local = czas();
        printf("%s[%02d:%02d:%02d  %d]%s Dziecko zakłada pampers do plywania. Wiek: %d\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, klient.pid, RESET, klient.wiek);
    }

    // Losuje możliwość założenia czepka przez klienta
    if((rand() % 6 + 1 == 6) ? 1 : 0){
        local = czas();
        printf("%s[%02d:%02d:%02d  %d]%s Klient zakłada czepek.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, klient.pid, RESET);
    }
    
    // Przygotowanie komunikatu do wysłania do ratownika
    msgr.pid = klient.pid;
    msgr.wiek = klient.wiek;
    msgr.wiek_opiekuna = klient.wiek_opiekuna;

    // Pętla działa dopóki czas nie przekroczy czasu pobytu klienta
    while (time(NULL) < klient.czas_wyjscia){      
        // Jeżeli klient nie ma przypisanego basenu to następuje losowanie
        if(!klient.basen){
            // Losowanie basenu, jeżeli wylosowany basen to basen z zakazem wstępu po wysłaniu sygnału to losuje jeszcze raz
            do {
                //nr_basenu = 2;    // Mozliwość wybrania konkretnego basenu do testów
                nr_basenu = (rand() % 3) + 1;
            } while (nr_basenu == ban); 

            // Przypisanie numeru basenu do typu komunikatu i wysłanie komunikatu do odpowiedniego ratownika
            msgr.mtype = nr_basenu;
            sprawdz_blad(msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0), "Blad msgsnd msgrID (klient) - wysyłanie komunikatu do ratownika o wejście do wybranego basenu");
            // Wyświetlenie informacji o wybranym basenie
            local = czas();
            printf("%s[%02d:%02d:%02d  %d]%s Klient chce wejść do basenu nr.%d.\n", MAGENTA, local->tm_hour, local->tm_min, local->tm_sec, msgr.pid, RESET, nr_basenu);
            // Odbieranie komunikatu od ratownika o wstępie do basenu
            sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), klient.pid, 0), "Blad msgsnd msgrID (klient) - odbieranie komunikatu od kasjera o wejście do wybranego basenu");

            if(msgr.kom == 't'){
                // Klient wchodzi do basenu, przypisuje sobie wybrany basen, ustawia spanie = 1 tak żeby wykonał się sleep(1)
                local = czas();
                printf("%s[%02d:%02d:%02d  %d]%s Klient wchodzi do basenu nr.%d.\n", MAGENTA, local->tm_hour, local->tm_min, local->tm_sec, msgr.pid, RESET, nr_basenu);
                klient.basen = nr_basenu;
                spanie = 1;
            }
            else{
                // Klient nie jest wpuszczony do basenu, wyświetla się komunikat z jakiego powodu
                if(msgr.kom == 'c'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d ponieważ basen jest tymczasowo nieczynny.\n", YELLOW, klient.pid, RESET, nr_basenu, klient.wiek);
                }
                else if(msgr.kom == 'w'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d przez wiek: %d.\n", YELLOW, klient.pid, RESET, nr_basenu, klient.wiek);
                }
                else if(msgr.kom == 'n'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d przez pełny basen.\n", YELLOW, klient.pid, RESET, nr_basenu);
                }
                else if(msgr.kom == 's'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d przez srednia wieku.\n", YELLOW, klient.pid, RESET, nr_basenu);
                }
                spanie = ((rand() % 20) + 1);
            }
        }

        // Zamiast spania losowego przedziału pętla co sekunde sprawdza czy nie został przekroczony czas pobytu na basenie, jeżeli nie to dalej jest wykonywany sleep(1)
        for(int i = 0; i < spanie; i++){
            if(time(NULL) < klient.czas_wyjscia)
                sleep(1);
            else
                break;
        }
    }
    
    // Jeżeli czas pobytu został przekroczony to klient wychodzi z kompleksu
    local = czas();
    // Jeżeli był w basenie to wysyła komunikat do ratownika że wychodzi z tego basenu, inaczej klient po prostu wyświetla że opuszcza kompleks
    if(klient.basen){
        printf("%s[%02d:%02d:%02d  %d]%s Klient wychodzi z kompleksu i basenu nr.%d.\n", RED, local->tm_hour, local->tm_min, local->tm_sec, klient.pid, RESET, klient.basen);
        // Wysyłanie komunikatu do ratownika o wyjściu Ratownik 1: 4, Ratownik 2: 5, Ratownik 3: 6
        msgr.mtype = klient.basen + 3;
        sprawdz_blad(msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0), "Blad msgsnd msgrID (klient) - wysyłanie komunikatu do ratownika o wyjście z basenu.");
    }
    else{
        printf("%s[%02d:%02d:%02d  %d]%s Klient wychodzi z kompleksu basenowego.\n", RED, local->tm_hour, local->tm_min, local->tm_sec, klient.pid, RESET);
    }
    
    return 0;
}

// Obsługa sygnałów
void signal_handler(int signal_num){
    // Po odebraniu sygnału SIGUSR1 klient ustawia swój obecny basen jako zablokowany basen, a swój basen ustawia jako 0
    if(signal_num == SIGUSR1){
        printf("%s[%d]%s Klient odebral sygnal 1 Wychodzi z basenu nr.%d.\n", RED, klient.pid, RESET, klient.basen);
        ban = klient.basen;
        klient.basen = 0;
        
    }
    // Po odebraniu sygnału SIGUSR2 klient odblokowuje zabkolowany basen aby mógł go ponownie losować
    else if (signal_num == SIGUSR2) {
        //printf("%s[%d]%s Klient odebrał sygnał 2. Znowu może wejśc na basen nr. %d\n", RED, klient.pid, RESET, ban); // Do testowania
        ban = 0;
    }
}
