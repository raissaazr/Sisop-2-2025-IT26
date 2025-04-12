# Soal Shift Sisop Modul 2
Anggota Kelompok:<br />
Azaria Raissa Maulidinnisa 5027241043<br />
Oscaryavat Viryavan 5027241053<br />
Naufal Ardhana 5027241118<br />

## Soal no 1
### 1.1 Mengecek folder ada atau tidak
```
int folder_exists(const char *folder) {
    struct stat st;
    return (stat(folder, &st) == 0 && S_ISDIR(st.st_mode));
}
```
Untuk mengecek ada tidaknya folder dan memastikan bahwa folder merupakan direktori.

### 1.2 Menampilkan usage message 
```
void print_usage() {
    printf("Usage:\n");
    printf("  ./action             -> Download and extract Clues.zip (if not exists)\n");
    printf("  ./action -m Filter   -> Filter clue files into 'Filtered/'\n");
    printf("  ./action -m Combine  -> Combine filtered file contents\n");
    printf("  ./action -m Decode   -> Decode Combined.txt using Rot13\n");
    printf("  ./action -m Check    -> Input password and verify with Decoded.txt\n")
}
```
Memberikan panduan kepada pengguna tentang cara menjalankan program dengan benar.

### 1a. Download the clues
```
void run_command(char *argv[]) {
    pid_t pid = fork();
    if(pid == 0) {
        execvp(argv[0], argv);
        perror("exec failed.");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork failed.");
        exit(EXIT_FAILURE);
    }
}
```
Fungsi untuk menjalankan command yang diberikan dalam bentuk array ```argv[]```  menggunakan child process dan ```execvp()```. ```fork()``` membuat proses baru/child process. ```pid``` menyimpan return value. ```execvp(argv[0], argv)``` menggantikan proses anak dengan anak perintah yang ingin dijalankan. Jika proses gagal, akan muncul message error dan keluar dari proses dengan status gagal. ```else if(pid > 0) { wait(NULL)``` merupakan parent process yang akan dijalankan setelah menunggu child process selesai. ```else { perror(“fork failed.”); exit``` proses fork gagal dan keluar dari program. 

```
int main(int argc, char *argv[]) {
    if (argc == 1) {
        if (!folder_exists("Clues")) {
            char *wget_args[] = {"wget", "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download", "-O", "Clues.zip", NULL};
            run_command(wget_args);
```
Fungsi ```run_command``` yang dipanggil di dalam fungsi main. ```if (argc == 1)``` Mengecek apakah program dijalankan tanpa argument tambahan, ```if (!folder_exists(“Clues”)``` Memeriksa apakah folder Clues ada atau tidak, jika tidak maka lanjut ke proses wget untuk mendownload Clues.zip.

### Proses downloading clues
![shift2_output download](https://github.com/user-attachments/assets/03fdc14c-4a29-4c45-9c5b-7dfc49f86b3a)

### Struktur directory setelah download
![shift2_hasil tree](https://github.com/user-attachments/assets/32528273-688c-49df-86f2-dc879b214a4d)

### 1b. Filter files
```
if(argc == 3 && strcmp(argv[1], "-m") == 0 && strcmp(argv[2], "Filter") == 0) {
        char *filtered_dir = "Filtered";
        mkdir(filtered_dir, 0755);
```
Memeriksa apakah argument yang diberikan sesuai, jika kondisi terpenuhi maka program akan mendeklarasi dan menginisialisasi variable ```filtered.dir``` dengan nama folder yang akan dibuat, yaitu ```Filtered```.

```
for(char c = 'A'; c <= 'D'; c++) {
            char folder_path[PATH_MAX];
            snprintf(folder_path, sizeof(folder_path), "Clues/Clue%c", c);
            DIR *dir = opendir(folder_path);
            if (!dir) continue;
```
Looping untuk memeriksa karakter ‘A’, ‘B’, ‘C’, ‘D’. Menginisialisasi variable ```c``` dengan karakter ```’A’```. Loop akan berlanjut selama nilai ```c````  kurang atau sama dengan ```‘D’```, dengan iterasi setiap nilai c bertambah.
```char folder_path[PATH_MAX]``` mendeklarasikan array yang bertipe char dengan ukuran PATH_MAX.
``` snprintf(folder_path, sizeof(folder_path), "Clues/Clue%c", c);```
Menulis string ke dalam buffer ```folder_path``` dengan format string yang digunakan untuk membuat nama folder. ```DIR *dir = opendir(folder_path)``` fungsi untuk membuka direktori yang sudah ditentukan dan mengembalikan pointer ke tipe DIR. ```if (!dir) continue``` memeriksa apakah pointer bernilai dir atau NULL, jika direktori tidak ada tau gagal, maka program akan melanjutkan ke iterasi berikutnya. 

```
struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] == '.') continue;
```
Mendeklarasikan pointer ke entri direktori. Loop selama masih ada file didalam dir yang diketahui dengan ```readdir``` dan melewati entri tersembunyi.
```
if (strlen(entry->d_name) == 5 && 
                    isalnum(entry->d_name[0]) &&
                    strcmp(entry->d_name + 1, ".txt") == 0) {

                        char src[PATH_MAX], dest[PATH_MAX];
                        snprintf(src, PATH_MAX, "%s/%s", folder_path, entry->d_name);
                        snprintf(dest, PATH_MAX, "%s/%s", filtered_dir, entry->d_name);
                        rename(src, dest);
                    } else {
                        char path[PATH_MAX];
                        snprintf(path, PATH_MAX, "%s/%s", folder_path, entry->d_name);
                        remove(path);
                    }
            }
            closedir(dir);
        }
        return 0;
    }
```
Memeriksa apakah nama file memiliki panjang 5 karakter, apakah karakter pertama huruf atau angka menggunakan ```isalnum()``` dan empat karakter terakhir adalah ```.txt```.  Jika syarat terpenuhi, file akan dipindahkan dan ke folder ```Filtered```. Jika tidak memenuhi syarat, file akan dihapus. Setelah seluruh isi folder dibaca dan difilter, folder akan ditutup dan proses selesai. Tujuan utama dari proses ini adalah menyisakan file petunjuk yang valid di dalam folder ```Filtered```.

### Directory setelah filter
![shift2_filtered](https://github.com/user-attachments/assets/80504e4b-3a82-41ff-abfd-eab62ff335f3)

### 1c. Combine file content
```
int cmpstr(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}
```
Fungsi ```cmpstr``` merupakan fungsi pembanding yang digunakan untuk mengurutkan array string dengan ```qsort```. Fungsi ini membandingkan dua pointer string dengan ```strcmp```, lalu mengembalikan hasil perbandingannya. 

```
void combine_files(const char *dir) {
    DIR *folder = opendir(dir);
    if(!folder) {
        perror("Failed to open Filtered folder");
        return;
    }

    struct dirent *entry;
    char *number_files[500];
    char *letter_files[500];
    int num_count = 0, let_count = 0;

    while((entry = readdir(folder)) != NULL) {
        if(entry->d_name[0] == '.') continue;

        if(isdigit(entry->d_name[0])) {
            number_files[num_count++] = strdup(entry->d_name);
        } else if (isalpha(entry->d_name[0])) {
            letter_files[let_count++] = strdup(entry->d_name);
        }
    }
    closedir(folder);

```
Fungsi combine akan membuka folder ```Filtered``` kemudian membaca semua file didalamnya dan memisahkannya menjadi dua kelompok, yang diawali angka dan diawali huruf. Deklarasi array ```number_files``` dan ```letter_files``` untuk menyimpan file secara terpisah, kemudian jumlah masing-masing akan disimpan dalam ```num_count``` dan ```let_count```. Dalam loop while, setiap entri akan diperiksa. Program akan melewati setiap file yang tersembunyi. Setelah selesai, direktori akan ditutup. 

```
 qsort(number_files, num_count, sizeof(char *), cmpstr);
    qsort(letter_files, let_count, sizeof(char *), cmpstr);

    FILE *out = fopen("Combined.txt", "w");
    if(!out) {
        perror("Failed to create Combined.txt");
        return;
    }
```
Fungsi ```qsort()``` untuk mengurutkan kedua array secara alfabetis menggunakan fungsi pembanding. Setelahnya file akan digabungkan dengan urutan yang benar. File output ```Combined.txt``` dibuka dengan mode ```w```, yang berarti akan membuat file baru. Jika gagal membuka, maka ```perror()``` akan menampilkan pesan eror dan menghentikan program.
```
int i = 0, j = 0;
    while(i < num_count || j < let_count) {
        if (i < num_count) {
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", dir, number_files[i]);
            FILE *f = fopen(path, "r");
            if(f) {
                char ch;
                while ((ch = fgetc(f)) != EOF)
                fputc(ch, out);
                fclose(f);
                remove(path);
            }
            i++;
        }
        if (j < let_count) {
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", dir, letter_files[j]);
            FILE *f = fopen(path, "r");
            if (f) {
                char ch;
                while((ch = fgetc(f)) != EOF)
                fputc(ch, out);
                fclose(f);
                remove(path);
            }
            j++;
        }
    }
    fclose(out);
    for(int k =0; k < num_count; k++) free(number_files[k]);
    for(int k =0; k < let_count; k++) free(letter_files[k]);
}

```
Indeks ```i``` dan ```j``` digunakan untuk melacak posisi dalam array. Program akan terus berlanjut selama masih ada file yang tersisa. Pada setiap iterasi, jika masih ada file angka ```I < num_count```, maka nama file akan digabungkan dengan path folder, kemudian dibuka dan dibaca isinya kemudian ditulis ke dalam ```Combined.txt```. Setelahnya file akan ditutup dan dihapus. Setelah proses selesai, file output ditutup dan semua memori yang dialokasikan dengan ```strdup()``` dibebaskan. Proses ini memastikan penggabungan dilakukan sesuai urutan yang diminta sekaligus membersihkan file sumber setelah isinya dipindahkan.

### Struktur directory setelah combine
![shift2_combine tree](https://github.com/user-attachments/assets/809e375f-bf6a-4a42-89bc-15f2289f5079)

### Hasil combine
![shift2_cat combine](https://github.com/user-attachments/assets/3857ca49-67ff-4dcf-9abd-f82b08988273)

### 1d. Decode dengan rot13
```
char rot13_char(char c) {
    if('a' <= c && c <= 'z') return 'a' + (c - 'a' + 13) % 26;
    else if('A' <= c && c <= 'Z') return 'A' + (c - 'A' + 13) % 26;
    else return c;
}
```
Fungsi ```rot13_char``` digunakan untuk mengenskripsi atau mendeskripsi satu karakter dengan algoritma ROT13, yaitu metode substitusi huruf dengan menggantinya 13 posisi setelahnya dalam alfabet. Jika ```c ``` bukan huruf, maka karakter akan dikembalikan apa adanya tanpa perubahan. 
```
void decode_rot13( const char *input_file, const char *output_file) {
    FILE *in = fopen(input_file, "r");
    if(!in) {
        perror("Failed to open Combined.txt");
        return;
    }

    FILE *out = fopen(output_file, "w");
    if(!out) {
        perror("Failed to create Decoded.txt");
        fclose(in);
        return;
    }

```
Fungsi ```decode_rot13``` akan membuka file teks hasil gabungan ```Combined.txt```, lalu mendeskripsi isinya menggunakan algoritma ROT13 dan menyimpan ke file baru ```Decoded.txt```. Fungsi akan membuka file input ```input_file``` dengan mode baca. jika fungsi gagal membuka file input, maka pesan eror akan ditampilkan. Selanjutnya, fungsi akan membuka file output ```output_file``` dengan mode tulis. Jika gagal, fungsi akan menampilakn pesan dan menutup file input yang sudah terbuka agar tidak terjadi kebocoran memori. Proses deskripsi dilakukan setelah kedua file berhasil dibuka. 
```
char ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(rot13_char(ch), out);
    }

    fclose(in);
    fclose(out);
}
```
Fungsi melakukan proses deskripsi isi file. Variable ```ch``` digunakan untuk membaca setiap karakter dari file input. Setiap karakter yang dibaca kemudian diterjemahkan menggunakan fungsi ```rot13_char```, lalu hasilnya ditulis ke file output. Proses berlangsung hingga seluruh isi file selesai dibaca. 

### Struktur directory setelah decode
![shift2_dr akhir](https://github.com/user-attachments/assets/81440ac9-e0d4-49d5-b867-3fcdbe3f44e4)

### 1e.  Check password
```
void check_pass(const char *decoded_file) {
    FILE *file = fopen(decoded_file, "r");
    if (!file) {
        perror("Failed to open Decoded.txt");
        return; 
    }

    char pass[256];
    fgets(pass, sizeof(pass), file);
    fclose(file);

    pass[strcspn(pass, "\n")] = 0;
```
Fungsi ```check_pass``` akan membaca password dari file ```Decoded.txt```.  Isi baris pertama file dibaca dengan ```fgets``` ke dalam array ```pass```. ```strcspn(pass, “\n”)``` untuk memastikan password tidak memiliki karakter new line di akhir, lalu diganti dengan karakter null sehingga string menjadi bersih. 
```
char input[256];
    printf("Enter password: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    if (strcmp(pass, input) == 0) {
        printf("Password correct!\n");
    } else {
        printf("Password incorrect!\n");
    }
}
```
Selanjutnya fungsi akan memverifikasi password  yang dimasukkan pengguna. Array dideklarasikan untuk menampung password yang dimasukkan. Kemudian fungsi ```fgets``` membaca input dari pengguna  dan ```input[strcspn(input, “\n”)``` untuk menghapus karakter new line. Fungsi ```srtcmp``` akan membandingkan password yang dibaca dari file ```pass``` dengan password yang diinput pengguna. Jika keduanya sama akan muncul pesan ```Password correct!``` dan jika tidak sama akan muncul pesan ```Password incorrect!```. Secara keseluruhan, fungsi digunakan untuk membandingkan password didalam file dengan password yang diinput dan menuliskan pesan hasil perbandingannya.

### Check password
![shift2_decode check](https://github.com/user-attachments/assets/ca958088-7ec9-4cee-8b32-b182e55ad130)

## Soal no 2
### a
sebelum menjalankan program, download file yang sudah diberikan dan unzip lalu menghapus file zi; asli
`wget "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download" -O starter_kit.zip && unzip starter_kit.zip -d starter_kit && rm starter_kit.zip`
<img src="https://github.com/user-attachments/assets/11042f39-f467-4161-b1fb-435b7f4c0ab8" width="400"/>
ini adalah bentuk strukturnya setelah menjalankan command tersebut

### b
Membuat direktori karantina yang dapat mendecrypt nama file.
`mkdir quarantine`
file didecrypt dengan algoritma Base64
```
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
```
 
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






