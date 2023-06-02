#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>

#define SIZE_MULTIPLICITY 256  //кратность количества записей
#define JULIAN_DAY 60079.80069 //текущий юллианский день

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
	if(argc != 2){
		fprintf(stderr, "count of parametrs is uncorrect\n");
		return -1;
	}

	//заголовок
	struct index_hdr_s index_hdr;

	//открытие файла
    if((file = fopen(argv[1], "rb+")) == NULL){
        fprintf(stderr, "file cannot openned\n");
		return -1;
    }

    fseek(file, -sizeof(uint64_t), SEEK_END);
    //чтение
    fread(&(index_hdr.records), sizeof(uint64_t), 1, file);
    fseek(file, 0, SEEK_SET);
    //выделение памяти
	index_hdr.idx = (struct index_s*)malloc(index_hdr.records * sizeof(struct index_s));
	//чтение
    fread(index_hdr.idx , sizeof(struct index_s), index_hdr.records, file);
	//закрытие файла
    fclose(file);

    //генерация записей
	for(int i = 0; i < index_hdr.records; ++i){
		printf("%f - %lld\n", index_hdr.idx[i].time_mark, index_hdr.idx[i].recno);
	}

	//очистка памяти
	free(index_hdr.idx);
	return 0;
}
