# Extended Systems Programming Laboratory (BGU)

This repository contains the complete set of labs developed during the Extended Systems Programming Laboratory course at the Department of Computer Science, Ben-Gurion University of the Negev.

## Overview

The course focuses on **low-level system programming** and interaction with the **Linux operating system**.  
All labs were implemented mainly in **C**, with some components written in **x86 assembly**.  
Throughout the semester, the work gradually builds up from basic I/O handling to full system-level tools such as a custom shell, ELF file manipulation, and assembly-based programming.

## Topics Covered
1. Processes, memory models, and interaction with the operating system.  
2. A programmer’s introduction to C — storage types, pointers, and structures.  
3. x86 assembly fundamentals and linking assembly with C.  
4. System calls and the programmer’s interface to kernel services.  
5. The Linux file system, process management, and access permissions.  
6. Command interpreters and the design of a Unix-style shell.  
7. Binary file formats and data manipulation.  
8. The ELF format — linking and loading.

## Labs Included
| Lab | Focus |
|-----|--------|
| **Lab 1 – Lab 5** | Core C programming, memory management, debugging, and pointer operations. |
| **Lab A** | File I/O, command-line parsing, and basic text encoding utilities. |
| **Lab B** | Linked lists, dynamic memory, and virus signature detection. |
| **Lab C** | Implementation of a mini shell supporting pipes, signals, and process management. |
| **Lab D** | Assembly programming and big-number arithmetic in 32-bit mode. |
| **Lab E** | Reading and manipulating ELF files, sections, and symbols using `mmap`, and implementing a basic linker. |

## Technologies
- **Language:** C (ANSI C, 32-bit)  
- **System:** Linux / WSL  
- **Tools:** `gcc`, `make`, `gdb`, `mmap`, `readelf`, `objdump`

## How to Build
Each lab includes its own `Makefile`.  
Example:
```bash
cd labC
make
./myshell
