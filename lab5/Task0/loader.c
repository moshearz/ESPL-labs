#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    
    // Check if this is actually an ELF file
    if (header->e_ident[0] != 0x7f || header->e_ident[1] != 'E' || 
        header->e_ident[2] != 'L' || header->e_ident[3] != 'F') {
        printf("This is not an ELF file!\n");
        return -1;
    }
    
    // Make sure it's 32-bit
    if (header->e_ident[4] != 1) {
        printf("This is not a 32-bit ELF file!\n");
        return -1;
    }
    
    // Find where the program headers start
    Elf32_Phdr *phdrs = (Elf32_Phdr *)((char *)map_start + header->e_phoff);
    
    // Go through each program header
    for (int i = 0; i < header->e_phnum; i++) {
        func(&phdrs[i], arg);
    }
    
    return header->e_phnum;
}

// Function to convert program header type to string
const char* get_type_string(Elf32_Word p_type) {
    switch(p_type) {
        case PT_NULL: return "NULL";
        case PT_LOAD: return "LOAD";
        case PT_DYNAMIC: return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE: return "NOTE";
        case PT_SHLIB: return "SHLIB";
        case PT_PHDR: return "PHDR";
        case PT_TLS: return "TLS";
        case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
        case PT_GNU_STACK: return "GNU_STACK";
        case PT_GNU_RELRO: return "GNU_RELRO";
        default: return "UNKNOWN";
    }
}

// Function to format flags into readable string
void get_flags_string(Elf32_Word p_flags, char *flags_str) {
    flags_str[0] = (p_flags & PF_R) ? 'R' : ' ';
    flags_str[1] = (p_flags & PF_W) ? 'W' : ' ';
    flags_str[2] = (p_flags & PF_X) ? 'E' : ' ';
    flags_str[3] = '\0';
}

// Function to print program header information in readelf format
void print_phdr_details(Elf32_Phdr *phdr, int index) {
    char flags[4];
    get_flags_string(phdr->p_flags, flags);
    
    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %-3s 0x%x\n",
           get_type_string(phdr->p_type),
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           flags,
           phdr->p_align);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <elf_file>\n", argv[0]);
        return 1;
    }
    
    // Open the file
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("Cannot open file %s\n", argv[1]);
        return 1;
    }
    
    // Get file size
    struct stat st;
    fstat(fd, &st);
    
    // Map the file into memory so we can read it
    void *file_data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        printf("Failed to map file\n");
        close(fd);
        return 1;
    }
    
    printf("Program Headers:\n");
    printf("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n");
    
    // Use our function to go through all program headers
    int num_headers = foreach_phdr(file_data, print_phdr_details, 0);
    
    if (num_headers > 0) {
        printf("\nFound %d program headers\n", num_headers);
    }
    
    // Clean up
    munmap(file_data, st.st_size);
    close(fd);
    
    return 0;
}