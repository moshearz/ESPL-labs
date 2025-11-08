#include <stdio.h> 
#include <stdlib.h> 

void PrintHex(unsigned char* buffer, int length) {
    for (int i = 0; i < length; ++i) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s FILE\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    unsigned char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        PrintHex(buffer, bytesRead);
    }

    fclose(fp);
    return 0;
}
