#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

pthread_mutex_t mutex_consumer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_producer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

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
    struct message *buffer; // Указатель на буфер
    int head; // Индекс головы
    int tail; // Индекс хвоста
    int size; // Размер буфера
    bool is_empty;
    int count_add;
    int count_errase;
} RingBuffer;

RingBuffer* init(int size) {        // Инициализация кольцевого буфера
    RingBuffer *rb = (RingBuffer*) malloc(sizeof(RingBuffer));
    rb->buffer = (struct message*) malloc(size * sizeof(struct message));
    rb->head = 0;
    rb->tail = 0;
    rb->size = size;
    rb->is_empty=true;
    rb->count_add=rb->count_errase=0;
    return rb;
}

bool enqueue(RingBuffer *rb, struct message data) {     // Добавление элемента в буфер
    if ((rb->head + 1) % rb->size == rb->tail) {
        printf("Буфер переполнен\n");
        rb->is_empty=false;
        return false;
    }
    rb->count_add++;
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % rb->size;
    //printf("%d\n",rb->head );
    return true;
}

bool dequeue(RingBuffer *rb, struct message* data) {        // Извлечение элемента из буфера
    if (rb->head == rb->tail) {
        printf("Буфер пуст\n");
        rb->is_empty=true;
        return false;
    }
    rb->count_errase++;
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    //printf("%d\n",rb->tail);
    return true;
}

void set_new_size(RingBuffer* rb, int new_size) {
    if(new_size<=0)
        return;
    pthread_mutex_lock(&mutex_consumer);
    pthread_mutex_lock(&mutex_producer);

    if(new_size>rb->size && rb->is_empty==false){
        rb->is_empty = true;
    }
    else if(new_size<rb->size && rb->is_empty==false) {
        rb->is_empty = false;
        rb->head--;
    }

    rb->size=new_size;
    rb->buffer= realloc(rb->buffer, new_size*sizeof(struct message));

    pthread_mutex_unlock(&mutex_consumer);
    pthread_mutex_unlock(&mutex_producer);
}

void destroy(RingBuffer *rb) {      // Освобождение памяти, занятой буфером
    free(rb->buffer);
    free(rb);
}

pthread_t* create_thread(pthread_t* threads, int* count, void* (*func) (void*)){
    (*count)++;
    threads = realloc(threads, (*count) * sizeof(pthread_t));
    int status = pthread_create(&threads[(*count)-1], NULL, func, NULL);
    if (status != 0) {
        perror(strerror(errno));
    }
    return threads;
}

void show_thread(pthread_t* threads, int count){
    printf("%d\n", count);
    for(int i=0;i<count;i++)
        printf("%ld\n", threads[i]);
}
