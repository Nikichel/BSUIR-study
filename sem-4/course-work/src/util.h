#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

struct modul{           //пакет ядра
   char name[256];      //название пакета
   char size[256];      //размер пакет
   char used_by[256];   //пакеты которые используются данный
};

struct cpu_info{            //инфморация ядра
    char cpu_product[128];      //продукт
    char cpu_vendor[128];       //производитель
    char cpu_family[128];       //семья процессора
    char cpu_cores[128];        //количество ядер
    char cpu_microcode[128];    //микрокод процуессора
    char cpu_cache[128];        //кеш
    char cpu_bogomips[128];     //скорость исполнения
    char cpu_addres_size[128];  //размеры адресов
};

void get_proc_info();   //информация о процессах
void cpu_temp();        //температура процессора
void get_system_info(); //информация о системе
void get_cpu_info();    //информация о процессоре
void get_modules_info();//установленные пакеты я ядре
void kernel_info();     //информация о ядре
void get_usb_info();    //информация о подключенныхх устройствах
void get_loop_info();   //информация о дисках
void get_information(); //информация об утилите

void activate(GtkApplication *app, gpointer user_data);     //основное оконное приложение
