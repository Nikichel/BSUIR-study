#define _GNU_SOURCE
#include "header.h"

int flag=1;

void sig_handler(int sig) {
    if(sig==SIGUSR1){
        flag=0;
    }
}

int main(int argc, char* argv[]){
    int stdout_sem = semget (atoi(argv[3]) ,1, IPC_EXCL);
    int consumer_sem=semget(atoi(argv[2]),1, IPC_EXCL);
    struct message ms;
    signal(SIGUSR1,sig_handler);
    int shmid=atoi(argv[0]);

    // присоединение разделяемой памяти к адресному пространству процесса
    RingBuffer* shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (RingBuffer*) -1) {
        perror("Ошибка при присоединении разделяемой памяти");
        exit(EXIT_FAILURE);
    }

    while(flag){
        sleep(2);
        if(shmaddr->empty==0) {
            sem_wait(consumer_sem);     //ожидание
            bool ans=errase(shmaddr,&ms);     //критическая секция
            shmaddr=shmaddr;
            sem_signal(consumer_sem);   //сигнал
            sem_wait(stdout_sem);
            if(!(ms.hash && 11))
                printf("Message is corrupted\n");
            if(ans) {
                print_message(ms);
            }
            sem_signal(stdout_sem);   //сигнал
        }
    }

    // отсоединение разделяемой памяти от адресного пространства процесса
    if (shmdt(shmaddr) == -1) {
        perror("Ошибка при отсоединении разделяемой памяти");
        exit(EXIT_FAILURE);
    }
    exit(0);
}