#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>

// Correct startup function signature based on startup.s analysis
// startup(argc, argv, entry_point)
extern void startup(int argc, char **argv, void *entry_point);

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

// Function to print program header information
void print_phdr_info(Elf32_Phdr *phdr) {
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

// Enhanced load_phdr function with BSS handling
void load_phdr(Elf32_Phdr *phdr, int fd) {
    // Only process PT_LOAD segments
    if (phdr->p_type != PT_LOAD) {
        return;
    }
    
    printf("\n=== Loading PT_LOAD Segment ===\n");
    print_phdr_info(phdr);
    
    // Calculate page-aligned addresses
    Elf32_Addr vaddr = phdr->p_vaddr & 0xfffff000;
    Elf32_Off offset = phdr->p_offset & 0xfffff000;
    Elf32_Word padding = phdr->p_vaddr & 0xfff;
    
    printf("Original VAddr: 0x%08x, Aligned VAddr: 0x%08x, Padding: 0x%x\n", 
           phdr->p_vaddr, vaddr, padding);
    
    // Calculate protection flags
    int prot = 0;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;
    
    printf("Protection flags: ");
    if (prot & PROT_READ) printf("READ ");
    if (prot & PROT_WRITE) printf("WRITE ");
    if (prot & PROT_EXEC) printf("EXEC ");
    printf("(0x%x)\n", prot);
    
    // Map the segment
    void *mapped = mmap((void *)vaddr, 
                       phdr->p_memsz + padding,
                       prot,
                       MAP_FIXED | MAP_PRIVATE,
                       fd, 
                       offset);
    
    if (mapped == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    
    printf("✓ Successfully mapped segment at 0x%08x\n", (Elf32_Addr)mapped);
    
    // Handle BSS section (when p_memsz > p_filesz)
    if (phdr->p_memsz > phdr->p_filesz) {
        printf("  BSS section detected: zeroing uninitialized memory\n");
        
        // Calculate where to start zeroing
        void *bss_start = (void *)(phdr->p_vaddr + phdr->p_filesz);
        size_t bss_size = phdr->p_memsz - phdr->p_filesz;
        
        printf("  Zeroing BSS: addr=0x%08x, size=0x%x\n", 
               (unsigned int)bss_start, (unsigned int)bss_size);
        
        // Zero out the BSS section
        memset(bss_start, 0, bss_size);
    }
    
    printf("=== Segment Loading Complete ===\n\n");
}

// Function to get the entry point from ELF header
Elf32_Addr get_entry_point(void *map_start) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    return header->e_entry;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <elf_file> [arg1] [arg2] ...\n", argv[0]);
        printf("Example: %s loadme\n", argv[0]);
        printf("Example: %s encode hello world\n", argv[0]);
        return 1;
    }
    
    printf("ELF Loader - Task 2d (Final)\n");
    printf("Loading and executing file: %s\n", argv[1]);
    
    // Show command line arguments that will be passed to loaded program
    printf("Arguments for loaded program:\n");
    printf("  argc = %d\n", argc - 1);
    for (int i = 1; i < argc; i++) {
        printf("  argv[%d] = \"%s\"\n", i - 1, argv[i]);
    }
    printf("\n");
    
    // Open the ELF file
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Cannot open file");
        return 1;
    }
    
    // Get file size and map it for reading headers
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat failed");
        close(fd);
        return 1;
    }
    
    void *file_data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("Failed to map file for reading");
        close(fd);
        return 1;
    }
    
    // Get the entry point before loading segments
    Elf32_Addr entry_point = get_entry_point(file_data);
    printf("Entry point: 0x%08x\n\n", entry_point);
    
    printf("Program Headers Overview:\n");
    printf("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n");
    
    // Load all PT_LOAD segments
    int loaded_segments = foreach_phdr(file_data, (void (*)(Elf32_Phdr *, int))load_phdr, fd);
    
    if (loaded_segments > 0) {
        printf("Program loading completed successfully!\n");
        printf("Total segments processed: %d\n\n", loaded_segments);
        
        // IMPORTANT: Keep file open and mapped as required by task instructions
        printf("=== Transferring Control to Loaded Program ===\n");
        printf("Jumping to entry point: 0x%08x\n", entry_point);
        printf("Note: File remains open (fd=%d) as required\n\n", fd);
        
        // Task 2d: Prepare arguments for the loaded program
        // The loaded program should see:
        // argv[0] = program name (argv[1] from our perspective)
        // argv[1] = first argument (argv[2] from our perspective)
        // etc.
        
        int loaded_argc = argc - 1;  // Skip our loader name
        char **loaded_argv = &argv[1];  // Point to the loaded program name and its args
        
        printf("Passing %d arguments to loaded program:\n", loaded_argc);
        for (int i = 0; i < loaded_argc; i++) {
            printf("  loaded_argv[%d] = \"%s\"\n", i, loaded_argv[i]);
        }
        printf("\n");
        
        // Transfer control using startup() with proper arguments
        startup(loaded_argc, loaded_argv, (void *)entry_point);
        
        // This should never be reached if the loaded program exits properly
        printf("ERROR: Control returned to loader unexpectedly!\n");
    } else {
        printf("No segments were loaded.\n");
    }
    
    // Clean up (though this may never be reached)
    munmap(file_data, st.st_size);
    close(fd);
    
    return 0;
}