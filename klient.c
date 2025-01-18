#include "utility.c"

void signal_handler(int signal_num);
int custom_rand(int min, int max, int exclude);
struct klient_dane klient;
int ban;

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0; // Możesz dodać SA_RESTART, jeśli potrzeba
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    srand(getpid());
    time_t now;
    struct tm *local;
    struct tm *wyjscie;

    int msgID, msgrID, nr_basenu, vip;
    key_t msg_key, msgr_key;
    struct msgbuf msg;
    struct msgbuf_r msgr;


    msg_key = ftok(".", 'M');
    sprawdz_blad(msg_key, "ftok M (klient)");
    msgID = msgget(msg_key, IPC_CREAT | 0666);
    sprawdz_blad(msgID, "msgget msgID (zarzadca)");

    msgr_key = ftok(".", 'R');
    sprawdz_blad(msgr_key, "ftok R (klient)");
    msgrID = msgget(msgr_key, IPC_CREAT | 0666);
    sprawdz_blad(msgrID, "msgget msgrID (zarzadca)");


    vip = (rand() % 5 + 1 == 5) ? 1 : 2;
    local = czas();
    if(vip == 1)
        printf("%s[%02d:%02d:%02d  %d]%s Pojawia się klient VIP.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
    else
        printf("%s[%02d:%02d:%02d  %d]%s Pojawia się klient.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
        
    klient.pid = getpid();
    //klient.wiek = 20;
    klient.wiek = (rand() % 70) + 1;
    klient.wiek_opiekuna = (klient.wiek < 10) ? ((rand() % 53) + 18) : 0;
    klient.basen = 0;

    msg.mtype = vip;
    msg.pid = klient.pid;
    msg.wiek = klient.wiek;
    msg.czas_wyjscia = 0;

    sprawdz_blad(msgsnd(msgID, &msg, sizeof(msg) - sizeof(long), 0), "Blad msgsnd msgID (klient) - wysyłanie komunikatu do kasjera o wejście na basen");
    
    sprawdz_blad(msgrcv(msgID, &msg, sizeof(msg) - sizeof(long), getpid(), 0), "Blad msgrcv msgID (klient) - odbieranie komunikatu od kasjera o wejście na basen");

    klient.czas_wyjscia = msg.czas_wyjscia;

    local = czas();
    printf("%s[%02d:%02d:%02d  %d]%s Klient wchodzi na kompleks basenowy.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);

    if(klient.wiek <= 3){
        local = czas();
        printf("%s[%02d:%02d:%02d  %d]%s Dziecko zakłada pampers do plywania. Wiek: %d\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET, klient.wiek);
    }

    if((rand() % 6 + 1 == 6) ? 1 : 0){
        local = czas();
        printf("%s[%02d:%02d:%02d  %d]%s Klient zakłada czepek.\n", GREEN, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
    }
    
    msgr.pid = getpid();
    msgr.wiek = klient.wiek;
    msgr.wiek_opiekuna = klient.wiek_opiekuna;
    int spanie;
    
    while (1){  
        if(!klient.basen){
            do {
                if(time(NULL) >= klient.czas_wyjscia)
                    break;
                //nr_basenu = 2;
                nr_basenu = (rand() % 3) + 1;
            } while (nr_basenu == ban);

            if (msgr.mtype <= 0) {
                fprintf(stderr, "Nieprawidłowy mtype: %ld\n", msgr.mtype);
                exit(EXIT_FAILURE);
            }

            msgr.mtype = nr_basenu;
            sprawdz_blad(msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0), "Blad msgsnd msgrID (klient) - wysyłanie komunikatu do ratownika o wejście do wybranego basenu");

            local = czas();
            printf("%s[%02d:%02d:%02d  %d]%s Klient chce wejść do basenu nr.%d.\n", MAGENTA, local->tm_hour, local->tm_min, local->tm_sec, msgr.pid, RESET, nr_basenu);


            sprawdz_blad(msgrcv(msgrID, &msgr, sizeof(msgr) - sizeof(long), getpid(), 0), "Blad msgsnd msgrID (klient) - odbieranie komunikatu od kasjera o wejście do wybranego basenu");

            if(msgr.kom == 't'){
                local = czas();
                printf("%s[%02d:%02d:%02d  %d]%s Klient wchodzi do basenu nr.%d.\n", MAGENTA, local->tm_hour, local->tm_min, local->tm_sec, msgr.pid, RESET, nr_basenu);
                klient.basen = nr_basenu;
                spanie = 1;
            }
            else{
                if(msgr.kom == 'c'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d ponieważ basen jest tymczasowo nieczynny.\n", YELLOW, getpid(), RESET, nr_basenu, klient.wiek);
                }
                else if(msgr.kom == 'w'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d przez wiek: %d.\n", YELLOW, getpid(), RESET, nr_basenu, klient.wiek);
                }
                else if(msgr.kom == 'n'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d przez pełny basen.\n", YELLOW, getpid(), RESET, nr_basenu);
                }
                else if(msgr.kom == 's'){
                    printf("%s[%d]%s Klient nie został wpuszczony do basenu nr.%d przez srednia wieku.\n", YELLOW, getpid(), RESET, nr_basenu);
                }
                spanie = ((rand() % 20) + 1);
            }
        }
        for(int i = 0; i < spanie; i++){
            if(time(NULL) < klient.czas_wyjscia)
                sleep(1);
            else
                break;
        }
        if(time(NULL) >= klient.czas_wyjscia)
            break;
    }
    
    local = czas();
    if(klient.basen){
        printf("%s[%02d:%02d:%02d  %d]%s Klient wychodzi z kompleksu i basenu nr.%d.\n", RED, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET, klient.basen);
        msgr.mtype = klient.basen + 3;
        sprawdz_blad(msgsnd(msgrID, &msgr, sizeof(msgr) - sizeof(long), 0), "Blad msgsnd msgrID (klient) - wysyłanie komunikatu do ratownika o wyjście z basenu.");
        
    }
    else{
        printf("%s[%02d:%02d:%02d  %d]%s Klient wychodzi z kompleksu basenowego.\n", RED, local->tm_hour, local->tm_min, local->tm_sec, getpid(), RESET);
    }
    
    return 0;
}

void signal_handler(int signal_num){
    if(signal_num == SIGUSR1){
        printf("%s[%d]%s Klient odebral sygnal 1 Wychodzi z basenu nr.%d.\n", RED, getpid(), RESET, klient.basen);
        ban = klient.basen;
        klient.basen = 0;
        
    }
    else if (signal_num == SIGUSR2) {
        printf("%s[%d]%s Klient odebrał sygnał 2. Znowu może wejśc na basen nr. %d\n", RED, getpid(), RESET, ban);
        ban = 0;
    }
}
