#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "elf.h"
#include "ustar.h"
#include "binary.h"
#include "api.h"

int loadBinary(const char* fname, struct KernelAPI api) {
    char *data = 0;
    int size = tarReadFile(fname, &data);
    if (size == 0) {
        return 0;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)data;
    int isElf = false;
    if (size >= sizeof(Elf32_Ehdr) &&
        ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
        ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
        ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
        ehdr->e_ident[EI_MAG3] == ELFMAG3 &&
        ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
        isElf = true;
    }
    if (ehdr->e_machine != 0x03) return 0; // not x86
    if (ehdr->e_type != ET_EXEC) return 0; // file isn't executable
    if (isElf) {
        for (int i = 0; i < ehdr->e_phnum; i++) { // since theres multiple segments
            Elf32_Phdr* phdr = (Elf32_Phdr*)(data + ehdr->e_phoff + i * ehdr->e_phentsize);
            if (phdr->p_type == PT_LOAD) {
                memcpy((void*)phdr->p_vaddr, (void*)(data + phdr->p_offset), phdr->p_filesz);
                size_t bssSize = phdr->p_memsz - phdr->p_filesz;
                memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, bssSize);
            }
        }
    }
    else return 0; // isnt elf
    void (*entry)(struct KernelAPI*) = (void*)ehdr->e_entry;
    entry(&api);
    return 1;
}