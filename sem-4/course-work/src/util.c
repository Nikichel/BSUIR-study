#include "util.h"

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    // инициализация приложения
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Запуск приложениz
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

int fill_cpu(struct cpu_info *cpu){     //получить информацио о процессоре
    char buffer[1024];
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("Не удалось открыть файл /proc/cpuinfo\n");
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {     //модель
        if (strncmp(buffer, "model name", 10) == 0) {
            strcpy(cpu->cpu_product, buffer);
        }
        else if (strncmp(buffer, "vendor_id", 9) == 0) {    //происзводитель
            strcpy(cpu->cpu_vendor, buffer);
        }
        else if (strncmp(buffer, "cpu cores", 9) == 0) {    //кол-во ядер
            strcpy(cpu->cpu_cores, buffer);
        }
        else if (strncmp(buffer, "cpu family", 9) == 0) {   //семья процессора
            strcpy(cpu->cpu_family, buffer);
        }
        else if (strncmp(buffer, "microcode", 9) == 0) {    //микрокод
            strcpy(cpu->cpu_microcode, buffer);
        }
        else if (strncmp(buffer, "cache size", 10) == 0) {  //размер кеша
            strcpy(cpu->cpu_cache, buffer);
        }
        else if (strncmp(buffer, "bogomips", 8) == 0) {     //скорость исполнения
            strcpy(cpu->cpu_bogomips, buffer);
        }
        else if (strncmp(buffer, "address sizes", 13) == 0) {   //размер адресов
            strcpy(cpu->cpu_addres_size, buffer);
        }
        else if(strncmp(buffer, "\n", 1) == 0)
            break;
    }

    fclose(fp);
    return 0;
}

void get_proc_info()    //информация о процессах
{
    printf("\n\n");
    FILE *fp;
    char path[1035];
    fp = popen("ps -A", "r");   //запуск процесса, который выводит список процессов
    if (fp == NULL) {
        perror("Ошибка команды \"ps -A\"\n" );
        return;
    }
    printf("PID\tCOMMAND\n");
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        printf("%s", path);     //вывод процессов
    }
    pclose(fp);
}

void cpu_temp(){    //температура процессора
    FILE* temp_file = fopen("/sys/class/thermal/thermal_zone0/temp", "r");      //открытие файла с тепературой

    if (temp_file == NULL) {
        perror("Не удалось открыть \"/sys/class/thermal/thermal_zone0/temp\"\n");
        return;
    }

    int temp_raw;
    fscanf(temp_file, "%d", &temp_raw);
    fclose(temp_file);

    float temp_c = (float) temp_raw / 1000;    //перевод в цельсии

    printf("CPU temperature: %.1f C\n", temp_c);    //вывод результата
}

void get_system_info()      //информация о системе
{
    printf("\n\n");
    //Получаем информацию о системе
    struct utsname uname_data;
    if (uname(&uname_data) == -1) {
        perror("uname");
        return;
    }
    struct sysinfo sysinfo_data;
    if (sysinfo(&sysinfo_data) == -1) {
        perror("sysinfo");
        return;
    }

    char buffer[1024];
    FILE* fp;
    fp = popen("cat /proc/cpuinfo | grep 'model name' | uniq", "r");
    fgets(buffer, sizeof(buffer), fp);
    pclose(fp);

    // Выводим информацию на экран
    printf("Operating system: %s %s\n", uname_data.sysname, uname_data.release);
    printf("CPU %s", buffer);
    cpu_temp();
    printf("Machine type: %s\n", uname_data.machine);
    printf("User ID: %d\n", getuid());
    printf("Group ID: %d\n", getgid());
    printf("Effective user ID: %d\n", geteuid());
    printf("Effective group ID: %d\n", getegid());
    printf("Number of processors: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    printf("Total RAM: %ld MB\n", sysinfo_data.totalram / 1024 / 1024);
    printf("Free RAM: %ld MB\n", sysinfo_data.freeram / 1024 / 1024);
}

void get_cpu_info(){        //информация о процессоре
    printf("\n\n");
    struct cpu_info cpu;
    if(fill_cpu(&cpu))
        return;

    printf("%s", cpu.cpu_product);
    printf("%s", cpu.cpu_vendor);
    printf("%s", cpu.cpu_family);
    printf("%s", cpu.cpu_cores);
    printf("%s", cpu.cpu_microcode);
    printf("%s", cpu.cpu_cache);
    printf("%s", cpu.cpu_bogomips);
    printf("%s", cpu.cpu_addres_size);
}

void get_modules_info(){        //информация об установленных пакетах
    FILE *fp;
    char line[256];
    struct modul ker_modul;
    // Открываем файл со списком загруженных модулей
    fp = fopen("/proc/modules", "r");
    if (fp == NULL) {
        perror("Не удалось открыть /proc/modules");
        return;
    }

    printf("Загруженные модули:\n");
    // Считываем построчно файл и выводим информацию о модулях
    while (fgets(line, sizeof(line), fp)) {
        // Разбиваем строку на токены с помощью функции sscanf
        sscanf(line, "%s %s %*s %s", ker_modul.name, ker_modul.size, ker_modul.used_by);
        // Выводим отформатированную информацию о модуле
        printf("%-30s %10s %10s\n", ker_modul.name, ker_modul.size, ker_modul.used_by);
    }
    fclose(fp);
}

void kernel_info(){     //информация о ядре
    printf("\n\n");
    struct utsname buffer;
    if (uname(&buffer) != 0) {      //заполняем структуру
        perror("Не удалось получить информацию\n");
        return;
    }
    printf("Operating System: %s\n", buffer.sysname);   //вывод информации
    printf("Node Name: %s\n", buffer.nodename);
    printf("Kernel Release: %s\n", buffer.release);
    printf("Kernel Version: %s\n", buffer.version);
    printf("Machine Hardware Name: %s\n", buffer.machine);
}

void get_usb_info(){        //информация о подключенных устройствах
    printf("\n\n");
    system("lsusb");        //вызов системной утилиты lsusb

}

void get_loop_info(){       //информация о жестких дисках
    printf("\n\n");
    system("lsblk");        //вызов системной утилиты lsblk
}

void get_information(){     //вывод информации об утилите
    printf("\n\nРазработчик: Гаращук Никита Васильевич\nСтудент группы 150501.\n");
}

// Функция для создания главного окна приложения
void activate(GtkApplication *app, gpointer user_data)
{
    // Объявление переменных для виджетов окна
    GtkWidget *window;
    GtkWidget *button_proc;
    GtkWidget *button_cpu;
    GtkWidget *button_system;
    GtkWidget *button_usb;
    GtkWidget *button_loop;
    GtkWidget *button_info;
    GtkWidget *modules_info;
    GtkWidget *button_kernel;
    GtkWidget *vbox;

    // Создание главного окна и задание его параметров
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Nigava (Сбор информации)");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Создание вертикального контейнера для размещения виджетов
    vbox = gtk_box_new(TRUE, 8);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Создание кнопок и подключение к ним функций обработки событий
    button_proc = gtk_button_new_with_label("Информация о работающих процессах");
    button_system = gtk_button_new_with_label("Общая информация о системе");
    button_cpu = gtk_button_new_with_label("Информация о процессоре");
    button_usb = gtk_button_new_with_label("Информация о подключенных устройствах");
    button_loop = gtk_button_new_with_label("Информация о жестких дисках");
    button_info = gtk_button_new_with_label("Информация об утилите");
    button_kernel = gtk_button_new_with_label("Информация об ядре");
    modules_info = gtk_button_new_with_label("Информация о модулях установленных в ядро");

    g_signal_connect(button_proc, "clicked", G_CALLBACK(get_proc_info), "proc_button");
    g_signal_connect(button_cpu, "clicked", G_CALLBACK(get_cpu_info), "cpu_button");
    g_signal_connect(button_system, "clicked", G_CALLBACK(get_system_info), "system_button");
    g_signal_connect(button_usb, "clicked", G_CALLBACK(get_usb_info), "usb_button");
    g_signal_connect(button_loop, "clicked", G_CALLBACK(get_loop_info), "loop_button");
    g_signal_connect(button_info, "clicked", G_CALLBACK(get_information), "info_button");
    g_signal_connect(button_kernel, "clicked", G_CALLBACK(kernel_info), "kernel_button");
    g_signal_connect(modules_info, "clicked", G_CALLBACK(get_modules_info), "modules_button");

    // Размещение кнопок в контейнере
    gtk_box_pack_start(GTK_BOX(vbox), button_proc, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_cpu, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_system, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_usb, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_loop, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_kernel, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), modules_info, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_info, TRUE, TRUE, 5);

    // Отображение всех виджетов на экране
    gtk_widget_show_all(window);
}
