#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#define DEBUG_MODE

int create_tar(const char* _PATH);
int add_to_tar(const char* _FILE);

#ifdef DEBUG_MODE
int main(int argc, const char* argv[]) {
    create_tar(argv[2]);
    add_to_tar("aaa");
    close(0);
    return 0;
}
#endif

#ifndef DEBUG_MODE
int main(int argc, char* argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Wrong usage. Usage: ./compress <folder> <tar_filename>\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}
#endif

int create_tar(const char* _PATH) {
    open(_PATH, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if(errno != 0) {
        perror("Exited with error");
    }
    return(0);
}

int add_to_tar(const char* _FILE) {
    struct stat buf;
    mode_t modes;
    open(_FILE, O_RDONLY);
    stat(_FILE, &buf);
    modes = buf.st_mode;
    if(S_ISDIR(modes)) {
        opendir()
    }
    return(0);
}

