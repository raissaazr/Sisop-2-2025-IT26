#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>

#define LOG_FILE "debugmon.log"
#define PID_FILE_FORMAT "daemon_%s.pid"
#define FAILED_USERS_FILE "failed_users.txt"

void write_log(const char *process_name, const char *status) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(log, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            process_name, status);
    fclose(log);
}

int is_user_failed(const char *username) {
    FILE *fp = fopen(FAILED_USERS_FILE, "r");
    if (!fp) return 0;
    char line[100];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, username) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void add_failed_user(const char *username) {
    FILE *fp = fopen(FAILED_USERS_FILE, "a");
    if (fp) {
        fprintf(fp, "%s\n", username);
        fclose(fp);
    }
}

void remove_failed_user(const char *username) {
    FILE *fp = fopen(FAILED_USERS_FILE, "r");
    if (!fp) return;

    FILE *temp = fopen("temp.txt", "w");
    char line[100];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, username) != 0) {
            fprintf(temp, "%s\n", line);
        }
    }
    fclose(fp);
    fclose(temp);
    rename("temp.txt", FAILED_USERS_FILE);
}

void list_user(const char *username) {
    char command[256];
    snprintf(command, sizeof(command), "ps -u %s -o pid,comm,%%cpu,%%mem --no-headers", username);
    FILE *fp = popen(command, "r");
    if (!fp) {
        perror("popen");
        return;
    }
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char pid[32], cmd[128], cpu[32], mem[32];
        sscanf(line, "%s %s %s %s", pid, cmd, cpu, mem);
        printf("PID: %s, CMD: %s, CPU: %s%%, MEM: %s%%\n", pid, cmd, cpu, mem);
        write_log(cmd, "RUNNING");
    }
    pclose(fp);
}

void daemon_mode(const char *username) {
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) {
        char pidfile[100];
        snprintf(pidfile, sizeof(pidfile), PID_FILE_FORMAT, username);
        FILE *f = fopen(pidfile, "w");
        if (f) {
            fprintf(f, "%d", pid);
            fclose(f);
        }
        printf("Daemon started for user %s\n", username);
        return;
    }
    setsid();
    while (1) {
        if (is_user_failed(username)) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "pkill -u %s", username);
            system(cmd);
        } else {
            list_user(username);
        }
        sleep(5);
    }
}

void stop_daemon(const char *username) {
    char pidfile[100];
    snprintf(pidfile, sizeof(pidfile), PID_FILE_FORMAT, username);
    FILE *f = fopen(pidfile, "r");
    if (!f) {
        printf("No daemon running for user %s\n", username);
        return;
    }
    int pid;
    fscanf(f, "%d", &pid);
    fclose(f);
    kill(pid, SIGTERM);
    remove(pidfile);
    write_log("stop", "RUNNING");
    printf("Daemon stopped for user %s\n", username);
}

void fail_user(const char *username) {
    add_failed_user(username);
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ps -u %s -o comm=", username);
    FILE *fp = popen(cmd, "r");
    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        write_log(line, "FAILED");
    }
    pclose(fp);
    snprintf(cmd, sizeof(cmd), "pkill -u %s", username);
    system(cmd);
    printf("All processes killed for user %s\n", username);
}

void revert_user(const char *username) {
    remove_failed_user(username);
    write_log("revert", "RUNNING");
    printf("User %s reverted\n", username);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <command> <user>\n", argv[0]);
        return 1;
    }

    const char *cmd = argv[1];
    const char *user = argv[2];

    if (strcmp(cmd, "list") == 0) {
        list_user(user);
    } else if (strcmp(cmd, "daemon") == 0) {
        daemon_mode(user);
    } else if (strcmp(cmd, "stop") == 0) {
        stop_daemon(user);
    } else if (strcmp(cmd, "fail") == 0) {
        fail_user(user);
    } else if (strcmp(cmd, "revert") == 0) {
        revert_user(user);
    } else {
        printf("Unknown command: %s\n", cmd);
    }

    return 0;
}
