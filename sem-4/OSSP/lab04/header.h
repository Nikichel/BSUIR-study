#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>

#define size_buf 10     //размер кольцевого буфера

struct message{
    char type;
    short hash;
    unsigned char size;
    int data[64];
};

struct message init_mes(){
    struct message mes;
    mes.type='a';
    mes.hash=11;
    mes.size=rand()%256;
    int real_size;
    if(mes.size)
        real_size=((mes.size + 3)/4)*4;
    else
        real_size=256;
    for(int i=0;i<64;i++){
        if(i<real_size/4)
            mes.data[i]=rand()%100;
        else
            mes.data[i]=0;
    }

    return mes;

}

void print_message(struct message ms){
    int real_size;
    if(ms.size)
        real_size=((ms.size + 3)/4)*4;
    else
        real_size=256;
    printf("DATA: ");
    for(int i=0;i<real_size/4;i++){
        printf("%d\t", ms.data[i]);
    }
    printf("\n");
}

typedef struct {
    struct message buffer[size_buf];
    int head;
    int tail;
    int count_add;
    int count_errase;
    int empty;
} RingBuffer;

RingBuffer init_buf() {
    RingBuffer rb;
    rb.head = 0;
    rb.tail = 0;
    rb.count_add=0;
    rb.count_errase=0;
    rb.empty=1;
    return rb;
}

bool add(RingBuffer* rb ,struct message data){
    rb->count_add++;
    rb->buffer[rb->head]=data;
    rb->head++;
    if(rb->head >= size_buf){
        printf("Буфер полный\n");
        rb->head=9;
        rb->count_add--;
        rb->empty=0;
        return false;
    }
    return true;
}

bool errase(RingBuffer* rb,struct message* data){
    rb->count_errase++;
    *data=rb->buffer[rb->head];
    rb->head--;
    if(rb->head<0){
        printf("Буфер пустой\n");
        rb->head=0;
        rb->count_errase--;
        rb->empty=1;
        return false;
    }
    return true;
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

void sem_wait(int semid) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = SEM_UNDO;
    semop(semid, &op, 1);
}

void sem_signal(int semid){
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = SEM_UNDO;
    semop(semid, &op, 1);
}

pid_t* create_proc(pid_t* proc, int count, const char* name, char** args){
    proc= realloc(proc, (count+1) * sizeof(pid_t));
    pid_t tmp_pid=fork();       //создание процесса
    if(tmp_pid == 0){
        execv(name, args);
    }
    else{
        proc[count]=tmp_pid;
    }
    return proc;
}

pid_t* stop_proc(pid_t* proc, int count){
    kill(proc[count], SIGUSR1);     //закончить процесс
    proc = realloc(proc,count * sizeof(pid_t));
    return proc;
}

void show_proc(pid_t* proc, int count){
    printf("%d\n", count);
    for(int i=0;i<count;i++){
        printf("%d\n", proc[i]);
    }
}