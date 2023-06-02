#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>

#define PAGE_SIZE 4096
#define MAX_BLOCKS 256
#define MIN_THREADS 4
#define MAX_THREADS 32

//индексная запись
struct index_s {
    double time_mark;
    long long int recno;
};

//заголовок
struct index_hdr_s {
    long long int records;
    struct index_s* idx;
};

//структура для сортировки
struct to_sort {
	long long int block_size; //размер блока
	struct index_s** mas;     //указатель на область памяти
	int id;                   //айди потока
};

pthread_t* tids;  //массив потоков
int count_of_threads = 0;     //количество потоков
int isready[MAX_BLOCKS];      //массив для отмечания отсортированных
int count_of_blocks = 0;      //количество блоков
long long int now_size_blocks = 0;      //размер блоков при склеивании
struct to_sort* parametrs;  //параметры для потоков

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

//сравнение
int comp(const void* a, const void* b){
    const struct index_s *idx = (const struct index_s *) a;
    const struct index_s *idx2 = (const struct index_s *) b;
    return idx->time_mark - idx2->time_mark;
}

int is_power_of_two(int n) {        //проверка на степень 2
    if (n <= 0) {
        return 0;
    }
    return (n & (n - 1)) == 0;
}

//функция слияния
void mymerge(struct index_s* mas1, long long int size1, struct index_s* mas2, long long int size2){
	struct index_s* newmas = (struct index_s*)malloc((size1 + size2) *  sizeof(struct index_s));

	//индексы по массивам
	long long int i = 0;
	long long int g = 0;
	long long int r = 0;
	//проход по двум массивам
	while(i < size1 && g < size2){
		if(mas1[i].time_mark < mas2[g].time_mark){
			newmas[r++] = mas1[i++];
		} else {
			newmas[r++] = mas2[g++];
		}
	}
	//проход по первому массиву
	while(i < size1){
		newmas[r++] = mas1[i++];
	}
	//проход по второму массиву
	while(g < size2){
		newmas[r++] = mas2[g++];
	}
	//обнуление
	i = 0;
	g = 0;
	r = 0;
	//запись обратно отсортированной части
	while(i < size1){
		mas1[i++] = newmas[r++];
	}
	//запись обратно отсортированной части
	while(g < size2){
		mas2[g++] = newmas[r++];
	}
	//очистка памяти
	free(newmas);
}

//сортировка потоков
void* mysort(void* argv){
	struct to_sort* param = (struct to_sort*)argv;
	int index = param->id;
	int flag = 1;
	//ожидание начала сортировки
	pthread_barrier_wait(&barrier);

	while(flag == 1){
		flag = 0;
		//сортировка
		qsort((*(param->mas)) + (index * (param->block_size)), param->block_size, sizeof(struct index_s), (int(*)(const void* elem1, const void* elem2))comp);
        //выбор блока для сортировки
		pthread_mutex_lock(&mutex);
		for(int i = 0; i < count_of_blocks; ++i){
			if(isready[i] != 1){
				isready[i] = 1;
				index = i;
				flag = 1;
				break;
			}
		}
		pthread_mutex_unlock(&mutex);
	}
	pthread_barrier_wait(&barrier);

	//подготовка к склеиванию
	for(int i = 2; i <= count_of_blocks; i *= 2){
		//начало склеивания
		pthread_barrier_wait(&barrier);
		flag = 1;
		while(flag == 1){
			flag = 0;
			//выбор блока для склеивания
			pthread_mutex_lock(&mutex);
			for(int g = 0; g < count_of_blocks / i; ++g){
				if(isready[g] == 0){
					flag = 1;
					index = g * 2;
					isready[g] = 1;
					break;
				}
			}
			pthread_mutex_unlock(&mutex);
			if(flag == 1){
				//склеивание
				mymerge((*(param->mas)) + (index * now_size_blocks), now_size_blocks, (*(param->mas)) + (index * now_size_blocks) + now_size_blocks, now_size_blocks);
			}
		}
		//завершение склеивания
		pthread_barrier_wait(&barrier);
	}
	pthread_exit(NULL);
}

//копирование
void copy(struct index_s* elem1, struct index_s* elem2, long long int size){
	for(long long int i = 0; i < size; ++i){
		elem1[i] = elem2[i];
	}
}

//старт программы
int main(int argc, char* argv[]){
	FILE* file;
	struct index_hdr_s index_hdr;
	long long int memsize;
	tids = (pthread_t*)malloc(MAX_THREADS * sizeof(pthread_t));
	parametrs = (struct to_sort*)malloc(MAX_THREADS * sizeof(struct to_sort));

	//проверка аргументов командной строки
	if(argc != 5){
		fprintf(stderr, "Usage: %s <memsize> <blocks> <threads> <filename>\n", argv[0]);
		exit(1);
	}

	//размер буфера
	memsize = atoll(argv[1]);
	if(memsize <= 0 || memsize % PAGE_SIZE != 0){
		fprintf(stderr, "memsize is uncorrect\n");
		exit(1);
	}

	//количество потоков
	count_of_threads = atoi(argv[3]);
	if(count_of_threads < MIN_THREADS || count_of_threads > MAX_THREADS){
		fprintf(stderr, "Количество потоков должно быть от %d до %d\n", MIN_THREADS, MAX_THREADS);
		exit(1);
	}

	//количество блоков
	count_of_blocks = atoi(argv[2]);
	    if(!is_power_of_two(count_of_blocks)){
        fprintf(stderr, "Количество блоков не является числом 2^n\n");
        exit(1);
    }

    if(!count_of_blocks || !count_of_threads){
        fprintf(stderr, "Нельзя нулевые значения для num_block и num_threads\n");
        exit(1);
    }

	//открытие файла
    if((file = fopen(argv[4], "rwb+")) == NULL){
        fprintf(stderr, "file cannot openned\n");
		return -1;
    }

    fseek(file, -sizeof(uint64_t), SEEK_END);
    //чтение
    fread(&(index_hdr.records), sizeof(uint64_t), 1, file);
    //закрытие файла
    fclose(file);

    long long int size_to_get = 0;
    long long int start = 0;

    int fd = open(argv[4], O_RDWR);
    //размер в записях
    long long int real_length = memsize / sizeof(struct index_s);
	pthread_barrier_init(&barrier, NULL, count_of_threads + 1);

    while(index_hdr.records - (start * real_length) > 0){
    	//получение размера для считывания
    	if(index_hdr.records - (start * real_length) < real_length){
			size_to_get = index_hdr.records - (start * real_length);
		} else {
			size_to_get = real_length;
		}

//Этап сортировки

		//инициализация карты сортировки
		for(int i = 0; i < count_of_blocks; ++i){
			isready[i] = 0;
		}

		//подготовка к этапу сортировки
		for(int i = 0; i < count_of_threads; ++i){
			isready[i] = 1;
			parametrs[i].block_size = size_to_get / count_of_blocks;
			parametrs[i].id = i;
			parametrs[i].mas = &index_hdr.idx;
			pthread_create(&tids[i], NULL, mysort, &parametrs[i]);
		}
        int offset= start * memsize;
		//отображение файла
    	index_hdr.idx = mmap(NULL, size_to_get * sizeof(struct index_s), PROT_READ | PROT_WRITE,
								MAP_SHARED, fd, offset);
		//ожидание начала сортировки
		pthread_barrier_wait(&barrier);
		//ожидание завершения сортировки
		pthread_barrier_wait(&barrier);

        printf("__________________________________________________\n");
        printf("\nSORT\n");
        for(int i=0; i<size_to_get;i++)
            printf("Record %ld: time_mark=%f, recno=%lld\n", i+offset/sizeof(struct index_s), index_hdr.idx[i].time_mark, index_hdr.idx[i].recno);
        printf("__________________________________________________\n");

//Этап склеивания

		//подготовка к склеиванию
		for(int i = 2; i <= count_of_blocks; i *= 2){
			now_size_blocks = (size_to_get / count_of_blocks) * (i / 2);
			for(int g = 0; g < count_of_blocks / i; ++g){
				isready[g] = 0;
			}
			//начало склеивания
			pthread_barrier_wait(&barrier);
			//завершение склеивания
			pthread_barrier_wait(&barrier);
		}

		//завершение потоков
		for(int i = 0; i < count_of_threads; ++i){
			pthread_join(tids[i], NULL);
		}

        printf("__________________________________________________\n");
        printf("\nMERGE\n");
        for(int i=0; i<size_to_get;i++)
            printf("Record %ld: time_mark=%f, recno=%lld\n", i+offset/sizeof(struct index_s), index_hdr.idx[i].time_mark, index_hdr.idx[i].recno);
        printf("__________________________________________________\n");


		//сохранение
		munmap(index_hdr.idx, size_to_get * sizeof(struct index_s));

		++start;
    }

	pthread_barrier_destroy(&barrier);

    struct index_s* res1 = NULL;
	struct index_s* res2 = NULL;

	//склеивание отсортированных частей
	for(int i = real_length; i < index_hdr.records - size_to_get; i += real_length){
		res1 = mmap(NULL, i * sizeof(struct index_s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		res2 = mmap(NULL, real_length * sizeof(struct index_s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, i * sizeof(struct index_s));
		mymerge(res1, i, res2, real_length);
		munmap(res1, i * sizeof(struct index_s));
		munmap(res2, real_length * sizeof(struct index_s));
	}

	//склеивание последней отсортированной части
	if(index_hdr.records - size_to_get != 0){
		res1 = mmap(NULL, (index_hdr.records - size_to_get) * sizeof(struct index_s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		res2 = mmap(NULL, size_to_get * sizeof(struct index_s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, (index_hdr.records - size_to_get) * sizeof(struct index_s));

		mymerge(res1, index_hdr.records - size_to_get, res2, size_to_get);
		munmap(res1, (index_hdr.records - size_to_get) * sizeof(struct index_s));
		munmap(res2, size_to_get * sizeof(struct index_s));
	}

    free(parametrs);
    free(tids);
	close(fd);
	return 0;
}
