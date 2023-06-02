#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define BUFFER_SIZE 1024
#define PORT 8081

char* removeSubstring(char* str, const char* sub) {
    size_t len = strlen(sub);
    if (len == 0) return str; // если подстрока пустая, возвращаем исходную строку

    char* p = str;
    while ((p = strstr(p, sub))) { // ищем первое вхождение подстроки
        memmove(p, p + len, strlen(p + len) + 1); // сдвигаем оставшуюся часть строки на длину подстроки
    }
    str[strlen(str)-1]='\0';
    return str;
}

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char* buffer=(char*)malloc(sizeof(char)* BUFFER_SIZE);
    FILE* file;
    char* file_name=(char*)malloc(sizeof(char)* BUFFER_SIZE);

    // Создание сокета
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Настройка адреса сервера
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Укажите IP-адрес сервера
    server_address.sin_port = htons(PORT);  // Укажите порт сервера

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        return 1;
    }

    char option[BUFFER_SIZE];

    while(1){
        printf("ECHO-отправить запрос\nINFO-получить информацию о сервере\nCD-сменить каталог сервера\nLIST-список объектов из текущего каталога\nQUIT-выход из программы\n>");
        fgets(option, sizeof(option), stdin);

        if(strstr(option, "@")){
            file_name=removeSubstring(option, "@");
            file=fopen(file_name, "r");
            if (file == NULL) { // проверяем, удалось ли открыть файл
                printf("Ошибка при открытии файла\n");
                continue;
            }
            while (fgets(option, sizeof(option), file)) { // читаем строки из файла
                send(client_socket, option , BUFFER_SIZE, 0);
                recv(client_socket, buffer, BUFFER_SIZE,0);
                printf("%s\n", buffer);
                sleep(1);
            }

            fclose(file); // закрываем файл
        }
        else if(strstr(option, "QUIT")){
            send(client_socket, option , BUFFER_SIZE, 0);
            recv(client_socket, buffer, BUFFER_SIZE,0);
            printf("%s\n", buffer);
            break;
        }
        else{
            send(client_socket, option , BUFFER_SIZE, 0);
            recv(client_socket, buffer, BUFFER_SIZE,0);
            printf("%s\n", buffer);
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
    if(buffer)
        free(buffer);
    close(client_socket);
}
