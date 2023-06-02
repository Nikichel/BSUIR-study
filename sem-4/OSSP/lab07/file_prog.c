#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

struct record_s {
    char name[80];
    char address[80];
    int semester;
};

int lock(int fd, int rec_no) {
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = rec_no * sizeof(struct record_s);
    fl.l_len = sizeof(struct record_s);
    return fcntl(fd, F_SETLKW, &fl);
}

int unlock(int fd, int rec_no) {
    struct flock fl;

    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = rec_no * sizeof(struct record_s);
    fl.l_len = sizeof(struct record_s);
    return fcntl(fd, F_SETLK, &fl);
}

void put(struct record_s rec, int fd, int rec_no) {
    lseek(fd, rec_no * sizeof(struct record_s), SEEK_SET);
    write(fd, &rec, sizeof(struct record_s));
}

void get(struct record_s *rec, int fd, int rec_no) {
    lseek(fd, rec_no * sizeof(struct record_s), SEEK_SET);
    read(fd, rec, sizeof(struct record_s));
}

void show_file(int fd){
    lseek(fd, 0, SEEK_SET);
    struct record_s rec;
    ssize_t nread;
    int num=0;
    while ((nread = read(fd, &rec, sizeof(rec))) > 0) {
        printf("%d)", num++);
        printf("\tName: %s\n", rec.name);
        printf("\tAddress: %s\n", rec.address);
        printf("\tSemester: %d\n\n", rec.semester);
    }
}

void printf_work_recno(int rec_no, struct record_s rec){
    printf("\nWORK_RECORD:\n");
    printf("\trec_no = %d\n", rec_no);
    printf("\tName: %s\n", rec.name);
    printf("\tAddress: %s\n", rec.address);
    printf("\tSemester: %d\n\n", rec.semester);
}

void modify_recno(struct record_s *rec){
    int option;
    while(1){
        printf("\n\tName: %s\n", rec->name);
        printf("\tAddress: %s\n", rec->address);
        printf("\tSemester: %d\n\n", rec->semester);
        printf("MODIFY\n1-Name\n2-Addres\n3-Semester\n0-exit\nOption:");
        scanf("%d", &option);
        char a;
        while ((a = getchar()) != '\n' && a != EOF);
        if(option == 1){
            char new_name[80];
            printf("New name: ");
            fgets(new_name, sizeof(new_name), stdin);
            strtok(new_name, "\n");
            strcpy(rec->name,new_name);
        }
        else if(option == 2){
            char new_addres[80];
            printf("New adress: ");
            fgets(new_addres, sizeof(new_addres), stdin);
            strtok(new_addres, "\n");
            strcpy(rec->address,new_addres);
        }
        else if(option == 3){
            int new_semester;
            printf("New semester: ");
            scanf("%d", &new_semester);
            rec->semester=new_semester;
        }
        else if (option == 0){
            return;
        }
    }
}

int main() {
    int fd = open("file.txt", O_RDWR);
    struct record_s work_rec, rec, rec_sav, rec_new;
    int rec_no=-1;
    work_rec.semester=0;
    int option=0;
    char c;
    while(1){
        printf_work_recno(rec_no, work_rec);
        printf("0-end\n1-show file\n2-get record by №\n3-modify record\n4-save record\nOprion:");
        scanf("%d", &option);
        while ((c = getchar()) != '\n' && c != EOF);
        if(option==0){
            close(fd);
            return 0;
        }
        else if(option == 1){
            show_file(fd);
        }
        else if(option == 2){
            printf("Input №: ");
            scanf("%d", &rec_no);
            get(&work_rec, fd, rec_no);
            get(&rec, fd, rec_no);
        }
        else if(option == 3){
            if(rec_no>0)
                modify_recno(&work_rec);
        }
        else if(option == 4){
            rec_sav = rec;
            if (memcmp(&rec, &work_rec, sizeof(struct record_s)) != 0) {
                printf("Запись модифицирована\n");
                lock(fd, rec_no);
                printf("Нажмите enter чтобы сохранить\n");
                while ((c = getchar()) != '\n' && c != EOF);
                get(&rec_new, fd, rec_no);
                if (memcmp(&rec_new, &rec_sav, sizeof(struct record_s)) != 0) {
                    printf("Другой процесс изменил запись\n\n");
                    unlock(fd, rec_no);
                    rec = rec_new;
                    work_rec = rec_new;
                    continue;
                }
                printf("Сохранениe\n");
                put(work_rec, fd, rec_no);
                unlock(fd, rec_no);
            }
        }
    }
}
