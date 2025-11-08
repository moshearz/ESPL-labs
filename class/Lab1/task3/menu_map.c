#include <stdio.h> //library for using in: printf (prints text into the screen) and fgets (reading text line from the input)
#include <stdlib.h>//for using: malloc,free,exit
#include <string.h>//for using actions on strings
#include "base.c"



#define BUFFER_SIZE 256 //Define how many chars we want to read each line from the user, in here we define 127 + "finish char"(\0)

struct fun_desc {
    char *name;
    char (*fun)(char);
    };


int main(int argc, char **argv){
    char input[BUFFER_SIZE];
    char *carray = calloc(5, sizeof(char)); //"Define a pointer 'carray' to a char array of length 5, initialized to the empty string (how?)."
    struct fun_desc menu[] = { { "Get string", my_get }, { "Print string", cprt }, {"Print octal", oprt }, {"Encrypt", encrypt}, {"Decrypt", decrypt} , { NULL, NULL } };

    //repeats forever unless it encounters an EOF condition for stdin
    while(1){ 
        printf("Select operation from the following menu: \n");

        //Displays a menu (as a numbered list) of names (or descriptions) of the functions contained in the array. The menu should be printed by looping over the menu item names from the fun_desc, not by printing a string (or strings) that contain a copy of the name.
        for(int i = 0; menu[i].name != NULL; i++){
            printf("%d) %s\n", i+1, menu[i].name);
        }
        //Displays a prompt asking the user to choose a function by its number in the menu, reads the number, and checks if it is within bounds
        printf("Option: ");

        //if fgets was not able to read input, its return NULL, and this happens when the user press  'Ctrl+D'
        if(fgets(input, BUFFER_SIZE, stdin) == NULL) exit(0);

        int choise = atoi(input);
        //If the number is within bounds, "Within bounds" is printed, otherwise "Not within bounds" is printed and the program exits gracefully.
        if (choise < 1 || choise > 5 ) {
            printf("Not within bounds\n");
            exit(0);
        }
        else{
            printf("Within bounds\n");
            char* new_carray = map(carray, 5, menu[choise-1].fun);
            free(carray);
            carray = new_carray;
            //printf("DONE\n");
        }
    }
    free(carray);
    return 0;
}