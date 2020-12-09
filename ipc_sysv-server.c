#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>

#define AUTHOR_SIZE 32
#define MSG_SIZE 64

char *sr = "[Serwer]:";
key_t memKey, semKey;
int memId, semId;

struct Wpis {
	int number;
	char author[AUTHOR_SIZE];
	char msg[MSG_SIZE];
};
struct Wpis *ksiega;
int capacity;
struct sembuf semBuf;

/* Ctrl + c --> SIGINT --> Przerwanie dzialania programu */
void sigintHandle(int signal) {
	printf("\n%s Otrzymano SIGINT (Ctrl+C) ---> zamkniecie serwera - inicjuje odlaczenie i usuniecie pamieci dzielonej\n", sr);

	/* Odlaczenie segmentu pamieci dzielonej */
	if((shmdt(ksiega) != 0)) {
		printf("%s Blad - problem z odlaczeniem pamieci dzielonej\n", sr);
		exit(1);
	}
	/* Usuniecie segmentu pamieci dzielonej */
	if((shmctl(memId, IPC_RMID, 0) != 0)) {
		printf("%s Blad - problem z usunieciem pamieci dzielonej\n", sr);
		exit(1);
	}
	printf("%s Usunieto segment pamieci dzielonej\n", sr);
	
	/* Usuwanie semafora */
	if((semctl(semId, IPC_RMID, 0)) == -1) {
		printf("%s Blad - problem z usunieciem semafora\n", sr);
		exit(1);
	}
	printf("%s Usunieto semafor\n", sr);

	exit(0);
}

/* Ctrl + z --> SIGSTOP --> Wyswietlenie listy skarg i wnioskow */
void sigstopHandle(int signal) {
	int i, ksiegaSize = ksiega[0].number;
	if(ksiegaSize < 0) {
		ksiegaSize = 0;
	}
	if(ksiegaSize == capacity) {
		printf("\n%s Ksiega skarg i wnioskow jest jeszcze pusta\n", sr);
	}
	else {
		printf("\n%s Zawartosc ksiegi skarg i wnioskow:\n", sr);
		for(i=capacity-1; i>=ksiegaSize; i--) {
			printf("[%s]: %s\n", ksiega[i].author, ksiega[i].msg);
		}
	}
}

int main(int argc, char **argv) {
	int i;
	char *file = "ksiega";
	
	/* Obsluga sygnalow - wyswietlenie ksiegi i przerwanie programu */
	signal(SIGINT, sigintHandle);
	signal(SIGTSTP, sigstopHandle);
	
	/* Obsluga bledow */
	if(argc < 2) {
		printf("%s Blad - prosze podac argument programu\n", sr);
		exit(1);
	}
	capacity = atoi(argv[1]); /* Pierwszy argument - pojemnosc ksiegi */

	/* Dynamiczna alokacja tablicy - ksiegi */
	ksiega = (struct Wpis *) malloc(capacity * sizeof(struct Wpis));

	/* Generowanie klucza dla pamieci dzielonej */
	printf("%s Tworze klucz dla pamieci dzielonej na podstawie pliku %s\n", sr, file);
	memKey = ftok(file, 1);
	if(memKey == -1) {
		printf("%s Blad - problem z wygenerowaniem klucza dla pamieci dzielonej\n", sr);
		exit(1);
	}
	printf("%s Wygenerowano klucz dla pamieci dzielonej (wartosc klucza: %ld)\n", sr, memKey);

	/* Generowanie klucza dla semafora */
	printf("%s Tworze klucz dla semafora na podstawie pliku %s\n", sr, file);
	semKey = ftok(file, 2);
	if(semKey == -1) {
		printf("%s Blad - problem z wygenerowaniem klucza dla semafora\n", sr);
		exit(1);
	}
	printf("%s Wygenerowano klucz dla semafora (wartosc klucza: %ld)\n", sr, semKey);

	/* Tworzenie segmentu pamieci dzielonej */
	printf("%s Tworze segment pamieci dzielonej na %d wpisow o pojemnosci 64B kazdy\n", sr, capacity);
	memId = shmget(memKey, capacity * sizeof(struct Wpis), 0644 | IPC_CREAT | IPC_EXCL);
	if(memId == -1) {
		printf("%s Blad - problem z utworzeniem segmentu pamieci dzielonej\n", sr);
		exit(1);
	}
	printf("%s Utworzono segment pamieci dzielonej (id: %d)\n", sr, memId);

	/* Tworzenie semafora */
	printf("%s Tworze semafor\n", sr);
	semId = semget(semKey, 1, 0644 | IPC_CREAT | IPC_EXCL);
	if(semId ==  -1) {
		printf("%s Blad - problem z utworzeniem semafora\n", sr);
		exit(1);
	}
	printf("%s Utworzono semafor (id: %d)\n", sr, semId);

	/* Inicjalizacja wartosci semafora */
	semBuf.sem_num = 0;
	semBuf.sem_op = 1;
	semBuf.sem_flg = 0;
	if((semop(semId, &semBuf, 1)) == -1) {
		printf("%s Blad - problem z inicjalizacja wartosci semafora\n", sr);
		exit(1);
	}
	printf("%s Zainicjalizowano wartosc semafora - zasoby dostepne\n", sr);

	/* Dolaczanie segmentu pamieci dzielonej */
	ksiega = (struct Wpis*) shmat(memId, (void*) 0, 0);
	if(ksiega == (struct Wpis*) -1) {
		printf("%s Blad - problem z dolaczeniem segmentu pamieci dzielonej\n", sr);
		exit(1);
	}
	printf("%s Podlaczono segment pamieci dzielonej\n", sr);

	/* Busy waiting */
	i = capacity-1;
	ksiega[0].number = capacity;
	ksiega[1].number = capacity;
	while(8) {
		if(strcmp(ksiega[i].msg, "") != 0) {
			ksiega[0].number--;
			i--;
		}
	}

	return 0;
}
