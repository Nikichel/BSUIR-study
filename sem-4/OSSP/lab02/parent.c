#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>

char* int_to_str(int num) {       //int в char*
    int i = 0, rem, len = 0;
    char* str;
    int tmp_num=num;
    while(tmp_num != 0) {       //длина числа
        len++;
        tmp_num /= 10;
    }

    str = (char*) malloc((len + 1) * sizeof(char));     //выделение памяти

    do {
        rem = num % 10;
        str[i++] = rem + '0';
        num /= 10;
    }while(num != 0);       //перевод в строку
    str[i] = '\0';

    for(i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;        //реверс строки
    }
    return str;
}

void bubble_sort(char** strs,int size){ //сортировка пузырьком
	char* tmp;
	for (int i = 0; i < size; i++){
		for (int j = i + 1; j < size; j++){
			if (strcmp(strs[i],strs[j])>1){
				tmp=strs[i];
				strs[i]=strs[j];
				strs[j]=tmp;
			}
		}
	}
}

char* cut(char *str, int index){ //усечение строки
	char *rez = calloc((strlen(str)-index), sizeof(char));
		for(int i = index; i < (int)strlen(str); i++) 
			strncat(rez, &str[i], 1);
    return rez;
}

void find_from_env(char* name, char** args, char** env){
    int child_status=0;
    pid_t pid=fork(); //создание процесса
    if (pid!=0) {
        fprintf(stderr,"%s\n", strerror(errno));
        return;
    }
    execve(strcat(getenv("CHILD_PATH"), "/child"), args, env);
    waitpid(-1,&child_status,0);    //ожидение конца дочернего процесса
    printf("Child process have ended with %d exit status\n", child_status);
}

void scan_env(char* name, char** args, char** env){
    int i=0, child_status=0;
    char* path=NULL;
    while (strstr(env[i++],"CHILD_PATH")==NULL); //сканирование окружения
    path=cut(env[i-1],strlen("CHILD_PATH")+1); //запись пути к каталогу, где находится дочерняя программа
    pid_t pid = fork();
    if (pid!=0) {
        fprintf(stderr,"%s\n", strerror(errno));
        return;
    }
    execve(strcat(getenv("CHILD_PATH"), "/child"), args, env);
    waitpid(-1,&child_status,0); //ожидaние конца дочернего процесса
    printf("Child process have ended with %d exit status\n", child_status);
    free(path);
}

int main(int argc, char *argv[], char* env[]){
    if (argc!=2) {
        printf("Low arguments\n");
        return -1;
    }
	setlocale(LC_COLLATE,"C");
	extern char **environ; //окружение
	int size=0;
	char option[5];
    char name[] = "Child_0\0";
	char** args=malloc(4*sizeof(char*));
    args[0]=malloc(10*sizeof(char));
    args[1]=malloc(sizeof(argv[1])*sizeof(char));
    strcpy(args[1], argv[1]);
    args[2]=malloc(2*sizeof(char));
    args[3]=NULL;
	int counter=0;
	char* num;
	while (env[size]){
        size++;     //кол-во переменных окружения
    };
    bubble_sort(env,size-1); //сортировка
    size=0;
    while (env[size]) {
        printf("%s\n",env[size++]);
    }
	while (1) {
            strcpy(option,"\0");
       	    fgets(option, sizeof(option), stdin);
            num = int_to_str(counter);
            strcpy(name, "Child_");
       	    strcat(name, num);      //формирование нового название дочернего процесса
            strcpy(args[0],name);
            if (strcmp(option, "q\n") == 0){
		free(args[2]);
               	free(args[1]);
                free(args[0]);
                free(args);
       	 	return 0;
            }
            else if(strcmp(option, "+\n") == 0){
        	strcpy(args[2],"+\0");
                find_from_env(name, args, env);    //путь из окружения
       	  	counter++;
            }
            else if(strcmp(option, "*\n") == 0){
       	 	strcpy(args[2],"*\0");
        	scan_env(name, args, env);    //сканирование окружение
        	counter++;
            }
            else if(strcmp(option, "&\n") == 0){
        	strcpy(args[2],"&\0");
                scan_env(name, args, environ);    //сканирование extern окружения
       	        counter++;
            }        
    	}
} 

