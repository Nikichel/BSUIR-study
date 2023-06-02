#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct child_struct{        //структура пар int
    int num1,num2;
};

bool output_info = true;        //разрешение на вывод
int count00=0;      //кол-во {0,0}
int count01=0;      //кол-во {0,1}
int count10=0;      //кол-во {1,0}
int count11=0;      //кол-во {1,1}
struct child_struct statistic;     //статистика
int count_of_loop=0;        //кол-во циклов
bool get_signal=false;      //получен ли сигнал

struct child_struct fill_statistic(struct child_struct statistic){      //заполнение структуры
    static int counter;
    if (counter==0) {
        statistic.num1=0;
        statistic.num2=0;
        counter++;
    }
    else if (counter==1) {
        statistic.num1=1;
        statistic.num2=0;
        counter++;
    }
    else if (counter==2) {
        statistic.num1=0;
        statistic.num2=1;
        counter++;
    }
    else if (counter==3) {
        statistic.num1=1;
        statistic.num2=1;
        counter++;
    }
    else {
        counter=0;
    }
    return statistic;
}

void alarm_handler(int sig) {           //обработчик будильника
    if (statistic.num1==0 && statistic.num2==0) {
        count00++;
    }
    else if (statistic.num1==1 && statistic.num2==0) {
        count01++;
    }
    else if (statistic.num1==0 && statistic.num2==1) {
        count10++;
    }
    else if (statistic.num1==1 && statistic.num2==1) {
        count11++;
    }
    alarm(1+rand() % 5);
}

void usr_handler(int sig){      //обработчик SIRUSR1/2
    if (sig==SIGUSR1) {
        output_info=false;
        get_signal=true;
    }
    else if (sig==SIGUSR2) {
        output_info = true;
        get_signal=true;
    }
}

int main() {
    union sigval info;
    srand(time(NULL));
    signal(SIGUSR1, usr_handler);
    signal(SIGUSR2, usr_handler);
    signal(SIGALRM, alarm_handler);     //установка обработчиков
    alarm(1+rand() % 5);        //будильник (0 6)
    while (1) {
        sleep(1);
        statistic=fill_statistic(statistic);        //заполнение статистики
        if (count_of_loop>=15 && output_info) {     //прошло определенное кол-во циклов
            get_signal=false;
            alarm(0);       //выкл будильник
            info.sival_int = getpid();  //передача информации в родительский процесс
            while (!get_signal) {       //ожидание получения сигнала от родитеслького процесса
                sigqueue(getppid(), SIGUSR1, info);        //отправка сигнала+данных род. процессу
                sleep(2);
            }
            alarm(1+rand() % 5);        //будильник
            if (!output_info) {
                count_of_loop=0;
                output_info=true;
                continue;
            }
            printf("\nPPID - %d PID - %d\n", (int)getppid(), (int)getpid());
            printf("{0;0}: %d\n{0;1}: %d\n{1;0}: %d\n{1;1}: %d\n", count00,count01,count10,count11);        //вывод статистики
            count_of_loop=0;
            sigqueue(getppid(), SIGUSR2, info);
        }
        count_of_loop++;
    }
    exit(0);
}
