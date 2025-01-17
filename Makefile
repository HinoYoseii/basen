all: zarzadca klient kasjer ratownik
zarzadca: zarzadca.o
	gcc -pthread -o zarzadca zarzadca.o
zarzadca.o: zarzadca.c
	gcc -pthread -c zarzadca.c
klient: klient.o
	gcc -pthread -o klient klient.o
klient.o: klient.c
	gcc -pthread -c klient.c
kasjer: kasjer.o
	gcc -pthread -o kasjer kasjer.o
kasjer.o: kasjer.c
	gcc -pthread -c kasjer.c
ratownik: ratownik.o
	gcc -pthread -o ratownik ratownik.o
ratownik.o: ratownik.c
	gcc -pthread -c ratownik.c
utility: utility.o
	gcc -pthread -o utility utility.o
utility.o: utility.c utility.h
	gcc -pthread -c utility.c
