#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> // ssize_t is a type which defined under: <unistd.h>

// (Given) global variables
char debug_mode;
char file_name[128];
int unit_size; //determine how much bits you read/write/present
unsigned char mem_buf[10000]; 
size_t mem_count;

char display_mode = 0 ; //1b - global var 

//funcs declaration
void toggle_debug_mode();
void set_file_name();
void set_unit_size();
void load_into_memory();
void toggle_display_mode();
void memory_display();
void save_into_file();
void memory_modify();
void not_implemented();
void quit();

struct fun_desc {
    char *name;
    void (*fun)(); //void funcs
    };


struct fun_desc menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"Load Into Memory", load_into_memory}, //1a
    {"Toggle Display Mode", toggle_display_mode}, //1b
    {"Memory Display", memory_display}, //1c
    {"Save Into File", save_into_file}, //1d
    {"Memory Modify", memory_modify},
    {"Quit", quit},
    {NULL, NULL}
};

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    fprintf(stderr, "Debug flag now %s\n", debug_mode ? "on" : "off");
}


void set_file_name() {
    printf("Please enter file name: ");
    while (getchar() != '\n'); //cleaning from last input
    fgets(file_name, 128, stdin); // file name is no longer than 100 characters
    size_t len = strlen(file_name);
    if(len >0 && file_name[len - 1] == '\n')
        file_name[len - 1] = '\0';
    if(debug_mode)
        fprintf(stderr, "Debug: file name set to '%s' \n", file_name);
}

void set_unit_size() {
    int user_input; //temp var for storing user input
    printf("Please enter unit size (1, 2, or 4): ");
    if(scanf("%d", &user_input) != 1){ //tries to read a integer from the user
        printf("Invalid input\n"); // if its not a number !
        while (getchar() != '\n'); //cleaning no needed chars 
        return;
    }
    if(user_input == 1 || user_input == 2 || user_input == 4){
        unit_size = user_input;
        if(debug_mode) fprintf(stderr, "Debug: set size to %d\n", unit_size);
    }
    else printf("Invalid unit size\n");
    while (getchar() != '\n'); // clear buffer
}

void quit(){ if(debug_mode) fprintf(stderr, "quitting\n"); exit(0); }

void load_into_memory(){
	if (strcmp(file_name, "") == 0){
		fprintf(stderr, "Error: file name is empty\n");
		return;
	}

    int fd = open(file_name, O_RDONLY);
    if(fd == -1){
        perror("Error with opening the file");
        return;
    }

    while (getchar() != '\n'); // clear leftover \n from previous scanf so now we can enter the lication and length in separate line from the menu option

    char input[256];
    unsigned int location;
    int length;

    printf("Please enter <location(hexadecimal)> <length(decimal)>\n");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        close(fd);
        return;
    }

    if (sscanf(input, "%x %d", &location, &length) != 2) {
        fprintf(stderr, "Invalid input format\n");
        close(fd);
        return;
    }

    if(debug_mode) printf("Debug: file_name='%s', location=0X%X, length=%d\n", file_name, location,length);
    int bytes_to_read = length * unit_size;
    if (bytes_to_read > sizeof(mem_buf)) {
        fprintf(stderr, "Error: requested read size exceeds memory buffer\n");
        close(fd);
        return;
    }

    if (lseek(fd, location, SEEK_SET) == -1) {
        perror("Error with lseek");
        close(fd);
        return;
    }

    ssize_t bytes_read = read(fd, mem_buf, bytes_to_read);
    if (bytes_read < 0) {
        perror("Error reading file");
        close(fd);
        return;
    }

    mem_count = bytes_read / unit_size;

    close(fd);
    printf("Loaded %zu units into memory\n", mem_count);

}


void toggle_display_mode() {
    display_mode = !display_mode;
    if (display_mode)
        printf("Display flag now on, decimal representation\n");
    else
        printf("Display flag now off, hexadecimal representation\n");
}


void memory_display(){
    char input[256]; //for storing input line
    unsigned int addr; //start point of memory( be in hex)
    int length; // num of inits to present

    //where u is the current unit size and val is the val that we want to print, and with the arrays defined as follows:
    //unit size := 1 -> hhx / hhd , 2 -> hx / hd, 3-> x / d
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
	static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
	
    printf("Please enter <address(hex)> <length(decimal)>\n");
    fgets(input, sizeof(input), stdin);

    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        return;
    }

    if (sscanf(input, "%x %d", &addr, &length) != 2) {
        fprintf(stderr, "Invalid input format\n");
        return;
    }

    unsigned char *start = (addr == 0) ? mem_buf : mem_buf + addr; //calc start adress to present inside mem_buf

    if (display_mode)
        printf("%s\n", "Hexadecimal");
    else
        printf("%s\n", "Decimal");


    for (int i = 0; i < length; i++) { //iterate over the num of unit the user ask for, every iterate present one unit in size of unit_size
        unsigned int val = 0; //for the val (unit) read from memory
        memcpy(&val, start + (i * unit_size), unit_size); //copied into val, from : start i (i * unit_size), num of: unit_size bytes
        printf(display_mode ? hex_formats[unit_size - 1] : dec_formats[unit_size - 1], val);
    }
}


void save_into_file(){
    while (getchar() != '\n' && !feof(stdin)); //clean the buffer before fgets
    if (strcmp(file_name, "") == 0) {
        fprintf(stderr, "Error: file name is empty\n");
        return;
    }

    char input[256];
	unsigned int source_address;
	unsigned int target_location;
    int length; // num of unit's in decimal

    printf("Please enter <source-address(hex)> <target-location(hex)> <length(decimal)>\n");
    fgets(input, sizeof(input), stdin); //reads whole line as text
	sscanf(input, "%X %X %d", &source_address, &target_location, &length); //parses the values from the input text in their correct formats(hex or decimal)

    unsigned char *src = (source_address == 0) ? mem_buf : mem_buf + source_address;

    int fd = open(file_name, O_RDWR); //open the file for read and write (without deleting it)
    if (fd < 0) {
        perror("Error opening file for writing");
        return;
    }

    off_t file_size = lseek(fd, 0, SEEK_END); // return the final size of the file (final offset)

    int total_bytes = length * unit_size; //calc the num of bytes to write according to the num of units(length) and unit size

    if (target_location + total_bytes > file_size) { //checkes that the file big enough to store with from the "requested pos/point"
        fprintf(stderr, "Error: target-location + length exceeds file size\n");
        close(fd);
        return;
    }

	if (lseek(fd, target_location, SEEK_SET) == -1) //move the "write-marker" to the "requested pos/point" --> target location
	{
		perror("Error seeking in file");
		close(fd);
		return;
	}

    if (write(fd, src, total_bytes) != total_bytes) { //write to the file the num of bytes which requiered from the memory
        perror("Error writing to file");
        close(fd);
        return;
    }

    if (debug_mode) fprintf(stderr, "Debug: wrote %d bytes (%d units) from mem_buf+0x%x to '%s' at offset 0x%x\n",total_bytes, length, source_address, file_name, target_location);
}


void memory_modify(){
    // Flush leftover stdin chars (critically important!)
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
   // replaces a unit at location in the memory buffer (not virtual memory address!) with val
   char input[128];
   unsigned int location = 0;
   unsigned int val = 0;

   printf("Please enter <location(hex)> and <val(hex)>\n");
   fgets(input, sizeof(input), stdin);

    if (sscanf(input, "%x %x", &location, &val) != 2) {
        fprintf(stderr, "Invalid input format\n");
        return;
    }


    //memory exception check
    if (location + unit_size > mem_count * unit_size) {
        fprintf(stderr, "Error: location + unit size exceeds loaded memory\n");
        return;
    }

    if (debug_mode)  fprintf(stderr, "Debug: location=0x%x val=0x%x\n", location, val); //If debug mode is on, print the location and val given by the user.
    
    memcpy(mem_buf + location, &val, unit_size); // Replace a unit at location in the memory with the value given by val.

}

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
