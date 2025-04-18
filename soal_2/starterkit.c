#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>  
#include <ctype.h>

#define STARTER_KIT_PATH "starter_kit"
#define LOG_PATH "activity.log"

int valid_base64(const char *str) {
    while (*str) {
        if (!(isalnum(*str) || *str == '+' || *str == '/' || *str == '='))
            return 0;
        str++;
    }
    return 1;
}

char *simple_base64_decode(const char *str) {
    char command[512];
    snprintf(command, sizeof(command), "echo %s | base64 -d", str);

    FILE *fp = popen(command, "r");
    if (fp == NULL) return NULL;

    static char result[256];
    fgets(result, sizeof(result), fp);
    pclose(fp);
    result[strcspn(result, "\n")] = '\0';

    return strdup(result);
}

void write_log(const char *msg) {
    FILE *f = fopen(LOG_PATH, "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%02d-%02d-%04d][%02d:%02d:%02d] - %s\n",
        t->tm_mday, t->tm_mon+1, t->tm_year+1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        msg);

    fclose(f);
}

void decrypt_files() {
    DIR *dir = opendir(STARTER_KIT_PATH);
    if (!dir) {
        perror("Gagal membuka starter_kit");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char old_path[512], new_path[512];
        snprintf(old_path, sizeof(old_path), "%s/%s", STARTER_KIT_PATH, entry->d_name);

        char *decoded = simple_base64_decode(entry->d_name);
        if (!decoded) continue;

        snprintf(new_path, sizeof(new_path), "%s/%s", STARTER_KIT_PATH, decoded);
        rename(old_path, new_path);

        char logmsg[512];
        snprintf(logmsg, sizeof(logmsg), "Successfully decrypted: %s", decoded);
        write_log(logmsg);
        free(decoded);
    }

    closedir(dir);
}

void quarantine_files() {
    DIR *dir = opendir(STARTER_KIT_PATH);
    if (!dir) {
        perror("starter_kit tidak ditemukan");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {

        char src_path[512], dst_path[512];
        snprintf(src_path, sizeof(src_path), "%s/%s", STARTER_KIT_PATH, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", "quarantine", entry->d_name);

        if (rename(src_path, dst_path) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully moved to quarantine directory.", entry->d_name);
            write_log(logmsg);
        }
    }
  }
    closedir(dir);
}

void return_files() {
    DIR *dir = opendir("quarantine");
    if (!dir) {
        perror("quarantine tidak ditemukan");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char src_path[512], dst_path[512];
        snprintf(src_path, sizeof(src_path), "quarantine/%s", entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", STARTER_KIT_PATH, entry->d_name);

        if (rename(src_path, dst_path) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully returned to starter kit directory.", entry->d_name);
            write_log(logmsg);
        }
    }

    closedir(dir);
}

void eradicate_files() {
    DIR *dir = opendir("quarantine");
    if (!dir) {
        perror("quarantine tidak ditemukan");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "quarantine/%s", entry->d_name);

        if (remove(filepath) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully deleted.", entry->d_name);
            write_log(logmsg);
        }
    }

    closedir(dir);
}

void shutdown_decrypt() {
    FILE *fp = fopen("decryption.pid", "r");
    if (!fp) {
        printf("PID file tidak ditemukan.\n");
        return;
    }

    int pid;
    fscanf(fp, "%d", &pid);
    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        char logmsg[256];
        snprintf(logmsg, sizeof(logmsg), "Successfully shut off decryption process with PID %d.", pid);
        write_log(logmsg);
        remove("decryption.pid"); // hapus file pid
    } else {
        perror("Gagal mematikan proses");
    }
}


void run_daemon() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) {
        // Simpan PID ke file
        FILE *fp = fopen("decryption.pid", "w");
        if (fp) {
            fprintf(fp, "%d", pid);
            fclose(fp);
        }

        // Log berhasil start
        char logmsg[256];
        snprintf(logmsg, sizeof(logmsg), "Successfully started decryption process with PID %d.", pid);
        write_log(logmsg);
        exit(EXIT_SUCCESS); // parent keluar
    }

    // Child jadi daemon
    umask(0);
    setsid();
    chdir(".");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    while (1) {
        DIR *qdir = opendir("quarantine");
        if (!qdir) {
            sleep(5);
            continue;
        }

        struct dirent *file;
        while ((file = readdir(qdir)) != NULL) {
            if (file->d_type == DT_REG && valid_base64(file->d_name)) {
                char old[512], new[512];
                snprintf(old, sizeof(old), "%s/%s", "quarantine", file->d_name);

                char *decoded = simple_base64_decode(file->d_name);
                if (decoded && strlen(decoded) && !strchr(decoded, '/')) {
                    snprintf(new, sizeof(new), "%s/%s", "quarantine", decoded);
                    if (rename(old, new) == 0) {
                        char logmsg[512];
                        snprintf(logmsg, sizeof(logmsg), "%s - Decrypted to %s.", file->d_name, decoded);
                        write_log(logmsg);
                    }
                }
                free(decoded);
            }
        }

        closedir(qdir);
        sleep(5);
    }


}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage:\n");
        printf("  ./starterkit --decrypt\n");
        printf("  ./starterkit --quarantine\n");
        printf("  ./starterkit --return\n");
        printf("  ./starterkit --eradicate\n");
        printf("  ./starterkit --shutdown\n");
        return 1;
    }

    if (strcmp(argv[1], "--decrypt") == 0) {
        run_daemon();
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        quarantine_files();
    } else if (strcmp(argv[1], "--return") == 0) {
        return_files();
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_files();
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_decrypt();
    } else {
        printf("Opsi tidak dikenali: %s\n", argv[1]);
    }


    return 0;
}
