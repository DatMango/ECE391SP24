#include "filesystem.h"


// static int offset_read_data = 0; // to keep track of offset for read_data, so will be incremented by bytes read each time 
// static int offset_directory = 0; // to keep track of directory offset every time directory_read is called               

/* 
 * 
 *   DESCRIPTION: This function finds the matching filename and copies over the corresponding dentry struct 
 *                  to the dentry passed in. 
 *   INPUTS: fname - filename 
 *           dentry - dentry to be copied to 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success, -1 on failure 
 *   SIDE EFFECTS: mem copies the dentry struct   
 * 
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    // find out which dentry corresponds to the filename and return a pointer to the file descriptor (later cp)
    // null check 
    if(dentry == NULL){
        return -1;
    }

    int i;
    for(i = 0; i < NUM_DIR_ENTRIES; i++){
        // string compares to find matching filename from bootblock
        if(strncmp((int8_t*)fname, (int8_t*)bootblock->direct_entry[i].filename, FILENAME_SIZE) == 0){ 
            // mem copies dentry struct based on filename  
            memcpy(dentry, &(bootblock->direct_entry[i]), sizeof(dentry_t));                         
            return 0; 
        }
    }

    return -1; 
}

/* 
 * 
 *   DESCRIPTION: Similar to read_dentry_by_name, copies over the struct based on index 
 *                  instead of filename 
 *   INPUTS: index - index into bootblock 
 *           dentry - dentry to be copied to 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success, -1 on failure 
 *   SIDE EFFECTS: mem copies the dentry struct   
 * 
 */
// not inode number but the index into bootblock
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){                       
    if(index < 0 || index > NUM_DIR_ENTRIES || dentry == NULL){
        return -1; 
    } 

    // mem copies dentry struct based on index 
    memcpy(dentry, &(bootblock->direct_entry[index]), sizeof(dentry_t));               
    return 0; 
}


/* 
 * 
 *   DESCRIPTION: gets pointer to inode based on inode passed in and grabs block num
 *                  based on the pointer which is in then used to access the correct 
 *                  block. Mem copies data into buffer and returns bytes read
 *   INPUTS:  inode - inode num
 *            offset - where you want to start reading from 
 *            buf - where you copy into 
 *            length - number of bytes you want to read
 *   OUTPUTS: none
 *   RETURN VALUE: returns nbytes 
 *   SIDE EFFECTS: mem copies data block into buffer and updates bytes read    
 * 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // null check, offset validity check, inode validity check and length validity check
    cli();
    if(buf == NULL || length < 0 || offset < 0 || inode < 0 || inode > bootblock->num_inodes){
        return -1;
    }
    if(length == 0){
        return length;
    } 

    // grabs inode pointer from bootblock address and inode num passed in 
    inode_t* inode_ptr = (inode_t*)(bootblock + inode + 1); 
    int i = 0; 
    int temp_length = length; 
    int32_t nbytes_read = 0; 
    int32_t block_num;

    uint32_t curr_block_offset = offset;
    for(i = offset; i < offset + length; ){
        // case for when length is greater than 4kB 
        if(temp_length >= FOURKB){
            curr_block_offset = i % FOURKB;
            block_num = inode_ptr->blocks[i/FOURKB];                                                    // grabs block num based on inode pointer and finds block to read 
            uint8_t* data_block = (uint8_t*)(bootblock + (bootblock->num_inodes + 1 + block_num));      // grabs pointer to relevant data block
            memcpy(buf, data_block + curr_block_offset, FOURKB - curr_block_offset); 
            temp_length -= FOURKB - curr_block_offset;                                                                      // updates bytes read 
            nbytes_read += FOURKB - curr_block_offset;  
            buf = buf + FOURKB - curr_block_offset;                                                                         // updates buffer location because reading 4kB each time
            curr_block_offset = 0;
            i += FOURKB - curr_block_offset;
        }
        else {
            curr_block_offset = i % FOURKB;
            // takes care of less than 4kB 
            block_num = inode_ptr->blocks[i/FOURKB];
            uint8_t* data_block_ptr = (uint8_t*)(bootblock + (bootblock->num_inodes+1+block_num)); 
            memcpy(buf, data_block_ptr + curr_block_offset, temp_length);
            nbytes_read += temp_length;                                                    // mem copies into buffer the temp length that is updated based on bytes read
            i += temp_length;                                                             // updates bytes read 
            temp_length -= temp_length;
        }
    }
    sti();

    // updates offset based on bytes read and returns bytes read 
    // offset_read_data += nbytes_read; 
    return nbytes_read; 
}

/* 
 * 
 *   DESCRIPTION: resets offset and calls read_dentry_by_name with passed 
 *                  in filename so dentry has right struct 
 *   INPUTS:  filename - filename to be opened 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success, -1 on failure  
 *   SIDE EFFECTS: resets offset and calls read_dentry_by_name which 
 *                  finds the right dentry struct corresponding to filename   
 * 
 */
int32_t file_open(const uint8_t* filename){
    // initialize any temp structures (later cp?)
    int ret;
    ret = read_dentry_by_name(filename, &dentry_glo);

    return ret;
}

/* 
 * 
 *   DESCRIPTION: reads data based on inode number from correctly matched dentry struct, 
 *                  will be modified later to include file descriptor. For now, works 
 *                  with fd as 0   
 *   INPUTS:  fd - file descriptor passed in
 *            buf - buf to be passed into read_data, where read bytes/data is copied into  
 *            nbytes - bytes to be read  
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success, -1 on failure 
 *   SIDE EFFECTS: calls read_data  
 * 
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    // Find file size
    int file_size = (uint32_t)cur_pcb->fd_array[fd].file_size;

    // If file size is less than nbytes (buffer) size, use the file size instead
    if(file_size - (uint32_t)cur_pcb->fd_array[fd].file_pos < nbytes){
        nbytes = file_size - (uint32_t)cur_pcb->fd_array[fd].file_pos;
    }

    int32_t read_bytes = read_data(cur_pcb->fd_array[fd].inode, (uint32_t)cur_pcb->fd_array[fd].file_pos, (uint8_t*)buf, nbytes);
    cur_pcb->fd_array[fd].file_pos += read_bytes;
    return read_bytes;
    //return 0;
}

/* 
 * 
 *   DESCRIPTION: read-only filesystem so no file_write   
 *   INPUTS:  fd - file descriptor passed in (0 for now, will be modified at later cp)
 *            buf - buf with bytes to be written   
 *            nbytes - bytes to be written   
 *   OUTPUTS: none
 *   RETURN VALUE: -1 because read-only filesystem 
 *   SIDE EFFECTS: none 
 * 
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){ // read-only file system 
    return -1; 
}

/* 
 * 
 *   DESCRIPTION: closes file   
 *   INPUTS:  fd - file descriptor (0 for now, will be modified at later cp)
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0  
 *   SIDE EFFECTS: none 
 * 
 */
int32_t file_close(int32_t fd){
    // undo what you did in the open (later cp?)
    
    cur_pcb->fd_array[fd].file_pos = 0;
    return 0; 
}


/* 
 * 
 *   DESCRIPTION: resets offset and calls read_dentry_by_name with passed 
 *                  in filename so dentry has right struct (directory open)
 *   INPUTS:  filename - filename to be opened 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success, -1 on failure  
 *   SIDE EFFECTS: resets offset and calls read_dentry_by_name which 
 *                  finds the right dentry struct corresponding to filename   
 * 
 */
int32_t directory_open(const uint8_t* filename){            // same code should work even though it is just "." 
    // initialize any temp structs (later cp?)
    
    return read_dentry_by_name(filename, &dentry_dir); 
}

/* 
 * 
 *   DESCRIPTION: copies filename and reads one file at a time 
 *   INPUTS:  fd - file descriptor passed in (0 for now, will be modified at later cp)
 *            buf - buf to be passed into read_data, where read bytes/data is copied into  
 *            nbytes - bytes to be read  
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success, -1 on failure  
 *   SIDE EFFECTS: increments directory offset and copies filename   
 * 
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    // return only 1 file at a time 
    if(fd == 0 || fd == 1){
        return -1; 
    }
    
    if(cur_pcb->fd_array[fd].file_pos == bootblock->num_dentry){
        cur_pcb->fd_array[fd].file_pos = 0; 
        return 0; 
    }
    // strcpy((int8_t*)buf, (int8_t*)(bootblock->direct_entry[offset_directory].filename)); 
    strncpy((int8_t*)buf, (int8_t*)(bootblock->direct_entry[cur_pcb->fd_array[fd].file_pos].filename), nbytes); 
    //offset_directory++; 
    cur_pcb->fd_array[fd].file_pos++;
    // If file size is less than nbytes (buffer) size, use the file size instead

    return nbytes; // change return value to how many bytes read
}

/* 
 * 
 *   DESCRIPTION: read-only so no directory_write   
 *   INPUTS:  fd - file descriptor passed in (0 for now, will be modified at later cp)
 *            buf - buf with bytes to be written   
 *            nbytes - bytes to be written   
 *   OUTPUTS: none
 *   RETURN VALUE: -1 because read-only  
 *   SIDE EFFECTS: none 
 * 
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1; 
}

/* 
 * 
 *   DESCRIPTION: closes directory    
 *   INPUTS:  fd - file descriptor (0 for now, will be modified at later cp)
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0  
 *   SIDE EFFECTS: none 
 * 
 */
int32_t directory_close(int32_t fd){
    // undo what you did in the open (later cp?)
    return 0; 
}
