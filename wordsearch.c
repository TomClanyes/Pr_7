#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 1024

void search_file(const char *path, const char *word) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error opening %s\n", path);
        return;
    }

    char line[4096];
    int line_num = 1;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, word)) {
            printf("%s:%d: %s", path, line_num, line);
        }
        line_num++;
    }
    fclose(file);
}

void search_dir(const char *dir, const char *word) {
    DIR *dp = opendir(dir);
    if (!dp) {
        fprintf(stderr, "Error opening %s\n", dir);
        return;
    }

    struct dirent *entry;
    struct stat st;
    char path[MAX_PATH];

    while ((entry = readdir(dp))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if (lstat(path, &st) == -1) {
            fprintf(stderr, "Error accessing %s\n", path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            search_dir(path, word);
        } else if (S_ISREG(st.st_mode)) {
            search_file(path, word);
        }
    }
    closedir(dp);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s [directory] search_word\n", argv[0]);
        return 1;
    }

    char *dir = argc == 2 ? "~/files" : argv[1];
    char *word = argc == 2 ? argv[1] : argv[2];

    if (dir[0] == '~') {
        char *home = getenv("HOME");
        char expanded[MAX_PATH];
        snprintf(expanded, sizeof(expanded), "%s%s", home, dir + 1);
        dir = expanded;
    }

    search_dir(dir, word);
    return 0;
}
