
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

#void* jump_table[4];
# Jump table for dispatcher (mp1_ioctl)
jumptable:
        .long   mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret


# void mp1_rtc_tasklet (unsigned long);
#
# Interface: Stack-based arguments
#    Inputs: unsigned long - Parameter is not used
#   Outputs: No register output. Outputs opposite character to video memory.
#            Whenever RTC interrupt is generated, call (rtc) tasklet.  Goes through linked list,
#            examines each blink struct for putting opposite on/off character and outputs to
#            video memory using mp1_poke().
# Registers: Clobbers EAX; (Stack frame - EBP, EBX, ESI, EDI, ECX); ECX is pushed to stack frame
#            to prevent clobbering it
mp1_rtc_tasklet:
        /* Create the stack frame */
        pushl   %ebp                    /* Save the old base pointer value */
        movl    %esp, %ebp              /* Set new base pointer value */
        pushl   %ebx                    /* protect callee-saved registers */
        pushl   %esi
        pushl   %edi
        pushl   %ecx
        
        /* Link to input interface */
        movl    mp1_list_head, %edi     /* List head to iterate through, original parameter unused */
        cmpl    $0, %edi                /* Check for if head is null pointer*/
        je      tasklet_tear_down

        /* Code */
        xorl    %ebx, %ebx              
        movw    COUNTDOWN(%edi), %bx    /* Check countdown is already 0, jump to examine status */
        cmpw	$0, %bx
        je      tasklet_examine_status

tasklet_loop:
        subw    $1, COUNTDOWN(%edi)     /* Decrement countdown */
        xorl    %esi, %esi
        movw    COUNTDOWN(%edi), %si    /* When countdown 0, examine status. Else, move to next node */
        cmpw    $0, %si
        je      tasklet_examine_status
        jmp     tasklet_next_node

tasklet_examine_status:
        xorl    %esi, %esi              /* Check status field for on(1)/off(0), jump based on 0 or 1 */
        movw    STATUS(%edi), %si       
        cmpw    $0, %si                 
        je      tasklet_when_zero
        ja      tasklet_when_one

tasklet_when_zero:
        movw    $1, STATUS(%edi)        /* Change opposite status; use on_char */
        movb    ON_CHAR(%edi), %cl      /* %cl - Byte wish to write for mp1_poke */
        xorl    %eax, %eax                      

        cmpw    $0, LOCATION(%edi)              /* Check bounds of 0 and 80*25-1, jump when incorrect bounds */
        jb      tasklet_tear_down
        cmpw    $80*25-1, LOCATION(%edi)        /* 80*25-1 is upper bound */
        ja      tasklet_tear_down

        movw    LOCATION(%edi), %ax      /* %eax - Offset from start of video memory wish to write to */
        shll    $1, %eax
        call	mp1_poke                 /* mp1_poke(void) */
        
        movw	ON_LENGTH(%edi), %cx     /* Copy on_length to countdown */
        movw    %cx, COUNTDOWN(%edi)
        jmp     tasklet_next_node
        
tasklet_when_one:
        movw    $0, STATUS(%edi)         /* Change opposite status; use off_char */
        movb    OFF_CHAR(%edi), %cl      /* %cl - Byte wish to write for mp1_poke */
        xorl    %eax, %eax                      

        cmpw    $0, LOCATION(%edi)              /* Check bounds of 0 and 80*25-1, jump when incorrect bounds */
        jb      tasklet_tear_down
        cmpw    $80*25-1, LOCATION(%edi)        /* 80*25-1 is upper bound */
        ja      tasklet_tear_down

        movw    LOCATION(%edi), %ax      /* %eax - Offset from start of video memory wish to write to */
        shll    $1, %eax
        call	mp1_poke                 /* mp1_poke(void) */

        movw    OFF_LENGTH(%edi), %cx    /* Copy off_length to countdown */
        movw    %cx, COUNTDOWN(%edi)
        jmp     tasklet_next_node

tasklet_next_node:
        cmpl    $0, NEXT(%edi)           /* Check if next node is null, jump when null; else move to next node */
        je      tasklet_tear_down
        movl    NEXT(%edi), %edi
        jmp     tasklet_loop

tasklet_tear_down:
        /* Tear Down */
        popl    %ecx                     /* Pop in reverse order, ecx was pushed and popped to not be clobbered*/
        popl    %edi
        popl    %esi
        popl    %ebx
        leave
	ret


# int mp1_ioctl (unsigned long arg, unsigned long cmd);
#
# Interface: Stack-based arguments
#    Inputs: unsigned long arg - Parameter is not used here
#            unsigned long cmd - Used to determine what function to jump to
#   Outputs: Returns -1 in %eax if cmd is invalid.
#            Is a "dispatcher" function, cmd jumps based on cmd value (uses jump table).
#            0 - add; 1 - remove; 2 - find; 3 - sync
# Registers: Clobbers EAX
mp1_ioctl:
        movl    8(%esp), %eax           /* cmd */
        cmpl    $0, %eax                /* Checking cmd to be 0-3 */
        jb      ret_neg_1
        cmpl    $3, %eax                /* Upper range (checking for not greater than 3)*/
        ja      ret_neg_1
        jmp     *jumptable(,%eax,4)     /* Jump based on EAX; dispatch */

ret_neg_1:
        movl    $-1, %eax               /* Returns -1 when cmd is anything other than 0-3*/
	ret


# int mp1_ioctl_add (unsigned long arg);
#
# Interface: Stack-based arguments
#    Inputs: unsigned long arg - user level pointer to mp1_blink_struct structure
#   Outputs: Returns 0 in %eax when function is successful at copying, returns -1 if copy has failed.
#            Allocates memory, copy blink structure from user space to allocated memory. Sets countdown to equal to
#            on_length and set status to 1. Calls poke to display character. (RTC will turn location "off".)
#            
# Registers: Clobbers EAX, ECX
mp1_ioctl_add:
        /* Create the stack frame */
        pushl   %ebp                    /* Save the old base pointer value */
        movl    %esp, %ebp              /* Set new base pointer value */
        pushl   %ebx                    /* protect callee-saved registers - non-volatile */
        pushl   %esi
        pushl   %edi

        /* Link to input interface */
        movl    8(%ebp), %edi           /* arg */
        cmpl    $0, %edi                /* Check if arg is valid */
        je	add_error

        /* Code */
        pushl   $STRUCT_SIZE            /* size */
        call    mp1_malloc              /* mp1_malloc(unsigned long size) */
        addl    $4, %esp
        movl    %eax, %esi              /* Store return address to newly allocated memory*/
        cmpl    $0, %eax                /* Check for memory allocation error, jump when error */
        je	add_error
        
        pushl   $STRUCT_SIZE            /* n */
        pushl   %edi                    /* from */
        pushl   %esi                    /* to */
        call    mp1_copy_from_user      /* mp1_copy_from_user (void *to, const void *from, unsigned long n) */
        
        popl    %esi                    /* Regain %esi by popping */
        addl    $8, %esp
        cmpl	$0, %eax                /* Check for bytes not copied, jump when failed copy */
        jne	add_failed_copy       

        movw    ON_LENGTH(%esi), %cx    /* on_length -> countdown */
        movw    %cx, COUNTDOWN(%esi)
        movw    $1, STATUS(%esi)        /* 1 -> status */

        movl    mp1_list_head, %ecx     /* Move list head to become next node, and change head */
        movl    %ecx, NEXT(%esi)
        movl    %esi, mp1_list_head

        cmpw    $0, LOCATION(%esi)              /* Check bounds of 0 and 80*25-1, jump when incorrect bounds */
        jb      add_failed_copy
        cmpw    $80*25-1, LOCATION(%esi)        /* Upper bound 80*25-1 */
        ja      add_failed_copy

        movb    ON_CHAR(%esi), %cl      /* %cl - Byte wish to write - on period */
        xorl    %eax, %eax
        movw    LOCATION(%esi), %ax     /* %eax - Offset from start of video memory wish to write to */
        shll    $1, %eax
        call	mp1_poke                /* mp1_poke(void) */

        movl    $0, %eax                /* Successful add, ret 0 */
        jmp	add_tear_down

add_failed_copy:
        pushl   %esi                    /* Free memory (memory allocated must be freed, unless malloc returns null)*/
        call	mp1_free
        addl	$4, %esp
        jmp	add_error
        
add_error:
        movl	$-1, %eax               /* Malloc, copy, or location bounds error, ret -1 */
        jmp	add_tear_down
        
add_tear_down:
        /* Tear Down */
        popl    %edi                    /* Standard tear down */
        popl    %esi
        popl    %ebx
        leave
	ret
        

# int mp1_ioctl_remove(unsigned long arg)
#
# Interface: Stack-based arguments
#    Inputs: unsigned long arg - an integer location
#   Outputs: Returns 0 in %eax if element from linked list matches the location field and successful removal, else returns -1.
#            Traverses linked list to search for element that matches location field from argument, removes from linked list.
# Registers: Clobbers EAX, ECX, 
mp1_ioctl_remove:
        /* Create the stack frame */
        pushl   %ebp                    /* Save the old base pointer value */
        movl    %esp, %ebp              /* Set new base pointer value */
        pushl   %ebx                    /* protect callee-saved registers - non-volatile */
        pushl   %esi
        pushl   %edi

        /* Link to input interface */
        movl    8(%ebp), %ecx           /* arg */

        /* Code*/
        cmpw    $80*25-1, %cx           /* Check if arg is above bounds of 80*25-1, if greater then follow error */
        ja      remove_error

        pushw   %cx                             /* location - Integer location input */
        call    mp1_search_linked_list          /* Use helper function, push integer location */
        addl    $2, %esp
        cmpl    $0, %eax                        /* If %eax 0, jump to error (no matching element)*/
        je      remove_error

        cmpl    %eax, mp1_list_head             /* Check if returned element location is the same as the list head */
        je      remove_replace_list_head
        jne     remove_node

remove_replace_list_head:
        movl    NEXT(%eax), %ebx        /* Store next address in %ebx (doesn't matter if next is null since will just transfer nulls) */
        movl    %ebx, mp1_list_head     /* Store next address in list head */
        pushl   %eax                    /* Free address */
        call    mp1_free
        addl    $4, %esp
        jmp     remove_success

remove_node:
        movl    NEXT(%eax), %ebx        /* Store next address in %ebx (will just move null to next value if null)*/
        movl    %ebx, NEXT(%ecx)        /* Store address in previous' NEXT location (%ecx is given as previous address from search linked list)*/
        pushl   %eax                    /* Free address */
        call    mp1_free
        addl    $4, %esp
        jmp     remove_success

remove_success:
        movl    $0, %eax                /* Return 0 if successful removal in %eax */
        jmp     remove_tear_down

remove_error:
        movl    $-1, %eax               /* Return 1 if successful removal in %eax */
        jmp     remove_tear_down

remove_tear_down:
        /* Tear Down */
        popl    %edi                    /* Standard tear down */
        popl    %esi
        popl    %ebx
        leave
	ret
        

# int mp1_ioctl_find(unsigned long arg)
#
# Interface: Stack-based arguments
#    Inputs: unsigned long arg - an integer location
#   Outputs: Returns 0 in %eax if element from linked list matches the location field, else returns -1.
#            Traverses linked list to search for element that matches location field from argument; copy from given location to user level.
# Registers: Clobbers EAX
mp1_ioctl_find:
        /* Create the stack frame */
        pushl   %ebp                    /* Save the old base pointer value */
        movl    %esp, %ebp              /* Set new base pointer value */
        pushl   %ebx                    /* protect callee-saved registers - non-volatile */
        pushl   %esi
        pushl   %edi

        /* Link to input interface */
        movl    8(%ebp), %edi           /* arg */
        cmpl    $0, %edi                /* Jump when arg isn't valid, else continue*/
        je	find_not_found
        
        pushl   $STRUCT_SIZE            /* size */
        call    mp1_malloc              /* mp1_malloc(unsigned long size) */
        addl    $4, %esp
        movl    %eax, %esi              /* Store returned address to newly added dynamic memory */
        cmpl    $0, %eax                /* Check for memory allocation error */
        je	find_not_found

        pushl   $STRUCT_SIZE            /* n */
        pushl   %edi                    /* from */
        pushl   %esi                    /* to */
        call    mp1_copy_from_user      /* mp1_copy_from_user (void *to, const void *from, unsigned long n) */
        popl    %esi
        addl    $8, %esp
        cmpl	$0, %eax                /* Check for bytes not copied, jump when failed copy */
        jne	find_failed_copy 

        cmpw    $0, LOCATION(%esi)              /* Check bounds of 0 and 80*25-1, jump when incorrect bounds */
        jb      find_failed_copy
        cmpw    $80*25-1, LOCATION(%esi)        /* Upper bound 80*25-1 */
        ja      find_failed_copy

        pushw   LOCATION(%esi)                  /* location - Integer location input */
        call    mp1_search_linked_list          /* Use helper function, push integer location */
        addl    $2, %esp
        cmpl    $0, %eax                        /* Check the returned location in %eax from linked list if is valid  */
        je      find_not_found

        pushl   $STRUCT_SIZE            /* n */
        pushl   %eax                    /* from */
        pushl   %edi                    /* to */
        call    mp1_copy_to_user        /* mp1_copy_to_user (void *to, const void *from, unsigned long n) */
        popl    %edi
        addl    $8, %esp
        
        pushl   %esi                    /* Free address */
        call    mp1_free
        addl    $4, %esp
        jmp     find_found

find_found:
        movl    $0, %eax                /* Return 0 for successful find and copy */
        jmp     find_tear_down

find_failed_copy:
        pushl   %esi                    /* Free address */
        call	mp1_free
        addl	$4, %esp
        jmp	find_not_found

find_not_found:
        movl    $-1, %eax               /* Return -1 if cannot find or copy */
        jmp     find_tear_down

find_tear_down:
        /* Tear Down */
        popl    %edi                    /* Standard tear down */
        popl    %esi
        popl    %ebx
        leave
	ret


# int mp1_ioctl_sync(unsigned long arg)
#
# Interface: Stack-based arguments
#    Inputs: unsigned long arg - two two-byte unsigned short integers; first integer in upper 16, second integer in lower 16
#   Outputs: Returns 0 in %eax for synchronization, returns -1 on failure.
#            Searches list for locations of elements that match with the integers. Copies timing information from first
#            element to second element (on_length, off_length, countdown, status). Then displays to video.
# Registers: Clobbers EAX, ECX
mp1_ioctl_sync:
        /* Create the stack frame */
        pushl   %ebp                    /* Save the old base pointer value */
        movl    %esp, %ebp              /* Set new base pointer value */
        pushl   %ebx                    /* protect callee-saved registers */
        pushl   %esi
        pushl   %edi

        /* Link to input interface */
        movw    8(%ebp), %bx            /* Lower 16 bits in %bx */
        movw    10(%ebp), %cx           /* Stored in %cx by right shifts */

        /* Code */
        cmpw    $80*25-1, %bx           /* Check upper bound of 80*25-1 */
        ja      sync_failure
        cmpw    $80*25-1, %cx           /* Check upper bound of 80*25-1 */
        ja      sync_failure

        pushw   %cx                     /* location - First integer */
        call    mp1_search_linked_list  /* Use helper function, push integer location */
        addl    $2, %esp
        cmpl    $0, %eax                /* Check if %eax has a valid element */
        je      sync_failure
        movl    %eax, %esi              /* Store element for first integer in %esi */

        pushw   %bx                     /* location - Second integer */
        call    mp1_search_linked_list  /* Use helper function, push integer location */
        addl    $2, %esp
        cmpl    $0, %eax                /* Check if %eax has a valid element */
        je      sync_failure
        movl    %eax, %edi              /* Store element for second integer in %esi */

        xorl    %ecx, %ecx              /* Copy timing informations (on_length, off_length, countdown, status) from first element to second element */
        movw    ON_LENGTH(%esi), %cx
        movw    %cx, ON_LENGTH(%edi)
        xorl    %ecx, %ecx
        movw    OFF_LENGTH(%esi), %cx
        movw    %cx, OFF_LENGTH(%edi)
        xorl    %ecx, %ecx
        movw    COUNTDOWN(%esi), %cx
        movw    %cx, COUNTDOWN(%edi)
        xorl    %ecx, %ecx
        movw    STATUS(%esi), %cx
        movw    %cx, STATUS(%edi)

sync_check_status:
        cmpw    $0, %cx                 /* Check if status is 0 or 1, jumps accordingly to display to video  */
        je      sync_when_zero
        ja      sync_when_one

sync_when_zero:
        movb    OFF_CHAR(%edi), %cl     /* %cl - Byte wish to write - on period */
        xorl    %eax, %eax              
        movw    LOCATION(%edi), %ax     /* %eax - Offset from start of video memory wish to write to */
        shll    $1, %eax
        call	mp1_poke                /* mp1_poke(void)*/
        jmp     sync_success

sync_when_one:
        movb    ON_CHAR(%edi), %cl      /* %cl - Byte wish to write - on period */
        xorl    %eax, %eax              
        movw    LOCATION(%edi), %ax     /* %eax - Offset from start of video memory wish to write to */
        shll    $1, %eax                
        call	mp1_poke                /* mp1_poke(void)*/
        jmp     sync_success

sync_success:
        movl    $0, %eax                /* Return 0 for successful sync */
        jmp     sync_tear_down

sync_failure:
        movl    $-1, %eax               /* Return -1 for unsuccessful sync */
        jmp     sync_tear_down

sync_tear_down:
        /* Tear Down */
        popl    %edi                    /* Standard tear down*/
        popl    %esi
        popl    %ebx
        leave
	ret


# int mp1_ioctl_sync(unsigned short arg)
# Helper function for remove, find, and sync 
#
# Interface: Stack-based arguments (instead of "not C-style")
#    Inputs: unsigned short arg - location that needs to be searched for
#   Outputs: Returns pointer to struct in %eax, returns 0 on failure.
#            Searches list for location of element that matches with the given integer location.
# Registers: Clobbers EAX, ECX
mp1_search_linked_list:
        /* Create the stack frame */
        pushl   %ebp                    /* Save the old base pointer value */
        movl    %esp, %ebp              /* Set new base pointer value */
        pushl   %ebx                    /* protect callee-saved registers */
        pushl   %esi
        pushl   %edi

        /* Link to input interface */
        movw    8(%ebp), %bx            /* location (arg) */
        movl    mp1_list_head, %edi     /* List head to start iterating from to find element */

        /* Code */
search_loop:
        cmpl    $0, %edi                /* Check if list head is null */
        je      search_not_found
        cmpw    %bx, LOCATION(%edi)     /* Check if location and element match to return */
        je      search_finish

        cmpl    $0, NEXT(%edi)          /* If next location is null, jump to not found */
        je      search_not_found
        movl    %edi, %ecx              /* Store current element location into %ecx (this will be returned if element is found) */
        movl    NEXT(%edi), %edi        /* Store next element into %edi and loop */
        jmp     search_loop

search_not_found:
        movl    $0, %eax                /* Returns 0 if location not found in linked list */
        jmp     search_tear_down

search_finish:
        movl    %edi, %eax              /* Returns matching pointer in %eax; %ecx holds previous pointer in return */
        jmp     search_tear_down

search_tear_down:
        /* Tear Down */
        popl    %edi                    /* Standard tear down */
        popl    %esi
        popl    %ebx
        leave
        ret


.end
