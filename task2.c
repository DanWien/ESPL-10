#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

# define buffLen 100
Elf32_Ehdr *ehdr; 
void* map_start; /* pointer to memory start */
struct stat fildes_stat; /* struct used for file size */
char* currOpenFile=NULL;
int Currentfildes = -1;

extern int startup(int argc, char **argv, void (*start)());


char* typeToStr(int type){
    switch (type){
        case PT_NULL: return "NULL";
        case PT_LOAD: return "LOAD";
        case PT_DYNAMIC: return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE: return "NOTE";
        case PT_SHLIB: return "SHLIB";
        case PT_PHDR: return "PHDR";
        case PT_TLS: return "TLS";
        case PT_NUM: return "NUM";
        case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
        case PT_GNU_STACK: return "GNU_STACK";
        case PT_GNU_RELRO: return "GNU_RELRO";
        case PT_SUNWBSS: return "SUNWBSS";
        case PT_SUNWSTACK: return "SUNWSTACK";
        case PT_HIOS: return "HIOS";
        case PT_LOPROC: return "LOPROC";
        case PT_HIPROC: return "HIPROC"; 
        default:return "Unknown";
    }
}

char* flagToStr(int flg){
    switch (flg){
        case 0x000: return "";
        case 0x001: return "E";
        case 0x002: return "W";
        case 0x003: return "WE";
        case 0x004: return "R";
        case 0x005: return "RE";
        case 0x006: return "RW";
        case 0x007: return "RWE";
        default:return "Unknown";
    }
}

int flagToStr2(int flg){
    switch (flg){
        case 0x000: return 0;
        case 0x001: return PROT_EXEC;
        case 0x002: return PROT_WRITE;
        case 0x003: return PROT_EXEC | PROT_EXEC;
        case 0x004: return PROT_READ;
        case 0x005: return PROT_READ | PROT_EXEC;
        case 0x006: return PROT_READ | PROT_WRITE;
        case 0x007: return PROT_READ | PROT_WRITE | PROT_EXEC;
        default:return -1;
    }
}

void printH(Elf32_Phdr *entry, int n){
    if(entry->p_type == PT_LOAD){
        printf("%s  %#08x  %#08x  %#10.08x  %#07x  %#07x  %s  %#-6.01x",
            typeToStr(entry->p_type),entry->p_offset,entry->p_vaddr,entry->p_paddr,entry->p_filesz,entry->p_memsz,flagToStr(entry->p_flags),entry->p_align);  
        int convertedFlag = flagToStr2(entry -> p_flags);
        printf("   flag : %d  mapping flags : MAP_PRIVATE | MAP_FIXED", convertedFlag);

        printf("\n");
            

    }else{
    printf("%s  %#08x  %#08x  %#10.08x  %#07x  %#07x  %s  %#-6.01x\n",
            typeToStr(entry->p_type),entry->p_offset,entry->p_vaddr,entry->p_paddr,entry->p_filesz,entry->p_memsz,flagToStr(entry->p_flags),entry->p_align);
    }
}


void load_phdr(Elf32_Phdr *phdr, int fd){
    if(phdr->p_type != PT_LOAD)
        return;
    void *vadd = (void *)(phdr->p_vaddr&0xfffff000);
    int offset = phdr->p_offset&0xfffff000;
    int padding = phdr->p_vaddr&0xfff;
    void* tmp;
    if ((tmp = mmap(vadd, phdr->p_memsz + padding, flagToStr2(phdr->p_flags), MAP_FIXED | MAP_PRIVATE, fd, offset)) == MAP_FAILED ) {
      perror("mmap failed");
      exit(1);
   }
    printH(phdr,0);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    for (size_t i = 0; i < ehdr->e_phnum; i++){       
        Elf32_Phdr* entry = map_start +  ehdr->e_phoff + (i * ehdr->e_phentsize);   
        func(entry,arg);
    }
    return 0;
}

int main(int argc, char ** argv){
    if(argc < 2){ // might need to change to <2
        printf("At least one command line argument required");
        exit(1);
    }
    int fildes;
    char* fileName = argv[1];
    if((fildes = open(fileName, O_RDWR)) < 0) {
      perror("Open failed");
      exit(1);
    }
    if(fstat(fildes, &fildes_stat) != 0 ) {
      perror("Could not retreive file status");
      exit(1);
    }
    if ((map_start = mmap(0, fildes_stat.st_size, PROT_READ | PROT_WRITE , MAP_SHARED, fildes, 0)) == MAP_FAILED ) {
      perror("Failed to load into memory");
      exit(1);
    }
    if(Currentfildes!=-1)
        close(Currentfildes);
    Currentfildes=fildes;
	strcpy((char*)&currOpenFile,(char*)fileName);
    ehdr = (Elf32_Ehdr *) map_start;
    printf("Type  Offset   VirtAddr   PhysAddr   FileSiz   MemSiz    Flg  Align\n");
    foreach_phdr(map_start, load_phdr, Currentfildes);
    startup(argc-1, argv+1, (void *)(ehdr->e_entry));
    return 0;
}