#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[], char* env[]){
	extern char **environ;
	printf("%s begin\n", argv[0]);
	printf("pid = %d , ppid = %d\n", (int)getpid(), (int)getppid()); //вывод pid и ppid
	printf("ENV:\n");
	FILE *File =fopen(argv[1],"r");
	if (File==NULL) {
		printf("Can't open file %s\n", argv[1]);
		exit(-1);
	}
	char option[256];
	while (1) {
		int i=0;
		fscanf(File,"%s", option); //чтение переменных из файла
		if (feof(File)) { //конец файла
			break;
		}
		if (strcpy(argv[2],"+")) {
			printf("%s=%s\n", option, getenv(option)); //звание из окружения
		}
		else if (strcpy(argv[2],"*")) {
			while (strstr(env[i++],option)==NULL) //сканирование окружения
				printf("%s\n", env[i]);
		}
		else if (strcpy(argv[2],"&")) {
			while (strstr(environ[i++],option)==NULL) //сканирование extern окружения
				printf("%s\n", environ[i]);
		}
	}
	exit(0);
} 
