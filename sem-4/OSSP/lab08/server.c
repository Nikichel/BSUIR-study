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
#include <pthread.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define MAX_THREAD 5
#define PORT 8080

int count_thread = 1;
char path_to_server[BUFFER_SIZE];
char info_server[] = {"==============================СЕРВЕР NIKITA=============================="};
bool server_free = false;

struct server_args{
    int socket;
    int adrlen;
};

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

char *foo(char *first_str, char *second_str) {
     char *result = malloc(BUFFER_SIZE);
     int i = 0;
     int j = 0;
     int k = 0;

     while (first_str[j]) {
          result[i] = first_str[j];
          i++;
          j++;
     }

     while (second_str[k]) {
          result[i] = second_str[k];
          i++;
          k++;
     }

     result[i] = '\0';

     return result;
}

char* foo_LIST(const char* current_dir) {
    int element_count = 0;
    DIR* directory;
    struct dirent* de;
    char *file_names = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    strcpy(file_names, "Current working directory: ");
    strcat(file_names, path_to_server);
    strcat(file_names, "\n");
    directory = opendir(current_dir); //открытие каталога
    if (!directory){
        return file_names;
    }

    while ((de = readdir(directory)) != NULL) {

        if (!strcmp(".", de->d_name) || !strcmp("..", de->d_name)){
            continue;
        }

        char name[BUFFER_SIZE];
        element_count++;

        memcpy(name, (char*)de->d_name, BUFFER_SIZE);

        if (de->d_type == 4) {
            strcat(name, "/");
        } else if (de->d_type == 10) {
            char link_path[BUFFER_SIZE];
            struct stat sb;

            ssize_t len = readlink(foo((char*)current_dir, name), link_path, sizeof(link_path)-1);
            if(len ==  -1){
                continue;
            }
            link_path[len] = '\0';

            lstat(link_path, &sb);
            if (S_ISLNK(sb.st_mode)) {
                strcat(name, "-->>");
            } else {
                strcat(name, "-->");
            }

            strcat(name, link_path);

            printf("Link: %s\n", name);
        }

        strcat(name, "\n");

        file_names = (char*)realloc(file_names, sizeof(char) * element_count * BUFFER_SIZE);

        strcat(file_names, name);

    }

    return file_names; // функция возвращает массив имен элементов
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char* buffer=(char*)malloc(sizeof(char)* BUFFER_SIZE);
    while (1) {
        read(client_socket, buffer, BUFFER_SIZE);
        printf("Get from client: %s\n", buffer);
        if(strstr(buffer, "ECHO")){
            buffer = removeSubstring(buffer, "ECHO ");
            printf("Received: %s\n", buffer);
            send(client_socket, buffer, BUFFER_SIZE,0);
        }
        else if(strstr(buffer, "LIST")){
            buffer=foo_LIST(path_to_server);
            send(client_socket, buffer, BUFFER_SIZE,0);
        }
        else if(strstr(buffer, "INFO")){
            strcpy(buffer, "Path: ");
            buffer = foo(buffer, path_to_server);
            buffer = foo(buffer, "\n");
            buffer = foo(buffer, info_server);
            send(client_socket, buffer, BUFFER_SIZE,0);
        }
        else if(strstr(buffer, "CD")){
            char tmp_path[BUFFER_SIZE];
            buffer = removeSubstring(buffer, "CD ");
            if (getcwd(tmp_path, BUFFER_SIZE) != NULL) {
                printf("Current working directory: %s\n", tmp_path);
            } else {
                perror("getcwd() error");
                send(client_socket, "Ошибка при изменении каталога", sizeof("Ошибка при изменении каталога"),0);
                continue;
            }
            strcat(tmp_path, "/");
            strcat(tmp_path, buffer);
            if (chdir(tmp_path) != 0) {
                perror("chdir() error");
                send(client_socket, "Ошибка при изменении каталога", sizeof("Ошибка при изменении каталога"),0);
                continue;
            }
            if (getcwd(path_to_server, BUFFER_SIZE) != NULL) {
                printf("New working directory: %s\n", path_to_server);
            } else {
                perror("getcwd() error");
                send(client_socket, "Ошибка при изменении каталога", sizeof("Ошибка при изменении каталога"),0);
                continue;
            }
            strcpy(buffer, "Path: ");
            strcat(buffer, path_to_server);
            send(client_socket, buffer, BUFFER_SIZE,0);
        }
        else if(strstr(buffer, "QUIT")){
            send(client_socket, "BYE BYE", strlen("BYE BYE"),0);
            break;
        }
        else{
            send(client_socket, "invalid option", strlen("invalid option"),0);
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
    close(client_socket);
    free(buffer);
    printf("Client %d ends\n", count_thread-1);
    count_thread--;
    if(!count_thread)
        server_free=true;
    pthread_exit(NULL);
}
void *handle_server(void *arg) {
    struct server_args server_info = *(struct server_args *)arg;
    struct sockaddr_in client_address;
    pthread_t clients[MAX_THREAD];
    int client_socket;
    int adrlen = server_info.adrlen;
    int server_socket = server_info.socket;
    while (!server_free) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        struct timeval pause;

        pause.tv_sec = 1;

        fd_set tmp_fds = read_fds;

        int res = select(server_socket+1, &read_fds, NULL, NULL, &pause);
        if(res == -1){
            perror("Error select\n");
            exit(1);
        }
        else if(res == 0){
            sleep(1);
            continue;
        }

        if(!FD_ISSET(server_socket,&tmp_fds))
            continue;

        client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t*)&adrlen);
        if (client_socket < 0) {
            printf("Ошибка при подключении клиента\n");
            continue;
        }

        if (count_thread >= MAX_THREAD) {
            printf("Превышено максимальное количество клиентов\n");
            close(client_socket);
            continue;
        }

        pthread_create(&clients[count_thread], NULL, handle_client, (void *)&client_socket);
        count_thread++;
        printf("Count clients: %d\n", count_thread-1);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if(argc<2){
        printf("Use: %s <path_to_server>\n", argv[0]);
        exit(-1);
    }
    chdir(argv[1]);
    getcwd(path_to_server, BUFFER_SIZE);
    struct server_args args;
    strcpy(path_to_server, argv[1]);
    struct sockaddr_in server_address, client_address;
    args.adrlen = sizeof(client_address);
    pthread_t server;

    // Создание сокета
    args.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (args.socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    int reuse = 1;
    setsockopt(args.socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Настройка адреса сервера
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);      // Используйте нужный порт

    // Привязка сокета к адресу
    if (bind(args.socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror(strerror(errno));
        return 1;
    }

    // Прослушивание входящих соединений
    if (listen(args.socket, MAX_THREAD) == -1) {
        perror("Listening failed");
        return 1;
    }

    printf("Server started. Waiting for incoming connections...\n");
    printf("%s\n",info_server);
    printf("Максимальное количество подключений: 4\n");
    printf("Для завершения работы сервера нужно ввести \"q\"\n");

    pthread_create(&server, NULL, handle_server, (void *)&args);

    char end;
    while (1){
        end = getchar();
        if(end == 'q')
            break;
    }
    server_free=true;
    printf("Server shutdown\n");
    pthread_join(server, NULL);
    // Закрытие соединения
    close(args.socket);

    return 0;
}
