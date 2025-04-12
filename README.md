# Soal Shift Sisop Modul 2
Anggota Kelompok:<br />
Azaria Raissa Maulidinnisa 5027241043<br />
Oscaryavat Viryavan 5027241053<br />
Naufal Ardhana 5027241118<br />

## Soal no 1

## Soal no 2

## Soal no 3

## Soal no 4
```
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
```
Diatas merupakan beberapa library yang berguna dalam peluncuran kode ini.

```
#define LOG_FILE "debugmon.log"
#define PID_FILE_FORMAT "daemon_%s.pid"
#define FAILED_USERS_FILE "failed_users.txt"
```
Selanjutnya ada 3 constants yang berisikan file log, Format nama serta PID saat daemon user, dan daftar user yang gagal dibanned
```
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
```
vodi diatas berfungs untuk menuliskan log status proses RUNNING atau FAILED, selanjutnya program juga akan menulis dengan append mode dan mengambil waktu saat ini di file log nanti.
```
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
```
program akan mengecek apakah ``username`` ada di ``FAILED_USERS_FILE`` dan akan membuka file txt yang berisi user gagal untuk memastikan. di ``char line[100];`` akan mengeloop isi file, dan jika ditemukan username akan mereturn 1.
```
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
```
Program ``void add_failed_user`` berfungsi untuk menambahkan username ke ``FAILED_USERS_FILE`` dan ``void remove_failed_user`` akan menghapus username dari file. ``while (fgets(line, sizeof(line), fp))`` akan membuat temp.txt yang menyalin semua user selain username.
```
void list_user(const char *username) {
    char command[256];
    snprintf(command, sizeof(command), "ps -u %s -o pid,comm,%%cpu,%%mem --no-headers", username);
    FILE *fp = popen(command, "r");
    if (!fp) {
        perror("popen");
        return;
    }
```
ini merupakan daemon pertama yang akan menampilkan list yang berisi command, user, PID, CPU dll. List akan berbentuk memanjang ke samping.
```
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char pid[32], cmd[128], cpu[32], mem[32];
        sscanf(line, "%s %s %s %s", pid, cmd, cpu, mem);
        printf("PID: %s, CMD: %s, CPU: %s%%, MEM: %s%%\n", pid, cmd, cpu, mem);
        write_log(cmd, "RUNNING");
    }
    pclose(fp);
}
```
buffer akan terus mengupdate dan mencimpan process ID, memory, cpu, dan command. Fungsi ini akan menyimpan log status RUNNING dan akan berjalan sampai dihentikan oleh stop.
```
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
```
program ini akan membuat daemon (proses latar belakang) untuk emmantau proses user secara real time. ``fork()`` akan menjalankan proses pemantauan untuk user tertentu dengan argumen `username` . Dalam child process akan dijalankan `setsid()` yang akan membuat session baru, memutus hubungan dengan terminal, dan proses akan benar benar menjadi daemon. Disini user juga akan dicek sebagai `FAILED_USER` atau bukan. Jika bukan user dakan dimasukkan ke log. Loop juga akan mengecek setiap 5 detik apakah user failed atau tidak.
```
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
```
Program diatas akan menerima user sebagai parameter yang akan menghentikan daemon memantau user. `FILE *f = fopen(pidfile, "r");` jika file tidak bisa dibuka, berarti daemon belum berjalan, command akan keluar dari fungsi. Selanjutnya, `FILE *f = fopen(pidfile, "r");` akan mengiirim sinyal ke proses PID dan memberitahu daemon untuk berhenti. Notifikasi akan muncul melalui `printf()`.
```
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
```
Program akan menambahkan user ke file `failed_users.txt`. Daemon akan membaca daftar ini dan otomatis menghentikan proses user yang masuk ke dalam daftar. `char cmd[256];` akan mencatat command program yang sedang berjalan oleh user. `line[strcspn(line, "\n")] = 0;` menulis ke file log jika user sudah `FAILED`. Selanjutnya program akan eksekusi semua proses milik user tersebut melalui `"pkill -u %s", username`. Dan akan menampilkan pesan notifikasi melalui `printf()`.
```
void revert_user(const char *username) {
    remove_failed_user(username);
    write_log("revert", "RUNNING");
    printf("User %s reverted\n", username);
}
```
Selanjutnya ada opsi revert yang akan mengembalikan user agar bisa running kembali. `void revert_user(const char *username)` akan memanggil fungsi untuk menghapus username dari file `failed_users.txt`. dengan membaca file, menghapus semua user kecuali user yang ingin direvert ke file sementara `temp.txt`. File sementara akan direname sebagai `failed_users.txt`. Aksi akan dicatat ke dalam log memlalui `1write_log("revert", "RUNNING");`.
```
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
```
perintah argumen akan dicek melalui `int main(int argc, char *argv[]) {` yang nantinya memastikan sesuai dengan instruksi soal, misal `./debugmon daemon aldo` masing masing dari `else if` memberikan case yang berbeda bergantung dengan command yang diberikan mulai dari list, daemon, stop, fail dan revert. `} else { printf("Unknown command: %s\n", cmd); }` akan memberikan jika command tidak sesuai dengan instruksi akan muncul pesan error.

### Output debugmon list
![image](https://github.com/user-attachments/assets/3066f500-bdcc-4078-8cf8-6ebb973a739d)

### Output debugmon stop
![image](https://github.com/user-attachments/assets/197d68d8-21fd-4b53-a222-cc2cb9c4f94f)

### Output debugmon fail 
![image](https://github.com/user-attachments/assets/7adf998a-9e23-4d96-b05d-5831fb7c0a8a)

### Output debugmon revert
![image](https://github.com/user-attachments/assets/cdb289b4-bf43-4e94-85ba-7c9ec1d1ed5d)

### Cat Debugmon.log
![image](https://github.com/user-attachments/assets/b4c57eff-ad02-4f4c-a4f9-678829434780)






