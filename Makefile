all: zarzadca klient kasjer
zarzadca: zarzadca.o
	gcc -o zarzadca zarzadca.o
zarzadca.o: zarzadca.c
	gcc -c zarzadca.c
klient: klient.o
	gcc -o klient klient.o
klient.o: klient.c
	gcc -c klient.c
kasjer: kasjer.o
	gcc -o kasjer kasjer.o
kasjer.o: kasjer.c
	gcc -c kasjer.c
utility: utility.o
	gcc -o utility utility.o
utility.o: utility.c
	gcc -c utility.c
