#include "util.h" 

#define SYS_WRITE 4
#define SYS_READ 3
#define SYS_OPEN 5
#define SYS_EXIT 1
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 64
#define STDIN 0
#define STDOUT 1
#define STDERR 2

extern int system_call();

int add_mode = 1;
int debug_mode = 0;
char *encodingKey = "0"; 

char encode(char c) {
    static int encoding_index = 0;
    int encoded = (encodingKey[encoding_index] - '0') * (add_mode ? 1 : -1);
    
    if (c >= '0' && c <= '9') {
        c = ((c - '0') + encoded + 10) % 10 + '0';
    } else if (c >= 'A' && c <= 'Z') {
        c = ((c - 'A') + encoded + 26) % 26 + 'A';
    }

    encoding_index = (encodingKey[encoding_index + 1] == '\0') ? 0 : encoding_index + 1;
    return c;
}

int main(int argc, char *argv[]) {
    int input_fd = STDIN;
    int output_fd = STDOUT;
    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '+' && argv[i][1] == 'd') {
            debug_mode = 1;
        } else if (argv[i][0] == '-' && argv[i][1] == 'd') {
            debug_mode = 0;
        } else if (argv[i][0] == '+' && argv[i][1] == 'e') {
            add_mode = 1;
            encodingKey = argv[i] + 2;
        } else if (argv[i][0] == '-' && argv[i][1] == 'e') {
            add_mode = 0;
            encodingKey = argv[i] + 2;
        } else if (argv[i][0] == '-' && argv[i][1] == 'i') {
            input_fd = system_call(SYS_OPEN, argv[i] + 2, O_RDONLY, 0);
            if (input_fd < 0) {
                system_call(SYS_WRITE, STDERR, "Error: cannot open input file\n", 33);
                system_call(SYS_EXIT, 1, 0, 0);
            }
        } else if (argv[i][0] == '-' && argv[i][1] == 'o') {
            output_fd = system_call(SYS_OPEN, argv[i] + 2, O_WRONLY | O_CREAT, 0777);
            if (output_fd < 0) {
                system_call(SYS_WRITE, STDERR, "Error: cannot open output file\n", 34);
                system_call(SYS_EXIT, 1, 0, 0);
            }
        }
    }

    if (debug_mode) {
        for (i = 0; i < argc; i++) {
            system_call(SYS_WRITE, STDERR, "Argv[", 5);
            char index_str[4];
            index_str[0] = '0' + (i / 10);
            index_str[1] = '0' + (i % 10);
            index_str[2] = ']';
            index_str[3] = '\0';
            system_call(SYS_WRITE, STDERR, index_str, 3);
            system_call(SYS_WRITE, STDERR, "=", 1);
            system_call(SYS_WRITE, STDERR, argv[i], strlen(argv[i]));
            system_call(SYS_WRITE, STDERR, "\n", 1);
        }
    }

    char buffer[1024];
    int bytes_read;
    while ((bytes_read = system_call(SYS_READ, input_fd, buffer, sizeof(buffer))) > 0) {
        for (i = 0; i < bytes_read; i++) {
            buffer[i] = encode(buffer[i]);
        }
        system_call(SYS_WRITE, output_fd, buffer, bytes_read);
    }

    system_call(SYS_EXIT, 0, 0, 0);
    return 0;
}
