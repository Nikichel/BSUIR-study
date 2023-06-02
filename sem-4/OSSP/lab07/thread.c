#define _GNU_SOURCE
#include "header.h"

RingBuffer* buf;

bool *flag_produser=NULL;
bool *flag_consumer=NULL;
int count_consumers=0;
int count_producers=0;

void* consumer_fun(){
    struct message ms;
    int index=count_consumers-1;
    while(flag_consumer[index]){
        sleep(1);
        pthread_mutex_lock(&mutex_consumer);
        while (buf->is_empty && flag_consumer[index]) {
            pthread_cond_wait(&buf_full, &mutex_consumer);
        }
        bool ans=dequeue(buf,&ms);     //критическая секция
        if(buf->is_empty){
           pthread_cond_broadcast(&buf_empty);
        }
        pthread_mutex_unlock(&mutex_consumer);
        pthread_mutex_lock(&mutex);
        if(ans) {
            print_message(ms);
            printf("GETS: %d\n", buf->count_errase);
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("CONSUMER %d ends\n", index);
    return NULL;
}

void* producer_fun(){
    struct message ms;
    int index=count_producers-1;
    while(flag_produser[index]){
        sleep(1);
        ms=init_mes();
        pthread_mutex_lock(&mutex_producer);
        while (!buf->is_empty && flag_produser[index]) {
            pthread_cond_wait(&buf_empty, &mutex_producer);
        }
        bool ans=enqueue(buf,ms);         //критическая секция
        if(!buf->is_empty){
            pthread_cond_broadcast(&buf_full);
        }
        pthread_mutex_unlock(&mutex_producer);
        pthread_mutex_lock(&mutex);
        if(ans)
            printf("SENDS: %d\n",buf->count_add);
        pthread_mutex_unlock(&mutex);
    }
    printf("PRODUSER %d ends\n", index);
    return NULL;
}

int main() {
    pthread_t* consumers=NULL;
    pthread_t* producers=NULL;
    char option[5];

    buf= init(10);
    while(1){
        strcpy(option,"\0");
        fgets(option, sizeof(option), stdin);
        if(strcmp(option,"+\n")==0){      //создать producer
            producers=create_thread(producers, &count_producers, producer_fun);
            flag_produser=realloc(flag_produser, count_producers*sizeof(bool));
            flag_produser[count_producers-1]=true;
        }
        else if(strcmp(option,"-\n")==0) {      //завершить producer
            if(count_producers){
                flag_produser[count_producers-1]=false;
                pthread_cond_broadcast(&buf_empty);
                pthread_join(producers[count_producers-1], NULL);
                count_producers--;
                producers = realloc(producers, count_producers * sizeof(pthread_t));
                flag_produser=realloc(flag_produser, count_producers * sizeof(bool));
            }
        }
        else if(strcmp(option,"a\n")==0){         //создать consumer
            consumers=create_thread(consumers, &count_consumers, consumer_fun);
            flag_consumer=realloc(flag_consumer,count_consumers*sizeof(bool));
            flag_consumer[count_consumers-1]=true;
        }
        else if(strcmp(option,"d\n")==0){        //завершить consumer
            if(count_consumers) {
                flag_consumer[count_consumers-1]=false;
                pthread_cond_broadcast(&buf_full);
                pthread_join(consumers[count_consumers-1], NULL);
                count_consumers--;
                consumers = realloc(consumers, count_consumers * sizeof(pthread_t));
                flag_consumer=realloc(flag_consumer,count_consumers*sizeof(bool));
            }
        }
        else if(strcmp(option,"l\n")==0){
            printf("PRODUCERS: ");
            show_thread(producers, count_producers);
            printf("CONSUMERS: ");
            show_thread(consumers, count_consumers);
        }
        else if(strcmp(option,"q\n")==0){       //выход из программы
            for(int i=0;i<count_consumers;i++){
                flag_consumer[i]=false;
                pthread_cond_broadcast(&buf_full);
                pthread_join(consumers[i], NULL);   //ожидание конца
            }
            for(int i=0;i<count_producers;i++){
                flag_produser[i]=false;
                pthread_cond_broadcast(&buf_empty);
                pthread_join(producers[i], NULL);
            }
            free(consumers);
            free(producers);
            free(flag_consumer);
            free(flag_produser);
            break;
        }
    }
    pthread_mutex_destroy(&mutex_consumer);
    pthread_mutex_destroy(&mutex_producer);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&buf_full);         // освобождение ресурсов
    pthread_cond_destroy(&buf_empty);

    destroy(buf);
    return 0;
}
