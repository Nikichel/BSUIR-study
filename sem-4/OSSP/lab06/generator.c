#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>

#define SIZE_MULTIPLICITY 256  //кратность количества записей

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

//старт программы
int main(int argc, char* argv[]){
	//файловый дискриптор
	FILE* file;

	//проверка аргументов командной строки
	if(argc != 3){
		fprintf(stderr, "Usage: %s <num_records> <filename>\n", argv[0]);
		return -1;
	}

	//заголовок
	struct index_hdr_s index_hdr;

	//получение количества записей
	if(((index_hdr.records = atoll(argv[1])) % 256) != 0 || index_hdr.records <= 0){
		if(index_hdr.records <= 0){
			fprintf(stderr, "count of records is uncorrect\n");
			return -1;
		}
		index_hdr.records = index_hdr.records - (index_hdr.records % 256) + 256;
	}

	//выделение памяти
	index_hdr.idx = (struct index_s*)malloc(index_hdr.records * sizeof(struct index_s));

	//генерация записей
	for(int i = 0; i < index_hdr.records; ++i){
		index_hdr.idx[i].time_mark = rand() % 36525 + 15020 + (double)rand() / RAND_MAX;
		index_hdr.idx[i].recno = rand() % index_hdr.records+1;
		printf("%f - %lld\n", index_hdr.idx[i].time_mark, index_hdr.idx[i].recno);
	}

	//открытие файла
    if((file = fopen(argv[2], "wb")) == NULL){
        fprintf(stderr, "file cannot openned/created\n");
		return -1;
    }

    //запись
    fwrite(index_hdr.idx , sizeof(struct index_s), index_hdr.records, file);
    fwrite(&(index_hdr.records), sizeof(uint64_t), 1, file);

    printf("SIZE=%lld\n", index_hdr.records*sizeof(struct index_s));
    printf("COUNT RECORDS=%lld\n", index_hdr.records);
	//закрытие файла
    fclose(file);
	//очистка памяти
	free(index_hdr.idx);
	return 0;
}

