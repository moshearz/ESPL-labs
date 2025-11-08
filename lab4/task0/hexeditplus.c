#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// (Given) global variables
char debug_mode;
char file_name[128];
int unit_size; //determine how much bits you read/write/present
unsigned char mem_buf[10000];
size_t mem_count;


struct fun_desc {
    char *name;
    char (*fun)(); //void funcs
    };


struct fun_desc menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"Load Into Memory", not_implemented},
    {"Toggle Display Mode", not_implemented},
    {"Memory Display", not_implemented},
    {"Save Into File", not_implemented},
    {"Memory Modify", not_implemented},
    {"Quit", quit},
    {NULL, NULL}
};

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    fprintf(stderr, "Debug flag now %s\n", debug_mode ? "on" : "off");
}


void set_file_name() {
    printf("Please enter file name: ");
    fgets(file_name, 128, stdin); // file name is no longer than 100 characters
    size_t len = strlen(file_name);
    if(len >0 && file_name[len - 1] == '\n')
        file_name[len - 1] = '\0';
    if(debug_mode)
        fprintf(stderr, "Debug: file name set to '%s \n", file_name);
}

void set_unit_size() {
    int user_input; //temp var for storing user input
    printf("Please enter unit size (1, 2, or 4): ");
    if(scanf("%d", &user_input) != 1){ //tries to read a integer from the user
        printf("Invalid input\n"); // if its not a number !
        while (getchar() != 'n'); //cleaning no needed chars 
        return;
    }
    if(user_input == 1 || user_input == 2 || user_input == 4){
        unit_size = user_input;
        if(debug_mode) fprintf(stderr, "Debug: set size to %d\n", unit_size);
    }
    else printf("Invalid unit size\n");
    while (getchar() != 'n'); // clear buffer
}

void quit(){ if(debug_mode) fprintf(stderr, "quitting\n"); exit(0); }

void not_implemented(){ printf ("Not implemented yet\n"); }


int main(){
    int choice;
    while (1)
    {
        if(debug_mode) 
            fprintf(stderr, "Debug: unit_size=%d file_name='%s' mem_count=%zu\n", unit_size, file_name, mem_count);

        for(int i = 0; menu[i].name != NULL; i++)
            printf("%d) %s\n", i, menu[i].name);
        
        if(scanf("%d", &choice) != 1){
            printf("Invalid input\n");
            while (getchar() != '\n');
            continue;
        }    

        if(choice >= 0 && choice < 9)
            menu[choice].fun(); // if its a valid chioce(0-8) - operate the right func
        else
            printf("Not within bounds\n");
        
    }
    



}
