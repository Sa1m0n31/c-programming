#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<string.h>

#define AUTHOR_SIZE 32
#define MSG_SIZE 64

struct Wpis {
	int number;
	char author[AUTHOR_SIZE];
	char msg[MSG_SIZE];
};
struct Wpis *ksiega;

struct sembuf semBuf;

key_t memKey, semKey;
int memId, semId;

long int ktoryWpis;
char *kl = "[Klient]:";
char *username;
char buf[MSG_SIZE];

void zwolnijSemafor() {
	/* Wyjscie z sekcji krytycznej - odblokowanie semafora */
	semBuf.sem_op = 1;
	if((semop(semId, &semBuf, 1)) == -1) {
		printf("%s Blad - problem z odblokowaniem semafora\n", kl);
		exit(1);
	}
	printf("%s Odblokowano dostep do zasobow pozostalym procesom \n", kl);
	printf("%s Koncze dzialanie\n", kl);
	exit(0);
}

int main(int argc, char **argv) {
	struct Wpis newWpis;

	/* Tworzenie klucza */
	if(argc == 1) {
		printf("%s Blad - brak argumentu programu\n", kl);
		exit(1);
	}
	else if(argc == 2) {
		printf("%s Blad - brak drugiego argumentu programu (nazwa uzytkownika)\n", kl);
		exit(1);
	}
	
	/* Zapamietanie autora wpisu */
	strcpy(newWpis.author, argv[2]);
	
	/* Tworzenie klucza dla pamieci dzielonej */
	memKey = ftok(argv[1], 1);
	if(memKey == -1) {
		printf("%s Blad - problem z generowaniem klucza dla pamieci dzielonej\n", kl);
		exit(1);
	}
	printf("%s Wygenerowano klucz dla pamieci dzielonej o numerze %ld\n", kl, memKey);

	/* Tworzenie klucza dla semafora */
	semKey = ftok(argv[1], 2);
	if(semKey == -1) {
		printf("%s Blad - problem z generowaniem klucza dla semafora\n", kl);
		exit(1);
	}
	printf("%s Wygenerowano klucz dla semafora o numerze %ld\n", kl, semKey);

	/* Otwieranie segmentu pamieci dzielonej */
	memId = shmget(memKey, 0, 0);
	if(memId == -1) {
		printf("%s Blad - problem z otworzeniem segmentu pamieci dzielonej\n", kl);
		exit(1);
	}
	printf("%s Otworzono segment pamieci dzielonej o id = %d\n", kl, memId);

	/* Podlaczenie segmentu pamieci dzielonej */
	ksiega = (struct Wpis*) shmat(memId, (void*)0, 0);
	if(ksiega == (struct Wpis*) -1) {
		printf("%s Blad - problem z podpieciem segmentu pamieci dzielonej\n", kl);
		exit(1);
	}

	/* Otwieranie semafora */
	semId = semget(semKey, 0, 0);
	if(semId == -1) {
		printf("%s Blad - problem z otworzeniem semafora\n", kl);
		exit(1);
	}
	printf("%s Otworzono semafor o id = %d\n", kl, semId);

	/* Wejscie do sekcji krytycznej - sprawdzenie czy semafor jest odblokowany i blokowanie semafora */
	semBuf.sem_num = 0;
	semBuf.sem_op = -1;
	semBuf.sem_flg = 0;
	if((semop(semId, &semBuf, 1)) == -1) {
		printf("%s Blad - problem ze zmiana wartosci semafora\n", kl);
		exit(1);
	}
	printf("%s Zablokowano dostep do zasobow pozostalym procesom\n", kl);

	/* Sprawdzenie czy jest miejsce w ksiedze */
	ktoryWpis = ksiega[0].number-1;
	if(ktoryWpis < 0) {
		printf("%s Brak miejsca w ksiedze skarg i wnioskow\n", kl);
		zwolnijSemafor();
	}

	/* Przyjmowanie wpisow do ksiegi */
	printf("%s [Wolnych %d wpisow (na %d)]\n", kl, ksiega[0].number, ksiega[1].number);
	printf("%s Napisz co ci doskwiera: ", kl);
	fgets(buf, MSG_SIZE, stdin);

	buf[strlen(buf)-1] = '\0';
	strcpy(newWpis.msg, buf);
	newWpis.number = ktoryWpis;
	
	memcpy(&ksiega[ktoryWpis], &newWpis, sizeof(struct Wpis));
	printf("%s Zapisano wpis do pamieci dzielonej\n", kl);

	/* Odlaczenie pamieci dzielonej */
	printf("%s Odlaczam pamiec dzielona\n", kl);
	shmdt(ksiega);

	zwolnijSemafor();

	return 0;
}
