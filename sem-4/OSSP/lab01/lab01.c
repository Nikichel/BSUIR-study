#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct treeNode{ 	//узел бинарного дерева
	char field[PATH_MAX]; 	//поле
	struct treeNode* left; 	//указатель на левый лист
	struct treeNode* right; //указатель на правый
};

struct Flags{ 		//флаг опций
	bool directory; 	//флаг каталогов
	bool file;		//флаг файлов
	bool link; 		//флаг ссылок
};

struct treeNode* putInTree(char* str, struct treeNode* tree){ 	//добавить узел
	if (tree==NULL) {
		tree=(struct treeNode*)calloc(1,sizeof(struct treeNode));	 //новый узел
		tree->left=tree->right=NULL;
		(void)strncpy(tree->field, str, PATH_MAX);
	}
	else if (strcmp(str, tree->field)<=0) {
		tree->left=putInTree(str,tree->left); 		//переход налево
	}
	else{
		tree->right=putInTree(str,tree->right); 	//переход вправо
	}
	return tree;
}

void treePrint(struct treeNode *tree) { 	//вывод дерева на экран
	if (tree!=NULL) {
		treePrint(tree->left);
		printf("%s\n", tree->field);
		treePrint(tree->right);
	}
}

bool setOption(int argc, char **argv, struct Flags* fl){ 	//установить опцию
	bool flag=false;
	while (1) {
		int val=getopt(argc, argv, "dfl");
		switch (val) {
		case('d'):{ 		//каталоги
			fl->directory=true;
			flag=true;
			break;
		}
		case('f'):{ 		//файлы
			fl->file=true;
			flag=true;
			break;
		}
		case('l'):{ 		//ссылки
			fl->link=true;
			flag=true;
			break;
		}
		default:
		return flag;
		}
	}
}

void dirWalk(char *introducedDir, struct treeNode** tree, struct Flags fl){ 	//функция dirWalker
	DIR *dir = opendir(introducedDir); 	//поток каталога
	char pathName[PATH_MAX + 1];
	if (dir==NULL) { 		//ошибка потока каталога
		printf( "Error opening %s: %s", introducedDir, strerror(errno));
		return ;
	}
	struct stat entryInfo; 		//информация о файле
	struct dirent *dir_entry;
	for (;;) {
		dir_entry = readdir(dir); 	//чтение файла из потока
		if (NULL == dir_entry) { 	//конец потока
			break;
		}
		if((strncmp(dir_entry->d_name, ".", PATH_MAX) == 0) || (strncmp(dir_entry->d_name, "..", PATH_MAX) == 0)) { 	//пропуск "." и ".."
			continue;
		}
		(void)strncpy(pathName, introducedDir, PATH_MAX);
		(void)strncat(pathName, "/", PATH_MAX);
		(void)strncat(pathName, dir_entry->d_name, PATH_MAX);
		if(lstat(pathName, &entryInfo)==0){
			char tmpPathName[PATH_MAX + 1];
			if(S_ISDIR(entryInfo.st_mode)) { 		//каталог
				if (fl.directory) {
					(void)strncpy(tmpPathName, "-d\t", PATH_MAX);
					(void)strncat(tmpPathName, pathName, PATH_MAX);
					*tree = putInTree(tmpPathName, *tree);
				}
				dirWalk(pathName,tree,fl); 			//проход в каталог
			}
			else if(S_ISREG(entryInfo.st_mode)) { 		//файл
				if (fl.file){
					(void)strncpy(tmpPathName, "-f\t", PATH_MAX);
					(void)strncat(tmpPathName, pathName, PATH_MAX);
					*tree = putInTree(tmpPathName, *tree);
				}
			}
			else if(S_ISLNK(entryInfo.st_mode)) { 		//ссылка
				if (fl.link){
					(void)strncpy(tmpPathName, "-l\t", PATH_MAX);
					(void)strncat(tmpPathName, pathName, PATH_MAX);
					*tree = putInTree(tmpPathName, *tree);
				}
			}
		}
	}
	return;
}

int main(int argc, char **argv){
	struct treeNode* tree=NULL; 		//бинарное дерево
	struct Flags flag; 			//флаги поиска
	flag.directory=flag.file=flag.link=false;
	char direct[PATH_MAX]; 			//старт
	if(!setOption(argc, argv, &flag))
		flag.directory=flag.file=flag.link=true;
	if (strstr(argv[argc-1],"/")!=NULL && argc>1) {
		strcpy(direct,argv[argc-1]);
	}
	else{
		strcpy(direct,".");
	}

	dirWalk(direct, &tree, flag); 		//обход файлов
	treePrint(tree); 			//вывод дерева на экран
	if (tree){
		free(tree);
	}
	return 0;
} 
