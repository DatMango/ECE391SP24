// Create struct for PDE, fill in (similar to IDT format)
// Do not need to load in x86_desc.S as it will automatically allocate space for us

// Use IA-32 Developer's Manual for paging + OSDEV
// #ifndef PAGING_H
// #define PAGING_H
#define ENTRIES     1024 
#define FOURKB      4096
#include "types.h"

#ifndef ASM
/* page directory entry */
// two structs to handle 4KB and 4MB which are slightly different but are unioned together, the bits are set 
// according to the ISA manual 

typedef union pde_desc_t {
    struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t write_through : 1;
        uint32_t cache_disabled : 1;
        uint32_t accessed : 1;
        uint32_t reserved : 1;
        uint32_t page_size : 1;
        uint32_t global_page : 1;
        uint32_t avail : 3;
        uint32_t pt_base_addr : 20;

    } four_kb __attribute__ ((packed));

    struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t write_through : 1;
        uint32_t cache_disabled : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t page_size : 1;
        uint32_t global_page : 1;
        uint32_t avail : 3; 
        uint32_t page_table_attr_index : 1;
        uint32_t reserved : 9;
        uint32_t pt_base_addr : 10;
    } four_mb __attribute__ ((packed));

} pde_desc_t;

// the 4KB, video memory, uses page table entry and this has the same setup as 4KB pde and is also set according
// to the ISA manual 
typedef union pte_desc_t {
    struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t write_through : 1;
        uint32_t cache_disabled : 1;
        uint32_t accessed : 1;
        uint32_t reserved : 1;
        uint32_t page_size : 1;
        uint32_t global_page : 1;
        uint32_t avail : 3;
        uint32_t pt_base_addr : 20;
    } __attribute__ ((packed));
} pte_desc_t;

// 1kb is 2^10 -- 1024 so 4kb is 4096, the FOURKB is set to this number 

pde_desc_t page_directory[ENTRIES] __attribute__((aligned(FOURKB))); 
pte_desc_t page_table_entries[ENTRIES] __attribute__((aligned(FOURKB))); // Entries is set to 1024 bc 2^10
pte_desc_t syscall_page_table_entries[ENTRIES] __attribute__((aligned(FOURKB))); // Entries is set to 1024 bc 2^10

// page enable just passes in the parameter of pointer type page_directory so the 
// asm code can load the address correctly 
extern void page_enable(uint32_t* page_directory); 

// extern void flush_TLB();

#endif
