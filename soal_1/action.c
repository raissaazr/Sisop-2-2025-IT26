#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <ctype.h>

int folder_exists(const char *folder) {
    struct stat st;
    return (stat(folder, &st) == 0 && S_ISDIR(st.st_mode));
}

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

int cmpstr(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

//combine
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

    qsort(number_files, num_count, sizeof(char *), cmpstr);
    qsort(letter_files, let_count, sizeof(char *), cmpstr);
    
    FILE *out = fopen("Combined.txt", "w");
    if(!out) {
        perror("Failed to create Combined.txt");
        return;
    }

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

//decode
char rot13_char(char c) {
    if('a' <= c && c <= 'z') return 'a' + (c - 'a' + 13) % 26;
    else if('A' <= c && c <= 'Z') return 'A' + (c - 'A' + 13) % 26;
    else return c;
}

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

    char ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(rot13_char(ch), out);
    }

    fclose(in);
    fclose(out);
}

//check password
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

void print_usage() {
    printf("Usage:\n");
    printf("  ./action             -> Download and extract Clues.zip (if not exists)\n");
    printf("  ./action -m Filter   -> Filter clue files into 'Filtered/'\n");
    printf("  ./action -m Combine  -> Combine filtered file contents\n");
    printf("  ./action -m Decode   -> Decode Combined.txt using Rot13\n");
    printf("  ./action -m Check    -> Input password and verify with Decoded.txt\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        if (!folder_exists("Clues")) {
            char *wget_args[] = {"wget", "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download", "-O", "Clues.zip", NULL};
            run_command(wget_args);
            
            char *unzip_args[] = {"unzip", "Clues.zip", NULL};
            run_command(unzip_args);

            char *rm_args[] = {"rm", "Clues.zip", NULL};
            run_command(rm_args);
        } else {
            printf("Folder already exists.\n");
        }  
    }

    // filter
    if(argc == 3 && strcmp(argv[1], "-m") == 0 && strcmp(argv[2], "Filter") == 0) {
        char *filtered_dir = "Filtered";
        mkdir(filtered_dir, 0755);

        for(char c = 'A'; c <= 'D'; c++) {
            char folder_path[PATH_MAX];
            snprintf(folder_path, sizeof(folder_path), "Clues/Clue%c", c);
            DIR *dir = opendir(folder_path);
            if (!dir) continue;

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] == '.') continue;

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

    //combine
    if(argc == 3 && strcmp(argv[1], "-m") == 0 && strcmp(argv[2], "Combine") == 0) {
        combine_files("Filtered");
        return 0;
    }

//decoded
    if(argc == 3 && strcmp(argv[1], "-m") == 0 && strcmp(argv[2], "Decode") == 0) {
        decode_rot13("Combined.txt", "Decoded.txt");
        return 0;
    }

 //check password
    if(argc == 3 && strcmp(argv[1], "-m") == 0 && strcmp(argv[2], "Check") == 0) {
        check_pass("Decoded.txt");
        return 0;
    }
    print_usage();
    return 1;
}
