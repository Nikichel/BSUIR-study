#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>

struct process{
    pid_t pid;          //pid
    char name[50];      //имя
    bool is_blocked;    //блокировка
};

struct process* child_processes=NULL;   //указатель на массив информации о процессах
int number_of_child_pids = 0;           //число процессов
bool stdout_is_free = true;

int get_num(char* str){         //число из строки
    char* num=NULL;
    int length=0;
    int i=0;
    while (str[i]!='\0') {
        if (str[i]>'0' && str[i]<'9'){      //цифра
            length++;
            num=realloc(num, length * sizeof(char));
            num[length-1]=str[i];           //добавляем число в троку числа
        }
        i++;
    }
    if (length) {
        return atoi(num);       //переводим число в int
    }
    return 0;
}

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

void signal_handler(int sig, siginfo_t *info, void *ucontext){      //обработчик сигналов
    if (sig==SIGUSR1) {
        if (!stdout_is_free) {
            kill((*info).si_value.sival_int, SIGUSR1);
        }
        else {
            stdout_is_free=false;
            kill((*info).si_value.sival_int, SIGUSR2);
        }
    }
    else if (sig==SIGALRM) {
        for (int i=0;i<number_of_child_pids;i++) {
            kill(child_processes[i].pid, SIGUSR2);
        }
    }
    if (sig==SIGUSR2)
        stdout_is_free=true;
}

void set_handler(){
    struct sigaction act;
    memset(&act, 0, sizeof(act));   	//заполнение структуры
    act.sa_sigaction = signal_handler;      //установка обработчика
    act.sa_flags = SA_SIGINFO;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGALRM);
    act.sa_mask = set;      	    //установка запретов на сигналы
    sigaction(SIGUSR1, &act, 0);
    sigaction(SIGUSR2, &act, 0);
    sigaction(SIGALRM, &act, 0);        //установка сигналов
}

void concatenate_strings(char *str1, char *str2, char *result) {
    strcpy(result, str1);   // копируем первую строку в результирующую
    strcat(result, str2);   // добавляем вторую строку к результирующей
}

void create_child_process(pid_t pid){       //создание дочернего процесса
    concatenate_strings("C_",int_to_str(number_of_child_pids-1),child_processes[number_of_child_pids-1].name);
    child_processes[number_of_child_pids-1].pid = pid;
    child_processes[number_of_child_pids-1].is_blocked=false;
}

void delete_process(struct process* child_processes, int num_process){      //удалить процесс
    printf("killing procces %d...\n", child_processes[num_process].pid);
    kill(child_processes[num_process].pid, SIGTERM);
}

int clean_all_process(struct process* child_processes, int count_process){      //очистить все процессы
    while (count_process) {
        delete_process(child_processes, count_process-1);
        count_process--;
    }
    if(child_processes!=NULL){
        free(child_processes);
        child_processes=NULL;
    }
    return count_process;
}

int main(){
    set_handler();      //обработчик сигналов
    char option[10];    //опция
    pid_t tmp_pid;
    while (1) {
        strcpy(option,"\0");
        fgets(option, sizeof(option), stdin);       //ввод опции
        if (strcmp(option, "+\n") == 0) {           //опция +
            tmp_pid = fork();       //создание процесса
            number_of_child_pids++;
            child_processes = realloc(child_processes, number_of_child_pids * sizeof(struct process));      //заполнение процесса
            create_child_process(tmp_pid);
            if (tmp_pid < 0) {
                fprintf(stderr,"%s\n" ,strerror(errno));
                exit(1);
            } else if (tmp_pid == 0) {
                execl("./child", "child", NULL);
            } else {
                printf("%s created with PID %d\n", child_processes[number_of_child_pids-1].name, tmp_pid);
                printf("Count of children: %d\n", number_of_child_pids);
            }
        }
        else if (strcmp(option, "-\n") == 0 && number_of_child_pids) {      //операция -

            delete_process(child_processes, number_of_child_pids-1);        //удалить процесс
            number_of_child_pids--;
            child_processes = realloc(child_processes, number_of_child_pids * sizeof(struct process));      //перераспределение памяти
            printf("Count of children: %d\n", number_of_child_pids);

        }
        else if (strcmp(option, "q\n") == 0) {      //операция q
            if(number_of_child_pids) {
                number_of_child_pids = clean_all_process(child_processes, number_of_child_pids);
            }
            break;
        }
        else if (strcmp(option, "l\n") == 0) {      //операция l
            printf("Parent: %d\n", (int)getpid());
            printf("Child:\n");
            for (int i=0;i<number_of_child_pids;i++) {
                printf("%s:\t%d\t", child_processes[i].name,child_processes[i].pid);        //вывод информации о дочерних процессах
                if (child_processes[i].is_blocked) {
                    printf("BLOCK\n");
                }
                else
                    printf("AVAILABLE\n");
            }

        }
        else if (strcmp(option, "k\n") == 0) {      //операция k
            if (number_of_child_pids) {
                number_of_child_pids = clean_all_process(child_processes, number_of_child_pids);
            }
        }
        else if (strcmp(option, "s\n") == 0) {      //операция s
            for (int i=0;i<number_of_child_pids;i++) {
                kill(child_processes[i].pid, SIGUSR1);      //отправка сигнала
                child_processes[i].is_blocked=true;
            }
        }
        else if (strcmp(option, "g\n") == 0) {      //операция g
            for (int i=0;i<number_of_child_pids;i++) {
                kill(child_processes[i].pid, SIGUSR2);      //отправка сигнала
                child_processes[i].is_blocked=false;
            }
        }
        else if (strstr(option,"<") && strstr(option,">")) {        //операции с <>
            int num_pid=get_num(option);
            if (num_pid>=number_of_child_pids) {
                printf("ERROR!Number of children: %d\n", number_of_child_pids);
                continue;
            }
            else if (option[0]=='s') {      //операция s<num>
                kill(child_processes[num_pid].pid, SIGUSR1);
                child_processes[num_pid].is_blocked=true;
                printf("%s is BLOCK\n",child_processes[num_pid].name);
            }
            else if (option[0]=='g') {      //операция g<num>
                kill(child_processes[num_pid].pid, SIGUSR2);
                child_processes[num_pid].is_blocked=false;
                printf("%s is AVAILABLE\n",child_processes[num_pid].name);
            }
            else if (option[0]=='p') {      //операция p<num>
                for (int i=0;i<number_of_child_pids;i++) {
                    kill(child_processes[i].pid, SIGUSR1);
                }
                kill(child_processes[num_pid].pid, SIGUSR2);
                sleep(27);
                alarm(7);       //будильник
            }
        }
    }
    return 0;
}

