#include "utils.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

void district_exists(const char* district){
    mkdir(district, 0750);
}

void log_action(const char* district, const char* user, const char* role, const char* action){
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/logged_district", district);
    int f = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(f != -1){
        time_t now = time(NULL);
        char entry[512];
        int len = snprintf(entry, sizeof(entry), "%ld\n%s\n%s %s\n", now, user, role, action);
        write(f, entry, len);
        close(f);
    }
}

void mode_to_string(mode_t mode, char* str){
    str[0] = (mode & S_IRUSR) ? 'r' : '-';
    str[1] = (mode & S_IWUSR) ? 'w' : '-';
    str[2] = (mode & S_IXUSR) ? 'x' : '-';
    str[3] = (mode & S_IRGRP) ? 'r' : '-';
    str[4] = (mode & S_IWGRP) ? 'w' : '-';
    str[5] = (mode & S_IXGRP) ? 'x' : '-';
    str[6] = (mode & S_IROTH) ? 'r' : '-';
    str[7] = (mode & S_IWOTH) ? 'w' : '-';
    str[8] = (mode & S_IXOTH) ? 'x' : '-';
    str[9] = '\0';
}
