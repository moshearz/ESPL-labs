#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> // ssize_t is a type which defined under: <unistd.h>
#include <elf.h> 


#define MAX_ELF_FILES 2


// struct represents a ELF File 
typedef struct { // define typedef (and not struct) so i be able to use it without the "struct" keyword
    char *fileName; // Name of ELF file
    int fd;         // File descriptor for ELF file
    size_t file_size;   // Size of ELF file
    void *map_start;    // Pointer to the data adress of mapped ELF file start (mmap)
} ELFFile;


// "limited to merging 2 ELF files"
ELFFile *elfFiles[MAX_ELF_FILES]; // Array to hold pointers to ELFFile structs
int elfFileCounter = 0; // Counter for the number of ELF files
int debug_mode = 0; // Debug flag

// Funcs Declarations
void toggle_debug_mode();
void examineELFFile();
void printSectionNames(ELFFile *elfFile);
void printSymbolTable(ELFFile *elfFile);
void checkFilesForMerge(ELFFile *elfFile1, ELFFile *elfFile2);
void mergeELFFiles(ELFFile *elfFile1, ELFFile *elfFile2);
void quit();


// struct - lab4
typedef struct {
    char *name;
    void (*func)();
} MenuOption;

MenuOption menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Examine Elf File", examineELFFile},
    {"Print Section Names", printSectionNames},
    {"Print Symbols",printSymbolTable }, //1a
    {"Check Files For merge", checkFilesForMerge}, //1b
    {"Merge ELF Files", mergeELFFiles}, //1c
    {"Quit", quit}, //1d
    {NULL, NULL}
};

void toggle_debug_mode() {
    debug_mode = !debug_mode; // Toggle the debug mode
    if (debug_mode) {
        printf("Debug mode is ON\n");
    } else {
        printf("Debug mode is OFF\n");
    }
}

void examineELFFile(){
    if (elfFileCounter >= MAX_ELF_FILES){
        printf("Cannot examine more than %d files.\n", MAX_ELF_FILES);
        return;
    }

    char filename[256];
    printf("Enter Elf file name: ");
    
}

void print_section_names(){
    printf("Section names are not implemented yet.\n");
}
void print_symbol_table(){
    printf("Symbol table is not implemented yet.\n");
}

void checkFilesForMerge(ELFFile *elfFile1, ELFFile *elfFile2){
    printf("Check files for merge is not implemented yet.\n");
}

void mergeELFFiles(ELFFile *elfFile1, ELFFile *elfFile2){
    printf("Merge ELF files is not implemented yet.\n");
}

void quit(){
    for(int i = 0; i < elfFileCounter; i++)
        cleanup_file(elfFiles[i]);
    printf("Exiting program.\n");
    exit(0);
}


int main(){
    while (1) {
        printf("Choose action:\n");
        for(int i = 0; menu[i].name != NULL; i++)
            printf("%d) %s\n", i, menu[i].name);
        printf("Please Choise: ");

        char input[32];
        int choise = atoi(input);
        int menu_size = sizeof(menu) / sizeof(menu[0]);
        if (!fgets(input, sizeof(input), stdin)) break;
        int choice = atoi(input);
        if (choice >= 0 && choice < menu_size) 
            menu[choice].func();
        else
            printf("Invalid option\n");
    return 0;
    }
}

