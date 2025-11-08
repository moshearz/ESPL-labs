#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>

// We can handle up to 2 ELF files as required
int fd1 = -1, fd2 = -1;
void *map1 = NULL, *map2 = NULL;
size_t size1 = 0, size2 = 0;
int file_count = 0;
int debug_mode = 0;

// Menu structure - same technique as lab 1
typedef struct {
    char* name;
    void (*func)(void);
} menu_option;

// Function declarations
void toggle_debug();
void examine_elf();
void print_section_names();
void print_symbols();
void check_files_merge();
void merge_elf_files();
void quit_program();

// Menu array
menu_option menu[] = {
    {"Toggle Debug Mode", toggle_debug},
    {"Examine ELF File", examine_elf},
    {"Print Section Names", print_section_names},
    {"Print Symbols", print_symbols},
    {"Check Files for Merge", check_files_merge},
    {"Merge ELF Files", merge_elf_files},
    {"Quit", quit_program}
};

void toggle_debug() {
    debug_mode = !debug_mode;
    printf("Debug flag now %s\n", debug_mode ? "on" : "off");
}

void examine_elf() {
    // Can't handle more than 2 files
    if (file_count >= 2) {
        printf("Error: Already have 2 files loaded\n");
        return;
    }
    
    char filename[256];
    printf("Enter ELF file name: ");
    scanf("%255s", filename);
    
    // Open the file for reading
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }
    
    // Find file size using lseek as instructed
    size_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    
    // Map entire file into memory with one mmap call
    void *map_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        printf("Error: Cannot map file\n");
        close(fd);
        return;
    }
    
    // Cast to ELF header to examine it
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)map_start;
    
    // Check if this is actually an ELF file
    if (elf_header->e_ident[0] != 0x7f || 
        elf_header->e_ident[1] != 'E' || 
        elf_header->e_ident[2] != 'L' || 
        elf_header->e_ident[3] != 'F') {
        printf("Error: Not a valid ELF file\n");
        munmap(map_start, file_size);
        close(fd);
        return;
    }
    
    // Make sure it's 32-bit ELF as required
    if (elf_header->e_ident[EI_CLASS] != ELFCLASS32) {
        printf("Error: Not a 32-bit ELF file\n");
        munmap(map_start, file_size);
        close(fd);
        return;
    }
    
    // Store file info in global variables
    if (file_count == 0) {
        fd1 = fd;
        map1 = map_start;
        size1 = file_size;
    } else {
        fd2 = fd;
        map2 = map_start;
        size2 = file_size;
    }
    file_count++;
    
    printf("File %s:\n", filename);
    
    // Print magic number bytes 1,2,3 (skip the 0x7f)
    printf("Magic number (bytes 1-3): %c%c%c\n", 
           elf_header->e_ident[1], elf_header->e_ident[2], elf_header->e_ident[3]);
    
    // Print data encoding
    printf("Data encoding scheme: ");
    if (elf_header->e_ident[EI_DATA] == ELFDATA2LSB) {
        printf("2's complement, little endian\n");
    } else if (elf_header->e_ident[EI_DATA] == ELFDATA2MSB) {
        printf("2's complement, big endian\n");
    } else {
        printf("Invalid\n");
    }
    
    // Print all the required header information
    printf("Entry point: 0x%x\n", elf_header->e_entry);
    printf("Section header table file offset: %d (bytes into file)\n", elf_header->e_shoff);
    printf("Number of section header entries: %d\n", elf_header->e_shnum);
    printf("Size of section header entries: %d (bytes)\n", elf_header->e_shentsize);
    printf("Program header table file offset: %d (bytes into file)\n", elf_header->e_phoff);
    printf("Number of program header entries: %d\n", elf_header->e_phnum);
    printf("Size of program header entries: %d (bytes)\n", elf_header->e_phentsize);
    
    if (debug_mode) {
        printf("DEBUG: fd=%d, size=%zu, map=%p\n", fd, file_size, map_start);
    }
}

void print_section_names() {
    // Check if any files are loaded
    if (file_count == 0) {
        printf("Error: No ELF files loaded\n");
        return;
    }
    
    // Print sections for each loaded file
    for (int file_idx = 0; file_idx < file_count; file_idx++) {
        void *map_start;
        char *filename;
        
        // Get the right file data
        if (file_idx == 0) {
            map_start = map1;
            filename = "File 1";
        } else {
            map_start = map2;
            filename = "File 2";
        }
        
        printf("File %s\n", filename);
        
        // Get ELF header
        Elf32_Ehdr *elf_header = (Elf32_Ehdr*)map_start;
        
        // Get section header table
        Elf32_Shdr *section_headers = (Elf32_Shdr*)((char*)map_start + elf_header->e_shoff);
        
        // Get string table section header (contains section names)
        int shstrndx = elf_header->e_shstrndx;
        Elf32_Shdr *string_table_header = &section_headers[shstrndx];
        
        // Get pointer to the actual string table data
        char *string_table = (char*)map_start + string_table_header->sh_offset;
        
        if (debug_mode) {
            printf("DEBUG: shstrndx=%d, string_table_offset=%d\n", 
                   shstrndx, string_table_header->sh_offset);
        }
        
        // Print each section
        for (int i = 0; i < elf_header->e_shnum; i++) {
            Elf32_Shdr *section = &section_headers[i];
            
            // Get section name from string table
            char *section_name = string_table + section->sh_name;
            
            if (debug_mode) {
                printf("DEBUG: section[%d] name_offset=%d\n", i, section->sh_name);
            }
            
            // Print section info in the required format
            printf("[%2d] %-15s 0x%08x 0x%06x %6d %2d\n",
                   i,                        // index
                   section_name,             // section_name
                   section->sh_addr,         // section_address
                   section->sh_offset,       // section_offset
                   section->sh_size,         // section_size
                   section->sh_type);        // section_type
        }
        
        printf("\n");
    }
}

void print_symbols() {
    // Check if any files are loaded
    if (file_count == 0) {
        printf("Error: No ELF files loaded\n");
        return;
    }
    
    // Print symbols for each loaded file
    for (int file_idx = 0; file_idx < file_count; file_idx++) {
        void *map_start;
        char *filename;
        
        // Get the right file data
        if (file_idx == 0) {
            map_start = map1;
            filename = "File 1";
        } else {
            map_start = map2;
            filename = "File 2";
        }
        
        printf("File %s\n", filename);
        
        // Get ELF header
        Elf32_Ehdr *elf_header = (Elf32_Ehdr*)map_start;
        
        // Get section header table
        Elf32_Shdr *section_headers = (Elf32_Shdr*)((char*)map_start + elf_header->e_shoff);
        
        // Get section name string table for section names
        int shstrndx = elf_header->e_shstrndx;
        Elf32_Shdr *shstrtab_header = &section_headers[shstrndx];
        char *section_name_table = (char*)map_start + shstrtab_header->sh_offset;
        
        // Look for symbol table sections (.symtab and .dynsym)
        int found_symbols = 0;
        
        for (int i = 0; i < elf_header->e_shnum; i++) {
            Elf32_Shdr *section = &section_headers[i];
            
            // Check if this is a symbol table section
            if (section->sh_type == SHT_SYMTAB || section->sh_type == SHT_DYNSYM) {
                found_symbols = 1;
                
                // Get symbol table data
                Elf32_Sym *symbol_table = (Elf32_Sym*)((char*)map_start + section->sh_offset);
                int num_symbols = section->sh_size / sizeof(Elf32_Sym);
                
                // Get associated string table (sh_link points to string table section)
                int strtab_index = section->sh_link;
                Elf32_Shdr *strtab_header = &section_headers[strtab_index];
                char *symbol_string_table = (char*)map_start + strtab_header->sh_offset;
                
                if (debug_mode) {
                    char *symtab_name = section_name_table + section->sh_name;
                    printf("DEBUG: Found symbol table '%s' at section %d\n", symtab_name, i);
                    printf("DEBUG: Symbol table size: %d bytes, %d symbols\n", 
                           section->sh_size, num_symbols);
                    printf("DEBUG: Associated string table at section %d\n", strtab_index);
                }
                
                // Print each symbol
                for (int j = 0; j < num_symbols; j++) {
                    Elf32_Sym *symbol = &symbol_table[j];
                    
                    // Get symbol name from string table
                    char *symbol_name = symbol_string_table + symbol->st_name;
                    
                    // Get section name where symbol is defined
                    char *section_name;
                    if (symbol->st_shndx == SHN_UNDEF) {
                        section_name = "UND";
                    } else if (symbol->st_shndx == SHN_ABS) {
                        section_name = "ABS";
                    } else if (symbol->st_shndx == SHN_COMMON) {
                        section_name = "COM";
                    } else if (symbol->st_shndx < elf_header->e_shnum) {
                        // Valid section index - get section name
                        Elf32_Shdr *sym_section = &section_headers[symbol->st_shndx];
                        section_name = section_name_table + sym_section->sh_name;
                    } else {
                        section_name = "???";
                    }
                    
                    if (debug_mode && j < 5) {  // Only show debug for first few symbols
                        printf("DEBUG: symbol[%d] name_offset=%d, section_index=%d\n", 
                               j, symbol->st_name, symbol->st_shndx);
                    }
                    
                    // Print symbol info in required format
                    printf("[%3d] 0x%08x %3d %-10s %s\n",
                           j,                     // index
                           symbol->st_value,      // value
                           symbol->st_shndx,      // section_index
                           section_name,          // section_name
                           symbol_name);          // symbol_name
                }
                
                printf("\n");
            }
        }
        
        if (!found_symbols) {
            printf("Error: No symbol table found\n");
        }
    }
}

// Helper function to find a symbol in a symbol table by name
int find_symbol(Elf32_Sym *symbol_table, int num_symbols, char *string_table, char *symbol_name) {
    for (int i = 1; i < num_symbols; i++) {  // Skip symbol 0 (dummy symbol)
        char *current_name = string_table + symbol_table[i].st_name;
        if (strcmp(current_name, symbol_name) == 0) {
            return i;  // Return index of found symbol
        }
    }
    return -1;  // Symbol not found
}

void check_files_merge() {
    // Check that exactly 2 ELF files are loaded
    if (file_count != 2) {
        printf("Error: Need exactly 2 ELF files for merging\n");
        return;
    }
    
    // Get ELF headers for both files
    Elf32_Ehdr *elf1 = (Elf32_Ehdr*)map1;
    Elf32_Ehdr *elf2 = (Elf32_Ehdr*)map2;
    
    // Get section headers for both files
    Elf32_Shdr *sections1 = (Elf32_Shdr*)((char*)map1 + elf1->e_shoff);
    Elf32_Shdr *sections2 = (Elf32_Shdr*)((char*)map2 + elf2->e_shoff);
    
    // Find symbol tables in both files
    Elf32_Shdr *symtab1 = NULL, *symtab2 = NULL;
    char *strtab1 = NULL, *strtab2 = NULL;
    int symtab_count1 = 0, symtab_count2 = 0;
    
    // Look for .symtab sections (not .dynsym for object files)
    for (int i = 0; i < elf1->e_shnum; i++) {
        if (sections1[i].sh_type == SHT_SYMTAB) {
            symtab1 = &sections1[i];
            strtab1 = (char*)map1 + sections1[sections1[i].sh_link].sh_offset;
            symtab_count1++;
        }
    }
    
    for (int i = 0; i < elf2->e_shnum; i++) {
        if (sections2[i].sh_type == SHT_SYMTAB) {
            symtab2 = &sections2[i];
            strtab2 = (char*)map2 + sections2[sections2[i].sh_link].sh_offset;
            symtab_count2++;
        }
    }
    
    // Check that each file has exactly one symbol table
    if (symtab_count1 != 1 || symtab_count2 != 1) {
        printf("Feature not supported: Each file must have exactly one symbol table\n");
        return;
    }
    
    if (!symtab1 || !symtab2) {
        printf("Error: Symbol tables not found\n");
        return;
    }
    
    // Get symbol table data
    Elf32_Sym *symbols1 = (Elf32_Sym*)((char*)map1 + symtab1->sh_offset);
    Elf32_Sym *symbols2 = (Elf32_Sym*)((char*)map2 + symtab2->sh_offset);
    int num_symbols1 = symtab1->sh_size / sizeof(Elf32_Sym);
    int num_symbols2 = symtab2->sh_size / sizeof(Elf32_Sym);
    
    if (debug_mode) {
        printf("DEBUG: File 1 has %d symbols, File 2 has %d symbols\n", 
               num_symbols1, num_symbols2);
    }
    
    printf("Checking merge compatibility...\n");
    
    // Check symbols in file 1 against file 2
    for (int i = 1; i < num_symbols1; i++) {  // Skip symbol 0 (dummy)
        Elf32_Sym *sym1 = &symbols1[i];
        char *sym_name = strtab1 + sym1->st_name;
        
        // Skip empty symbol names
        if (strlen(sym_name) == 0) continue;
        
        if (debug_mode) {
            printf("DEBUG: Checking symbol '%s' from file 1\n", sym_name);
        }
        
        // Look for this symbol in file 2
        int found_idx = find_symbol(symbols2, num_symbols2, strtab2, sym_name);
        
        if (sym1->st_shndx == SHN_UNDEF) {
            // Symbol is undefined in file 1
            if (found_idx == -1 || symbols2[found_idx].st_shndx == SHN_UNDEF) {
                // Not found in file 2 or also undefined there
                printf("Symbol %s undefined\n", sym_name);
            }
        } else {
            // Symbol is defined in file 1
            if (found_idx != -1 && symbols2[found_idx].st_shndx != SHN_UNDEF) {
                // Also defined in file 2
                printf("Symbol %s multiply defined\n", sym_name);
            }
        }
    }
    
    // Check symbols in file 2 against file 1
    for (int i = 1; i < num_symbols2; i++) {  // Skip symbol 0 (dummy)
        Elf32_Sym *sym2 = &symbols2[i];
        char *sym_name = strtab2 + sym2->st_name;
        
        // Skip empty symbol names
        if (strlen(sym_name) == 0) continue;
        
        if (debug_mode) {
            printf("DEBUG: Checking symbol '%s' from file 2\n", sym_name);
        }
        
        // Look for this symbol in file 1
        int found_idx = find_symbol(symbols1, num_symbols1, strtab1, sym_name);
        
        if (sym2->st_shndx == SHN_UNDEF) {
            // Symbol is undefined in file 2
            if (found_idx == -1 || symbols1[found_idx].st_shndx == SHN_UNDEF) {
                // Not found in file 1 or also undefined there
                printf("Symbol %s undefined\n", sym_name);
            }
        } else {
            // Symbol is defined in file 2
            if (found_idx != -1 && symbols1[found_idx].st_shndx != SHN_UNDEF) {
                // Also defined in file 1 (already reported above, but instruction says to continue)
                printf("Symbol %s multiply defined\n", sym_name);
            }
        }
    }
    
    printf("Merge check completed\n");
}

// Helper function to find section by name in file
int find_section_by_name(void *map_start, char *section_name) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)map_start;
    Elf32_Shdr *sections = (Elf32_Shdr*)((char*)map_start + elf_header->e_shoff);
    
    // Get section name string table
    int shstrndx = elf_header->e_shstrndx;
    Elf32_Shdr *shstrtab_header = &sections[shstrndx];
    char *section_names = (char*)map_start + shstrtab_header->sh_offset;
    
    for (int i = 0; i < elf_header->e_shnum; i++) {
        char *current_name = section_names + sections[i].sh_name;
        if (strcmp(current_name, section_name) == 0) {
            return i;
        }
    }
    return -1;  // Not found
}

void merge_elf_files() {
    // Check that exactly 2 ELF files are loaded
    if (file_count != 2) {
        printf("Error: Need exactly 2 ELF files for merging\n");
        return;
    }
    
    printf("Merging ELF files...\n");
    
    // Get ELF headers and section headers for both files
    Elf32_Ehdr *elf1 = (Elf32_Ehdr*)map1;
    Elf32_Ehdr *elf2 = (Elf32_Ehdr*)map2;
    Elf32_Shdr *sections1 = (Elf32_Shdr*)((char*)map1 + elf1->e_shoff);
    Elf32_Shdr *sections2 = (Elf32_Shdr*)((char*)map2 + elf2->e_shoff);
    
    // Get section name string table from file 1
    int shstrndx = elf1->e_shstrndx;
    Elf32_Shdr *shstrtab_header = &sections1[shstrndx];
    char *section_names = (char*)map1 + shstrtab_header->sh_offset;
    
    // Create output file
    int out_fd = open("out.ro", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) {
        printf("Error: Cannot create output file out.ro\n");
        return;
    }
    
    // Create new ELF header (copy from file 1)
    Elf32_Ehdr new_elf_header = *elf1;
    
    // Write initial ELF header (we'll update e_shoff later)
    if (write(out_fd, &new_elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        printf("Error: Cannot write ELF header\n");
        close(out_fd);
        return;
    }
    
    // Create new section header table (copy from file 1)
    Elf32_Shdr *new_sections = malloc(elf1->e_shnum * sizeof(Elf32_Shdr));
    if (!new_sections) {
        printf("Error: Cannot allocate memory for section headers\n");
        close(out_fd);
        return;
    }
    memcpy(new_sections, sections1, elf1->e_shnum * sizeof(Elf32_Shdr));
    
    if (debug_mode) {
        printf("DEBUG: Created new section header table with %d entries\n", elf1->e_shnum);
    }
    
    // Process each section
    for (int i = 0; i < elf1->e_shnum; i++) {
        char *section_name = section_names + sections1[i].sh_name;
        
        if (debug_mode) {
            printf("DEBUG: Processing section %d: '%s'\n", i, section_name);
        }
        
        // Update section offset to current file position
        new_sections[i].sh_offset = lseek(out_fd, 0, SEEK_CUR);
        
        if (i == 0) {
            // Section 0 is always empty - just update offset
            if (debug_mode) {
                printf("DEBUG: Section 0 (NULL) - skipping\n");
            }
            continue;
        }
        
        // Check if this is a mergeable section
        if (strcmp(section_name, ".text") == 0 || 
            strcmp(section_name, ".data") == 0 || 
            strcmp(section_name, ".rodata") == 0) {
            
            // Mergeable section - concatenate content from both files
            char *section1_data = (char*)map1 + sections1[i].sh_offset;
            int section1_size = sections1[i].sh_size;
            
            // Write content from file 1
            if (write(out_fd, section1_data, section1_size) != section1_size) {
                printf("Error: Cannot write section data from file 1\n");
                free(new_sections);
                close(out_fd);
                return;
            }
            
            // Find corresponding section in file 2
            int section2_idx = find_section_by_name(map2, section_name);
            if (section2_idx != -1) {
                char *section2_data = (char*)map2 + sections2[section2_idx].sh_offset;
                int section2_size = sections2[section2_idx].sh_size;
                
                // Write content from file 2
                if (write(out_fd, section2_data, section2_size) != section2_size) {
                    printf("Error: Cannot write section data from file 2\n");
                    free(new_sections);
                    close(out_fd);
                    return;
                }
                
                // Update merged section size
                new_sections[i].sh_size = section1_size + section2_size;
                
                if (debug_mode) {
                    printf("DEBUG: Merged '%s': file1_size=%d + file2_size=%d = total_size=%d\n",
                           section_name, section1_size, section2_size, new_sections[i].sh_size);
                }
            } else {
                // Section not found in file 2, just use file 1's content
                new_sections[i].sh_size = section1_size;
                
                if (debug_mode) {
                    printf("DEBUG: Section '%s' not found in file 2, using only file 1\n", section_name);
                }
            }
        } else {
            // Non-mergeable section - copy from file 1 as-is
            char *section_data = (char*)map1 + sections1[i].sh_offset;
            int section_size = sections1[i].sh_size;
            
            if (write(out_fd, section_data, section_size) != section_size) {
                printf("Error: Cannot write section data\n");
                free(new_sections);
                close(out_fd);
                return;
            }
            
            new_sections[i].sh_size = section_size;
            
            if (debug_mode) {
                printf("DEBUG: Copied '%s' as-is, size=%d\n", section_name, section_size);
            }
        }
    }
    
    // Write section header table at the end
    off_t shoff = lseek(out_fd, 0, SEEK_CUR);
    int shdr_table_size = elf1->e_shnum * sizeof(Elf32_Shdr);
    
    if (write(out_fd, new_sections, shdr_table_size) != shdr_table_size) {
        printf("Error: Cannot write section header table\n");
        free(new_sections);
        close(out_fd);
        return;
    }
    
    if (debug_mode) {
        printf("DEBUG: Section header table written at offset %ld\n", shoff);
    }
    
    // Update ELF header with correct section header table offset
    new_elf_header.e_shoff = shoff;
    
    // Seek back to beginning and rewrite ELF header
    lseek(out_fd, 0, SEEK_SET);
    if (write(out_fd, &new_elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        printf("Error: Cannot update ELF header\n");
        free(new_sections);
        close(out_fd);
        return;
    }
    
    // Clean up and close
    free(new_sections);
    close(out_fd);
    
    printf("Merge completed successfully. Output file: out.ro\n");
}

void quit_program() {
    // Clean up any open files before exiting
    if (map1) munmap(map1, size1);
    if (map2) munmap(map2, size2);
    if (fd1 != -1) close(fd1);
    if (fd2 != -1) close(fd2);
    
    printf("Goodbye!\n");
    exit(0);
}

int main() {
    int choice;
    int menu_size = sizeof(menu) / sizeof(menu_option);
    
    while (1) {
        // Print menu options
        printf("\nChoose action:\n");
        for (int i = 0; i < menu_size; i++) {
            printf("%d-%s\n", i, menu[i].name);
        }
        
        printf("Choice: ");
        scanf("%d", &choice);
        
        // Call the selected function
        if (choice >= 0 && choice < menu_size) {
            menu[choice].func();
        } else {
            printf("Invalid choice\n");
        }
    }
    
    return 0;
}