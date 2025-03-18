#include "paging_modifier.h"
#include "lib.h"
#include "paging.h"
#include "types.h"


// in this checkpoint virtual address and physical same -- one-to-one mapping 

/* 
 * 
 *   DESCRIPTION: Initializes paging for 4KB and 4MB, that is video memory and 
 *                  kernel respectively
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: sets all the present bits and any other relevant bits 
 *                  and sets unused bits to 0 appropriately 
 * 
 */

void paging_init(){ 
    // set everything unused to 0 which is later overwritten to the correct present bits for the chunks of 4MB used 
    // this loops through the page directory itself to set those to 0
    int page_directory_index; 
    for(page_directory_index = 0; page_directory_index < ENTRIES; page_directory_index++){
        page_directory[page_directory_index].four_kb.pt_base_addr = page_directory_index;   
        page_directory[page_directory_index].four_kb.avail = 0;                    
        page_directory[page_directory_index].four_kb.global_page = 0; 
        page_directory[page_directory_index].four_kb.page_size = 0;                 
        page_directory[page_directory_index].four_kb.reserved = 0;                    
        page_directory[page_directory_index].four_kb.accessed = 0;                 
        page_directory[page_directory_index].four_kb.cache_disabled = 0; 
        page_directory[page_directory_index].four_kb.write_through = 0;
        page_directory[page_directory_index].four_kb.user_supervisor = 0;          
        page_directory[page_directory_index].four_kb.read_write = 0;               
        page_directory[page_directory_index].four_kb.present = 0;    
    }
    
    // video bits set
    // here page directory base address points to page table since its 4KB 
    // read and write bit is set to 1 to allow for both read and write 
    // present bit for first chunk, that is, where video memory is location, is set to 1 
    page_directory[0].four_kb.pt_base_addr = (uint32_t)page_table_entries >> shiftbytwelve; // to get 20 bits 
    page_directory[0].four_kb.avail = 0;
    page_directory[0].four_kb.global_page = 0; 
    page_directory[0].four_kb.page_size = 0; 
    page_directory[0].four_kb.reserved = 0; 
    page_directory[0].four_kb.accessed = 0; 
    page_directory[0].four_kb.cache_disabled = 0; 
    page_directory[0].four_kb.write_through = 0;
    page_directory[0].four_kb.user_supervisor = 1; 
    page_directory[0].four_kb.read_write = 1;  
    page_directory[0].four_kb.present = 1; 

    // loops through page table and sets unused bits to 0, but when it matches the next 10 bits of virtual address it sets the 
    // present bit to be 1 to allow for mapping 
    int page_table_index; 
    for(page_table_index = 0; page_table_index < ENTRIES; page_table_index++){
        if(page_table_index == ((VIDEO & MASK_NUMBER) >> shiftbytwelve)){                  // to get next 10 bits 
            page_table_entries[page_table_index].pt_base_addr = VIDEO >> shiftbytwelve;   // to get 20 bits
            page_table_entries[page_table_index].avail = 0;
            page_table_entries[page_table_index].global_page = 0; 
            page_table_entries[page_table_index].page_size = 0; 
            page_table_entries[page_table_index].reserved = 0; 
            page_table_entries[page_table_index].accessed = 0;
            page_table_entries[page_table_index].cache_disabled = 0; 
            page_table_entries[page_table_index].write_through = 0;
            page_table_entries[page_table_index].user_supervisor = 1; 
            page_table_entries[page_table_index].read_write = 1;  
            page_table_entries[page_table_index].present = 1; 
        } 
        else if(page_table_index == ((TERMINAL1_VIDEO & MASK_NUMBER) >> shiftbytwelve)){                  // to get next 10 bits 
            page_table_entries[page_table_index].pt_base_addr = TERMINAL1_VIDEO >> shiftbytwelve;   // to get 20 bits
            page_table_entries[page_table_index].avail = 0;
            page_table_entries[page_table_index].global_page = 0; 
            page_table_entries[page_table_index].page_size = 0; 
            page_table_entries[page_table_index].reserved = 0; 
            page_table_entries[page_table_index].accessed = 0;
            page_table_entries[page_table_index].cache_disabled = 0; 
            page_table_entries[page_table_index].write_through = 0;
            page_table_entries[page_table_index].user_supervisor = 1; 
            page_table_entries[page_table_index].read_write = 1;  
            page_table_entries[page_table_index].present = 1; 
        }
        else if(page_table_index == ((TERMINAL2_VIDEO & MASK_NUMBER) >> shiftbytwelve)){                  // to get next 10 bits 
            page_table_entries[page_table_index].pt_base_addr = TERMINAL2_VIDEO >> shiftbytwelve;   // to get 20 bits
            page_table_entries[page_table_index].avail = 0;
            page_table_entries[page_table_index].global_page = 0; 
            page_table_entries[page_table_index].page_size = 0; 
            page_table_entries[page_table_index].reserved = 0; 
            page_table_entries[page_table_index].accessed = 0;
            page_table_entries[page_table_index].cache_disabled = 0; 
            page_table_entries[page_table_index].write_through = 0;
            page_table_entries[page_table_index].user_supervisor = 1; 
            page_table_entries[page_table_index].read_write = 1;  
            page_table_entries[page_table_index].present = 1; 
        }
        else if(page_table_index == ((TERMINAL3_VIDEO & MASK_NUMBER) >> shiftbytwelve)){                  // to get next 10 bits 
            page_table_entries[page_table_index].pt_base_addr = TERMINAL3_VIDEO >> shiftbytwelve;   // to get 20 bits
            page_table_entries[page_table_index].avail = 0;
            page_table_entries[page_table_index].global_page = 0; 
            page_table_entries[page_table_index].page_size = 0; 
            page_table_entries[page_table_index].reserved = 0; 
            page_table_entries[page_table_index].accessed = 0;
            page_table_entries[page_table_index].cache_disabled = 0; 
            page_table_entries[page_table_index].write_through = 0;
            page_table_entries[page_table_index].user_supervisor = 1; 
            page_table_entries[page_table_index].read_write = 1;  
            page_table_entries[page_table_index].present = 1; 
        }
        else if(page_table_index == ((0xBC000 & MASK_NUMBER) >> shiftbytwelve)){                  // to get next 10 bits 
            page_table_entries[page_table_index].pt_base_addr = VIDEO >> shiftbytwelve;   // to get 20 bits
            page_table_entries[page_table_index].avail = 0;
            page_table_entries[page_table_index].global_page = 0; 
            page_table_entries[page_table_index].page_size = 0; 
            page_table_entries[page_table_index].reserved = 0; 
            page_table_entries[page_table_index].accessed = 0;
            page_table_entries[page_table_index].cache_disabled = 0; 
            page_table_entries[page_table_index].write_through = 0;
            page_table_entries[page_table_index].user_supervisor = 1; 
            page_table_entries[page_table_index].read_write = 1;  
            page_table_entries[page_table_index].present = 1; 
        } 
        else{
            page_table_entries[page_table_index].pt_base_addr = page_table_index; 
            page_table_entries[page_table_index].avail = 0;
            page_table_entries[page_table_index].global_page = 0; 
            page_table_entries[page_table_index].page_size = 0; 
            page_table_entries[page_table_index].reserved = 0; 
            page_table_entries[page_table_index].accessed = 0;
            page_table_entries[page_table_index].cache_disabled = 0; 
            page_table_entries[page_table_index].write_through = 0;
            page_table_entries[page_table_index].user_supervisor = 0; 
            page_table_entries[page_table_index].read_write = 0;  
            page_table_entries[page_table_index].present = 0; 
        }
    } 
    
    // kernel bits set 
    page_directory[1].four_mb.pt_base_addr = (KERNEL_ADDR >> shiftbytwentytwo);  // to get 10 bits 
    page_directory[1].four_mb.reserved = 0; 
    page_directory[1].four_mb.page_table_attr_index = 0;    
    page_directory[1].four_mb.avail = 0;                    
    page_directory[1].four_mb.global_page = 0; 
    page_directory[1].four_mb.page_size = 1;                // 1 indicates 4 MBytes 
    page_directory[1].four_mb.dirty = 0;                    // 0 when initially loaded 
    page_directory[1].four_mb.accessed = 0;                 // 0 when initially loaded 
    page_directory[1].four_mb.cache_disabled = 0; 
    page_directory[1].four_mb.write_through = 0;
    page_directory[1].four_mb.user_supervisor = 0;          // kernel so 0
    page_directory[1].four_mb.read_write = 1;               // want both read and write
    page_directory[1].four_mb.present = 1;                  // mapping is present 

    // calls page_enable 
    page_enable((uint32_t*)page_directory); 

}
