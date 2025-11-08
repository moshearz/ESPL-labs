#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//global
FILE *infile = NULL;
FILE *outfile = NULL;

//task2 - globals
char* encodingKey = "0"; // Default mode is "no encoding"`
int add_mode = 1; // 1 == e+ | 0 == e-
int encoding_index = 0;


char encode(char c){
    int result = c;
    int encoded = (encodingKey[encoding_index] - '0') * (add_mode ? 1 : -1); 

    if (c>='0' && c<='9'){
        result = ((c - '0') + encoded + 10) % 10 + '0';
    }
    else if (c>= 'A' && c<= 'Z'){
        result = ((c - 'A') + encoded + 26) % 26 + 'A';
    }

    encoding_index = (encodingKey[encoding_index+1] == '\0') ? 0 : encoding_index+1; 
    return result;
}


int main(int argc, char** argv){
    int debug_mode = 1;
    
    infile = stdin;
    outfile = stdout;

    for(int i = 0; i < argc; i++){
        if (debug_mode) { fprintf(stderr, "Argv[%d]=%s\n", i , argv[i]); }

        if (strcmp(argv[i], "-d") == 0) { debug_mode = 0; }

        else if (strcmp(argv[i], "+d") == 0) { debug_mode = 1; }

        else if (strncmp(argv[i], "+e", 2) == 0){
            add_mode = 1;
            encodingKey = argv[i] + 2;
        }
        else if (strncmp(argv[i], "-e", 2) == 0){
            add_mode = 0;
            encodingKey = argv[i] + 2;
        }

        else if(strncmp(argv[i], "-i", 2) == 0){
            infile = fopen(argv[i] + 2, "r");
            if(!infile){
                fprintf(stderr, "Error: can not open input file %s\n", argv[i] + 2 );
                exit(1);
            }
        }
        else if(strncmp(argv[i], "-o", 2) == 0){
            outfile = fopen(argv[i] + 2, "w");
            if(!outfile){
                fprintf(stderr, "Error: can not open output file %s\n", argv[i] + 2 );
                exit(1);
            }
        }
    }

    int c = fgetc(infile);
    while(c != EOF){
        c = encode((char)c);
        fputc(c, outfile);
        c = fgetc(infile); // call next c (next char)
    }

    return 0;
    
}

