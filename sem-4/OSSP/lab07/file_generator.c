#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

struct record_s {
    char name[80];
    char address[80];
    int semester;
};

int main() {
    int fd = open("file.txt", O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    srand(time(NULL)); // инициализация генератора случайных чисел

    for (int i = 0; i < 10; i++) {
        struct record_s rec;
        sprintf(rec.name, "Student %d", rand() % 30 + i);
        sprintf(rec.address, "Address %d", rand() % 100 + i);
        rec.semester = rand() % 8 + 1; // случайное число от 1 до 8

        ssize_t nwritten = write(fd, &rec, sizeof(rec));
        if (nwritten == -1) {
            perror("write");
            return 1;
        }
    }

    close(fd);
    return 0;
}
