#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Part 1a
//struct that represents a virus description - GIVEN
//typedef is for creating a new name for an existing type. in our case: original type name: struct virus, new name: virus
//sigSize = how many bytes the signature is
//"The name of the virus is a null terminated string that is stored in 16 bytes"
//sig = the signature of the virus, which is a sequence of bytes (The signature is the most important)
typedef struct virus {
    unsigned short SigSize;
    unsigned char* VirusName;
    unsigned char* Sig;
} virus;

//"Each node in the linked list is represented by the following structure:" (Part 1b)
typedef struct link {
    struct link *nextVirus;
    virus *vir;
} link; 

typedef struct fun_desc {
    char *name;
    void (*fun)();
}fun_desc;


//globals
link* virus_list = NULL; 
char *suspect_file_name = NULL;
int isBigEndian = 0;  // 0 = Little Endian, 1 = Big Endian


//Read one virus from the signature file. return: a pointer to the virus struct
virus* readVirus(FILE* signatureFile){

    virus *v = malloc(sizeof(virus));
    if (!v) return NULL;

    //(fread) read one element from the signature file, the size of the elemnt is 2 bytes (unsigned(only positive values) short(2 bytes(16 bits))). save them into v->SigSize
    //fread returns the number of elements read, if it is not 1, then there was an error so we free the memory and return NULL
    //NOTE: v->sigSize is a number not a pointer, so we use &v->SigSize because in order that fread to work its need an address
    if(fread(&v->SigSize, sizeof(unsigned short), 1, signatureFile) != 1){
        free(v);
        return NULL;
    }

    if (isBigEndian) v->SigSize = (v->SigSize >> 8) | (v->SigSize << 8);
    
    //NOTE: v->VirusNmae is already an address (pointer to an adress in the memorry), so we don't need to use &v->VirusName. v->VirusName points to the memory we allocated for it (using malloc)
    v->VirusName = malloc(16);
    if(fread(v->VirusName, sizeof(unsigned char), 16, signatureFile) != 16){
        free(v->VirusName);
        free(v);
        return NULL;
    }

    v->Sig = malloc(v->SigSize);
    if(fread(v->Sig, sizeof(unsigned char), v->SigSize, signatureFile) != v->SigSize){
        free(v->VirusName);
        free(v->Sig);
        free(v);
        return NULL;
    }

    return v; // (if everything succssed) return the pointer to the virus struct
}


//prints the virus name (in ASCII), the virus signature length (in decimal), and the virus signature (in hexadecimal representation).
void printVirus(virus *v, FILE* output){

    fprintf(output, "Virus name: %s\n", v->VirusName);
    fprintf(output, "Virus size: %u\n", v->SigSize);
    fprintf(output, "signature:\n");

    for(int i = 0; i < v->SigSize; i++) {
        fprintf(output, "%02X ", v->Sig[i]);
        // An attempt that the output will be with the exact spaced and line divion like in the expeted result.. but it seems that there are no regularity
        // if ((i + 1) % 20 != 0 && i != v->SigSize - 1)
        //     fprintf(output, " ");
        // if ((i + 1) % 20 == 0 || i == v->SigSize - 1)
        //     fprintf(output, "\n");
    }

    fprintf(output, "\n\n");
}


//part 1c - this written above part 1b because the func detect_viruses use the func detect detect_virus 
//compares the content of the buffer byte-by-byte with the virus signatures stored in the virus_list linked list
void detect_virus(char *buffer, unsigned int size, link *virus_list){

    for(unsigned int i = 0; i < size; i++){ //running on every byte in the buffer
        link *curr = virus_list;
        while (curr != NULL){
            virus *v = curr->vir; //current virus we want to check
            if(i + v->SigSize <= size){ //checks if there is enough space in the buffer for curr point(i)
                if(memcmp(buffer+i, v->Sig, v->SigSize ) == 0){ //checks if segment of sigsize bytes starting from pos i (buffer + i(pointer to cell num i)) in the buffer identical to v->sig.
                    //"If a virus is detected...prints: "
                    printf("Starting byte location: %d\n", i); //The starting byte location in the suspected file
                    printf("Virus name: %s\n", v->VirusName);// The virus name
                    printf("Size of virus signature: %d\n\n", v->SigSize); //The size of the virus signature
                }
            }
        curr = curr->nextVirus;
        }
    }
}


//Part 1b
/* Print the data of every link in list to the given stream. Each item followed by a newline character. */
//the func recieves a pointer to the first node in a linked list (link == struct that contains virus struct) and a file pointer where the virus info will be printed
void list_print(link *virus_list, FILE*){
    link* pointer = virus_list;
    while(pointer != NULL){
        printVirus(pointer->vir, stdout);
        pointer = pointer->nextVirus;
    }
}


/* Add a new link with the given data to the list (at the end CAN ALSO AT BEGINNING), and return a pointer to the list (i.e., the first link in the list). If the list is null - create a new entry and return a pointer to the entry. */
//the func recieves a pointer to the first node in a linked list and a pointer to virus (we want to add it to our virus_list)
link* list_append(link* virus_list, virus* data){
    link* newLink = (link*) malloc(sizeof(link));
    if (newLink == NULL) return fprintf(stderr, "Memory allocation failed\n"), virus_list;

    newLink->vir = data;
    newLink->nextVirus = NULL;
    if(virus_list == NULL) return newLink;

    link *curr = virus_list;
    while (curr->nextVirus != NULL) curr = curr->nextVirus;
    curr->nextVirus = newLink;
    return virus_list;
}


/* Free the memory allocated by the list. */
void list_free(link *virus_list){
    while(virus_list != NULL){
        //virus list is a pointer to the first node of the linked list, we we will use the adress of the next link before we free the current one (else we lose "contact" with the linked-list and gets a memory lick)
        link* next = virus_list->nextVirus; 
        free(virus_list->vir->Sig);
        free(virus_list->vir->VirusName);
        free(virus_list->vir); //the struct itself 
        free(virus_list); //the structure on the link itself (only at the end!)
        virus_list = next; // move to the next link which we prepered before 
    }
}


//Part 2b - needs to be before fix_file() func 
//The func recieves the name of the file where the virus are and the exact location where virus sig starts
void neutralize_virus(char *fileName, int signatureOffset){
    FILE *file = fopen(fileName, "rb+"); // r=read, b=binary, +=you can also write 
    if (!file){
        perror("Eror open file (for neutralize)");
        return;
    }
    fseek(file, signatureOffset, SEEK_SET); //SEEK_SET – It moves file pointer position to the beginning of the file.
    unsigned char binaryCodeRET = 0xC3; 
    if(fwrite(&binaryCodeRET, 1, 1, file) != 1) return perror("Failed to write RET instruction"); 
    fclose(file);
}


//"Load signatures requests a signature file name parameter from the user after the user runs it by entering "1".
void load_signatures(){
    char bufferFilleName[256]; //Allocate space to the input from the end-user for the file name.
    printf("Please enter signature file name: ");
    if (!fgets(bufferFilleName, sizeof(bufferFilleName), stdin)) return; //if fails - do nothing.
    bufferFilleName[strcspn(bufferFilleName, "\n")] = '\0';

    //argv[1] == name of the file the user insert. "rb" == read binary mode (because this is a binary file so the data inside is just like it kept it the memory).
    FILE *signatureFile = fopen(bufferFilleName, "rb");
    if (!signatureFile){
        perror("Error opening signature file");
        return;
    }

    //cheack the mahgic number (character sequence "VIRL" for little-endian encoding, and "VIRB" for big-endian encoding).
    //in C there are no string type as we know. instead we have an array of char(= char[]) that ends with: /0 (= null terminating character)
    //NOTE: memcmp compares binary-byte, so we dont use magicNum as a string because we use it a sequence of bytes.
    char magicNum[4] = {0};
    if(fread(magicNum, 1, 4, signatureFile) != 4){
        fprintf(stderr, "Error reading magic number\n");
        fclose(signatureFile);
        return;
    }
    // int isBigEndian = memcmp(magicNum, "VIRB", 4) == 0; //memcmp() == 0 -> isBigEndian = 1 (=true). else -> 0(=false).
    if (memcmp(magicNum, "VIRB", 4) == 0)
        isBigEndian = 1;
    else if (memcmp(magicNum, "VIRL", 4) == 0)
        isBigEndian = 0;
    else {
        fprintf(stderr, "Error: magic number is incorrect\n");
        fclose(signatureFile);
    return;
    }

    //Endianess check (Endian define the order of bytes when we save a number with more than 1 byte)
    //Big-endian: the most significant byte (MSB) is sortet first
    //Little-endian: the least significant byte (LSB) is sortet first
    //Data: 0x1234. Big-endian: 0x12 0x34. Little-endian: 0x34 0x12
    virus *v = NULL;
    list_free(virus_list);
    virus_list = NULL;
    while ((v = readVirus(signatureFile)) != NULL){
        // if(isBigEndian) v->SigSize = (v->SigSize << 8) | (v->SigSize >> 8);
        virus_list = list_append(virus_list, v);
    }

    fclose(signatureFile);

}


//Print signatures can be used to print them to the screen. If no file is loaded, nothing is printed
void print_signatures(){ if (virus_list) list_print(virus_list,stdout); }


void detect_viruses() {
    if (virus_list == NULL) return;
    FILE *file = fopen(suspect_file_name, "rb");
    char buffer[10000]; //read the data of the file into this buffer we just made 
    int size = fread(buffer,1, 10000, file);
    fclose(file);
    detect_virus(buffer,size,virus_list);
 }


void fix_file() {
    FILE *file = fopen(suspect_file_name, "rb");
    if(!file) return;
    char buffer[10000];
    fseek(file, 0, SEEK_SET);
    int size = fread(buffer, 1,10000,file); //reads the whole data from the file to a buffer 1000 bytes(requred)
    fclose(file);
    for(int i = 0; i < size; i++){
        link *curr = virus_list;
        while (curr!=NULL){
            virus *v = curr->vir;
            if(i+v->SigSize <= size){
                if(memcmp(buffer + i, v->Sig, v->SigSize) == 0){
                    printf("Neutralizing %s at offset %d\n", v->VirusName, i);
                    neutralize_virus(suspect_file_name, i);
                }
            }
            curr= curr->nextVirus;
        }
    }
 }


void quit(){
    list_free(virus_list);
    virus_list = NULL;
    printf("Exiting program...\n");
    exit(0);
}


int main(int argc, char *argv[]){ 
    char input[256];
    //int soffiesChoice = 0;
    suspect_file_name = argv[1];
    struct fun_desc menu[] = { 
        { "Load signatures", load_signatures}, 
        { "Print signatures", print_signatures},
        { "Detect viruses", detect_viruses} ,
        { "Fix file", fix_file},
        { "Quit", quit},
        { NULL, NULL}
    };

    while(1){
        printf("Select operation from the following menu: \n");

        //Displays a menu (as a numbered list) of names (or descriptions) of the functions contained in the array. The menu should be printed by looping over the menu item names from the fun_desc, not by printing a string (or strings) that contain a copy of the name.
        for(int i = 0; menu[i].name != NULL; i++){ printf("%d) %s\n", i+1, menu[i].name);}

        printf("Option: ");
        if (!fgets(input, sizeof(input), stdin)) quit();
        //printf("\n");

        int soffiesChoice = atoi(input);
        //If the number is within bounds, "Within bounds" is printed, otherwise "Not within bounds" is printed and the program exits gracefully.
        if (soffiesChoice< 1 || soffiesChoice > 5 ) {
            printf("Not within bounds\n");
            quit();
        }
        else{
            printf("Within bounds\n");
            menu[soffiesChoice - 1].fun(); //Reads the func which the num represnts/
            printf("\n");
        }
    }
    return 0;
}

