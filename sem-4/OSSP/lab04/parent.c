#define _GNU_SOURCE
#include "header.h"

int main(int argc, char* argv[]) {
    key_t stdout_key = ftok("./", 2);
    key_t producer_key = stdout_key+1;
    key_t consumer_key=stdout_key+2;
    int producer_sem=semget(producer_key,1, IPC_CREAT);     //cемафор для producer
    int consumer_sem=semget(consumer_key,1, IPC_CREAT);     //семафор для consumer
    int stdout_sem=semget(stdout_key ,1, IPC_CREAT);      //семофор вывода
    semctl(producer_sem,1,SETVAL, 1);
    semctl(consumer_sem,1,SETVAL, 1);    //установка в 1
    semctl(stdout_sem,1,SETVAL, 1);
    pid_t* producer_proc=NULL, *consumer_proc=NULL;
    int number_of_producer, number_of_consumer;     //кол-во процессов
    number_of_producer=number_of_consumer=0;
    char option[5];        //опция

    int shmid;
    RingBuffer *shmaddr;

    // создание разделяемой памяти
    shmid = shmget(IPC_PRIVATE, sizeof (RingBuffer), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Ошибка при создании разделяемой памяти");
        exit(EXIT_FAILURE);
    }

    // присоединение разделяемой памяти к адресному пространству процесса
    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (RingBuffer*) -1) {
        perror("Ошибка при присоединении разделяемой памяти");
        exit(EXIT_FAILURE);
    }

    *shmaddr = init_buf();      //внесение буфера в разделяемую память

    char* args[] ={int_to_str(shmid), int_to_str(producer_key), int_to_str(consumer_key),int_to_str(stdout_key),NULL};
    while(1){
      strcpy(option,"\0");
      fgets(option, sizeof(option), stdin);    //ввод опции
      
      if(strcmp(option,"+\n")==0){      //создать producer
          producer_proc=create_proc(producer_proc, number_of_producer, "./producer", args);
          number_of_producer++;
      }
      else if(strcmp(option,"-\n")==0) {      //завершить producer
          if(number_of_producer){
              number_of_producer--;
              producer_proc=stop_proc(producer_proc,number_of_producer);
          }
      }
      else if(strcmp(option,"a\n")==0){         //создать consumer
          consumer_proc=create_proc(consumer_proc, number_of_consumer, "./consumer", args);
          number_of_consumer++;
      }
      else if(strcmp(option,"d\n")==0){        //завершить consumer
          if(number_of_consumer) {
              number_of_consumer--;
              consumer_proc=stop_proc(consumer_proc, number_of_consumer);
          }
      }
      else if(strcmp(option,"l\n")==0){
          printf("PRODUCERS: ");
          show_proc(producer_proc,number_of_producer);
          printf("CONSUMERS: ");
          show_proc(consumer_proc, number_of_consumer);
      }
      else if(!strcmp(option,"q\n")){       //выход из программы
          while (number_of_producer) {      //завершить все prodecer
              number_of_producer--;
              producer_proc=stop_proc(producer_proc,number_of_producer);
          }
          while (number_of_consumer) {      //заершить все consumer
              number_of_consumer--;
              consumer_proc=stop_proc(consumer_proc, number_of_consumer);
          }
          free(producer_proc);
          free(consumer_proc);
          sleep(2);
          break;
      }
    }

    // отсоединение разделяемой памяти от адресного пространства процесса
    if (shmdt(shmaddr) == -1) {
        perror("Ошибка при отсоединении разделяемой памяти");
        exit(EXIT_FAILURE);
    }

    // удаление разделяемой памяти
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("Ошибка при удалении разделяемой памяти");
        exit(EXIT_FAILURE);
    }
    return 0;

}
