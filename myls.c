#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<dirent.h>
#include<unistd.h>

#include<sys/stat.h>
#include<sys/types.h>

#include<pwd.h>
#include<grp.h>

#include<time.h>
#include<fcntl.h>

/* Konwersja praw dostepu */
char* modConvert(char *modStr, char type, int tryb) {
	static char modPrint[10];
	int i, k = 1;

	/* Jesli jestesmy w trybie 1 - dodaj informacje o rodzaju pliku */
	if(tryb == 1) modPrint[0] = type;
	else k = 0;

	/* Przetwarzanie praw dostepu w trzech iteracjach - dla uzytkownika, grupy i pozostalych */
	for(i=0; i<3; i++) {
		switch(modStr[i]) {
		case '7':
			modPrint[k] = 'r';
			modPrint[k+1] = 'w';
			modPrint[k+2] = 'x';
			break;
		case '6':
			modPrint[k] = 'r';
			modPrint[k+1] = 'w';
			modPrint[k+2] = '-';
			break;
		case '5':
			modPrint[k] = 'r';
			modPrint[k+1] = '-';
			modPrint[k+2] = 'x';
			break;
		case '4':
			modPrint[k] = 'r';
			modPrint[k+1] = '-';
			modPrint[k+2] = '-';
			break;
		case '3':
			modPrint[k] = '-';
			modPrint[k+1] = 'w';
			modPrint[k+2] = 'x';
			break;
		case '2':
			modPrint[k] = '-';
			modPrint[k+1] = 'w';
			modPrint[k+2] = '-';
			break;
		case '1':
			modPrint[k] = '-';
			modPrint[k+1] = '-';
			modPrint[k+2] = 'x';
			break;
		default:
			modPrint[k] = '-';
			modPrint[k+1] = '-';
			modPrint[k+2] = '-';
		break;
	}
		k+=3;
	}
	return modPrint;
}

/* Konwersja czasu na lancuch znakow dla trybu 2 */
char* convertTime(const struct tm *timeinfo) {
	static char date[40];
	int day, month, year, hour, min, sec;
	char dayS[2], monthS[2], yearS[4], hourS[2], minS[2], secS[2];

	/* "Wyciaganie" poszczegolnych skladowych daty ze struktury tm */
	day = timeinfo->tm_mday;
	month = timeinfo->tm_mon+1;
	year = timeinfo->tm_year + 1900;
	hour = timeinfo->tm_hour;
	min = timeinfo->tm_min;
	sec = timeinfo->tm_sec;

	/* Zamiana int na lancuch znakow */
	sprintf(dayS, "%d", day);
	sprintf(monthS, "%d", month);
	sprintf(yearS, "%d", year);
	/* W przypadku czasu - jesli mamy jedna cyfre, dodaj wiodace zero */
	if(hour>9) sprintf(hourS, "%d", hour);
	else sprintf(hourS, "0%d", hour);
	if(min>9) sprintf(minS, "%d", min);
	else sprintf(minS, "0%d", min);
	if(sec>9) sprintf(secS, "%d", sec);
	else sprintf(secS, "0%d", sec);

	strcpy(date, dayS);
		
	/* Polskie nazwy miesiecy */
	switch(month) {
		case 1:
			strcat(date, " stycznia ");
			break;
		case 2:
			strcat(date, " lutego ");
			break;
		case 3:
			strcat(date, " marca ");
			break;
		case 4:
			strcat(date, " kwietnia ");
			break;
		case 5:
			strcat(date, " maja ");
			break;
		case 6:
			strcat(date, " czerwca ");
			break;
		case 7:
			strcat(date, " lipca ");
			break;
		case 8:
			strcat(date, " sierpnia ");
			break;
		case 9:
			strcat(date, " wrzesnia ");
			break;
		case 10:
			strcat(date, " pazdziernika ");
			break;
		case 11:
			strcat(date, " listopada ");
			break;
		case 12:
			strcat(date, " grudnia ");
			break;
		default:
			break;
	}

	/* Dodanie do zwracanego lancucha pozostalych skladowych daty */
	strcat(date, yearS);
	strcat(date, " ");
	strcat(date, hourS);
	strcat(date, ":");
	strcat(date, minS);
	strcat(date, ":");
	strcat(date, secS);
	return date;
	
}

int main(int argc, char **argv) {
	
	// Zmienne dla trybu pierwszego
	struct dirent **fileList;
	struct stat statbuf;
	int n, i = 0;
	char fileName[50] = "";
	char modStr[20] = "";
	char modPrint[10] = "";
	struct passwd *passwdp;
	struct group *groupp;
	struct tm *timeinfo;
	char timeBuff[20];
	char type[3] = "";
	char mode1[3] = "";
	int symbolicLink = 0;

	// Zmienne dla trybu drugiego
	char par[10], rozmiarBuff[10];
	int rodzajPliku = 0;
	char *line = NULL;
	size_t len = 0;
	FILE *fp;
	char c;
	char mode2[3] = "";
	char type2[3] = "";
	char curDir[100];
	char dowiazanieBuff[50];


	if(argc == 1) { /* Tryb pierwszy */
		if((n = scandir(".", &fileList, NULL, alphasort)) < 0) { /* alphasort odpowiada za sortowanie wedlug nazwy */
			perror("Blad");
		}
		else {
			/* Iteracja po kolejnych plikach katalogu */
			while(i<n) {
				/* Jesli nie uda sie odnalezc pliku - wroc do poczatku petli */
				if(lstat(fileList[i]->d_name, &statbuf) == -1) {
					i++;
					continue;
				}

				/* Zapis daty do struktury tm, a nastepnie do odpowiedniego formatu */
				timeinfo = localtime(&(statbuf.st_mtime));
				strftime(timeBuff, 20, "%m-%d %H:%M", timeinfo);
				
				/* Pobranie informacji o grupie i o uzytkowniku */
				groupp = getgrgid(statbuf.st_gid);
				passwdp = getpwuid(statbuf.st_uid);

				/* Zapis praw dostepu do lancucha znakow */				
				sprintf(modStr, "%o", statbuf.st_mode);
				
				if(strlen(modStr) == 5) { /* plik jest katalogiem */
					mode1[0] = modStr[2];
					mode1[1] = modStr[3];
					mode1[2] = modStr[4];
					strcpy(modPrint, modConvert(mode1, 'd', 1));
				}
				else { /* plik jest zwyklym plikiem lub linkiem symbolicznym */
					strncpy(type, modStr, 3);
					type[0] = modStr[0];
					type[1] = modStr[1];
					type[2] = modStr[2];
					mode1[0] = modStr[3];
					mode1[1] = modStr[4];
					mode1[2] = modStr[5];
					if(strcmp(type, "100") == 0) { /* plik jest zwyklym plikiem */
						strcpy(modPrint, modConvert(mode1, '-', 1));
					}
					else { /* plik jest linkiem symbolicznym */
						strcpy(modPrint, modConvert(mode1, 'l', 1));
						symbolicLink = 1;
					}
				}


				/* Obsluga pelnej nazwy w przypadku, gdy plik jest linkiem symbolicnzym */
				if(symbolicLink == 1) { /* link symboliczny */
					readlink(fileList[i]->d_name, fileName, 50);
					strcat(fileList[i]->d_name, " -> ");
					strcat(fileList[i]->d_name, fileName);
					strcpy(fileName, fileList[i]->d_name);
				}
				else { /* zwykly plik lub katalog */
					strcpy(fileName, fileList[i]->d_name);
				}
					printf("%s\t %4ld %20s %14s\t %14ld\t %20s\t %-30s\n", modPrint, statbuf.st_nlink, passwdp->pw_name, groupp->gr_name, statbuf.st_size, timeBuff, fileName);
				i++;
				symbolicLink = 0;	
		}
			free(fileList);
		}
	}
	/* Tryb drugi */
	else if(argc == 2) {
		strcpy(par, argv[1]);
		
		// Informacje o pliku
		if(lstat(par, &statbuf) != -1) {

			printf("Informacje o %s:\n", par);
			
			// Typ pliku
			sprintf(modStr, "%o", statbuf.st_mode);
			
			if(strlen(modStr) == 5) { /* plik jest katalogiem */
				mode2[0] = modStr[2];
				mode2[1] = modStr[3];
				mode2[2] = modStr[4];
				strcpy(modPrint, modConvert(mode2, 'd', 2));
				printf("Typ pliku: katalog\n");
			}
			else { /* plik jest zwyklym plikiem lub linkiem symbolicznym */
				strncpy(type2, modStr, 3);
				mode2[0] = modStr[3];
				mode2[1] = modStr[4];
				mode2[2] = modStr[5];
				if(!((modStr[0] == '1')&&(modStr[1] == '2')&&(modStr[2] == '0'))) { /* plik jest zwyklym plikiem  */
					strcpy(modPrint, modConvert(mode2, '-', 2));
					printf("Typ pliku: zwykly plik\n");
					rodzajPliku = 1;
				}
				else { /* plik jest linkiem symbolicznym */
					strcpy(modPrint, modConvert(mode2, 'l', 2));
					printf("Typ pliku: link symboliczny\n");
					rodzajPliku = 2;
				}
			}

			// Sciezka
			getcwd(curDir, 100);
			strcat(curDir, "/");
			strcat(curDir, par);
			printf("Sciezka:\t%s\n", curDir);
			
			// Wskazuje na
			if(rodzajPliku == 2) {
				getcwd(curDir, 100);
				readlink(par, dowiazanieBuff, 50);
				strcat(curDir, "/");
				strcat(curDir, dowiazanieBuff);
				printf("Wskazuje na:\t%s\n", curDir);
			}

			// Rozmiar
			sprintf(rozmiarBuff, "%ld", statbuf.st_size);
			c = rozmiarBuff[strlen(rozmiarBuff)-1];
			
			/* Instrukcja wartnkowa dla roznych koncowek */
			if(strcmp(rozmiarBuff, "1") == 0) {
				printf("Rozmiar:\t%s bajt\n", rozmiarBuff);
			}
			else if(((statbuf.st_size > 11)&&(statbuf.st_size < 22))||((c != '2')&&(c != '3')&&(c != '4'))) {
				printf("Rozmiar:\t%s bajtow\n", rozmiarBuff);
			}
			
			else {
				printf("Rozmiar:\t%s bajty\n", rozmiarBuff);			
			}
	
			// Uprawnienia
			printf("Uprawienia:\t%s\n", modPrint);

			// Ostatnio uzywany
			timeinfo = localtime(&(statbuf.st_atime));
			strcpy(timeBuff, convertTime(timeinfo));
			
			printf("Ostatnio uzywany:\t\t%s\n", timeBuff);

			// Ostatnio zmodyfikowany
			timeinfo = localtime(&(statbuf.st_mtime));
			strcpy(timeBuff, convertTime(timeinfo));
			
			printf("Ostatnio modyfikowany:\t\t%s\n", timeBuff);


			// Ostatnio zmieniony stan
			timeinfo = localtime(&(statbuf.st_ctime));
			strcpy(timeBuff, convertTime(timeinfo));

			printf("Ostatnio zmieniony stan:\t%s\n", timeBuff);
			
			// Poczatek zawartosci
			if((rodzajPliku == 1)&&((modPrint[2]!='x')&&(modPrint[5]!='x')&&(modPrint[8]!='x'))) {
				fp = fopen(par, "r");
				if(fp == NULL) {
					printf("Nie udalo sie otworzyc pliku\n");
				}
				else {
					printf("Poczatek zawartosci:\n");
					if(getline(&line, &len, fp) != -1) {
						printf("%s", line);
					}
					if(getline(&line, &len, fp) != -1) {
						printf("%s", line);
					}
					fclose(fp);
				}
			}
		}
		else { /* Pliku nie znaleziono */
			printf("Plik nie istnieje\n");
		}
		
	}
	/* Niepoprawna ilosc argumentow */
	else {
		printf("Niepoprawna liczba argumentow\n");
	}
	

	return 0;
}
