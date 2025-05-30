#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_LINE_LEN 2048

// Функция для проверки, является ли файл текстовым (по расширению)
int is_text_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 1; // Если нет расширения, считаем текстовым
    
    // Список текстовых расширений
    const char *text_exts[] = {".txt", ".c", ".h", ".cpp", ".hpp", ".py", ".sh", ".md", NULL};
    
    for (int i = 0; text_exts[i]; i++) {
        if (strcmp(ext, text_exts[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Функция поиска слова в файле
void search_in_file(const char *filename, const char *search_word) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Ошибка открытия файла");
        return;
    }

    char line[MAX_LINE_LEN];
    int line_num = 1;
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, search_word)) {
            printf("%s:%d: %s", filename, line_num, line);
        }
        line_num++;
    }
    
    fclose(file);
}

// Рекурсивная функция обхода директорий
void search_directory(const char *dirname, const char *search_word) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("Ошибка открытия директории");
        return;
    }

    struct dirent *entry;
    struct stat statbuf;
    char path[MAX_PATH_LEN];

    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем текущую и родительскую директории
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Формируем полный путь
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        // Получаем информацию о файле/директории
        if (lstat(path, &statbuf) == -1) {
            perror("Ошибка получения информации о файле");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Рекурсивный обход поддиректорий
            search_directory(path, search_word);
        } else if (S_ISREG(statbuf.st_mode) && is_text_file(entry->d_name)) {
            // Поиск в текстовых файлах
            search_in_file(path, search_word);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Использование: %s [директория] искомое_слово\n", argv[0]);
        fprintf(stderr, "По умолчанию используется директория ~/files\n");
        return 1;
    }

    const char *search_dir;
    const char *search_word;

    if (argc == 2) {
        search_dir = "~/files";
        search_word = argv[1];
    } else {
        search_dir = argv[1];
        search_word = argv[2];
    }

    // Разворачиваем ~ в домашнюю директорию
    if (search_dir[0] == '~') {
        char *home = getenv("HOME");
        char expanded_dir[MAX_PATH_LEN];
        snprintf(expanded_dir, sizeof(expanded_dir), "%s%s", home, search_dir + 1);
        search_dir = expanded_dir;
    }

    printf("Поиск слова '%s' в директории '%s'...\n", search_word, search_dir);
    search_directory(search_dir, search_word);

    return 0;
}
