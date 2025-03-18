#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
// #include "filesystem.h"
#include "syscall_modifier.h"

#define PASS 1
#define FAIL 0


#define fivehundred 500
#define fourtythousand 40000
#define fishfilesize 36164
#define thirtyseventhousand 37000
#define eighthundred 800


/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");
// #define EXCEPTION_HANDLER(name)
	// printf("Exception: %s\n", name); 
static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

//raises divide by 0 exception
int div_zero(){
	TEST_HEADER;
	int a = 0;
	int b;
	b = 1 / a;
	return FAIL;
}

//force raise overflow exception
int overflow_test(){
	TEST_HEADER;
	asm volatile("int $4");
	return FAIL;
}

//force raise overflow exception
int paging_exception_idt_test(){
	TEST_HEADER;
	asm volatile("int $14");
	return FAIL;
}

int idt_garbage_value(){
	TEST_HEADER;
	asm volatile("int $55");
	return FAIL;
}

//changes the rate of interrupts to 1024 hz 
int rtc_test_1024hz(){
	TEST_HEADER;
	change_rtc_rate(1024); 
	// test_interrupts(); // Supposed to be in rtc handler
	return FAIL;
}

//changes rate of interrupts to 8192 hz
int rtc_test_8192hz(){
	TEST_HEADER;
	change_rtc_rate(8192); 
	return FAIL;
}

//slows down rate of interrupts to 2 hz
int rtc_test_2hz(){
	TEST_HEADER;
	change_rtc_rate(2); 
	return FAIL;
}

// changes rate of interrupt to 1 hz, out of bounds so should set to rate = 3
int rtc_test_out_bounds(){
	TEST_HEADER;
	change_rtc_rate(1);
	return FAIL;
}

//paging test, accesses accessible kernel memory
int page(){
	char* c;
	c = (char*)(0x400000);
	printf("hello\n");
	return PASS;
}

//paging test, tries to access NULL raises page fault
int null_page(){
	char* c;
	c = (char*)(0x0);
	printf("%s", c);
	return FAIL;
}

//paging test, tries to access out of boundary memory raises page fault
int out_page(){
	char* c;
	c = (char*)(0x800000);
	printf("%s", c);
	return FAIL;
}

//paging test, tries to access out of boundary memory raises page fault
int out_pageF(){
	char* c;
	c = (char*)(0xFFFFFF);
	printf("%s", c);
	return FAIL;
}



/* Checkpoint 2 tests */
// RTC test incremental
const int rtc_buffer_inc[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
int rtc_test_increment(){
	TEST_HEADER;
	int i;
	int j;
	for(i = 0; i < 10; i++){
		printf("\n");	
		for(j = 0; j < (i+1)*10; j++){
			rtc_write(0, (void*)(rtc_buffer_inc + i), 4);
			rtc_read(0, (void*)(rtc_buffer_inc + i), 4);
		}
	} 
	return PASS;
}

// Reads and writes to the terminal test
int terminal_rw(){
	TEST_HEADER;
	char* terminal_buffer[NUM_COLS * NUM_ROWS];
	uint32_t read_bytes = 1;
	uint32_t write_bytes ;
	read_bytes = terminal_read(0, terminal_buffer, 1);
	if(read_bytes == 0){
		return FAIL;
	}
	write_bytes = terminal_write(0, terminal_buffer, read_bytes);
	if(write_bytes == -1){
		return FAIL;
	}
	return PASS;
}

// Opens file
int file_open_test(){
	TEST_HEADER;
	return (file_open((uint8_t*)("frame0.txt")) == 0);
}

// frame0.txt read test
int read_test_case_frame0(){
	TEST_HEADER;
	clear();
	int file_close_val;
	int file_write_val;
	uint8_t buffer[buffer_size]; 
	char buffer0[buffer_size];
	file_open((uint8_t*)("frame0.txt"));
	read_data(dentry_glo.inode_num, (uint32_t)180, buffer, fivehundred);	
	int i; 
	for(i = 0; i < 300; i++){
		// putc(buffer[i]);
		// printf("CURRENT I VALUE : %d", i);
		printf("%c", buffer[i]); 
	}
	file_write_val = file_write(0,buffer0,buffer_size);
	file_close_val = file_close(0);
	printf("\n");
	if(file_write_val == -1 && file_close_val == 0){
		return PASS;
	}
	return FAIL; 
}

// frame1.txt read test
int read_test_case_frame1(){
	TEST_HEADER;
	clear();
	char buffer[buffer_size]; 
	file_open((uint8_t*)("frame1.txt"));
	file_read(0, buffer, fivehundred);	
	int i; 
	for(i = 0; i < fivehundred; i++){
		printf("%c", buffer[i]); 
	}
	printf("\n");
	return PASS; 
}

// verylargetextwithverylongname.txt read test
int read_test_case_longname(){
	TEST_HEADER;
	clear();
	char buffer[fourtythousand]; 
	file_open((uint8_t*)("verylargetextwithverylongname.txt"));
	file_read(0, buffer, 200);	
	int i; 
	for(i = 0; i < 200; i++){
		if(buffer[i] == NULL || buffer[i] == '\n'){
			continue;
		}
		printf("%c", buffer[i]); 
	}
	printf("\n");
	return PASS; 	
}

// verylargetextwithverylongname.tx read test
int read_test_case_longname_missing_last_letter(){
	TEST_HEADER;
	clear();
	char buffer[fourtythousand]; 
	file_open((uint8_t*)("verylargetextwithverylongname.tx"));
	file_read(0, buffer, fourtythousand-1);	
	int i; 
	for(i = 0; i < fourtythousand - 2; i++){
		if(buffer[i] == NULL){
			continue;
		}
		printf("%c", buffer[i]); 
	}
	printf("\n");
	return PASS; 	
}

// Read dentry using an index test
int read_dentry_by_index_test(){
	TEST_HEADER;
	clear();
	dentry_t dentry_test;
	return (read_dentry_by_index(1, &dentry_test) == 0);
}

// Testing bounds of dentry
int read_dentry_by_index_test_bounds(){
	TEST_HEADER;
	clear();
	dentry_t dentry_test;
	return (read_dentry_by_index(-1, &dentry_test) == -1);
}

// GREP read test case prints
int read_test_case_GREP(){
	TEST_HEADER;
	clear();
	char buffer[6500]; 
	file_open((uint8_t*)("grep"));
	file_read(0, buffer, 6200);	
	int i; 
	for(i = 0; i < 6149; i++){
		if(buffer[i] == NULL){
			continue;
		}
		printf("%c", buffer[i]); 
	}
	printf("\n");
	return PASS; 
}

// fish read test case prints
int read_test_case_fish(){
	TEST_HEADER;
	clear();
	char buffer[fourtythousand]; 
	file_open((uint8_t*)("fish"));
	file_read(0, buffer, 5400);	
	int i; 
	for(i = 0; i < 10; i++){
		if(buffer[i] == NULL){
			continue;
		}
		printf("%c", buffer[i]); 
	}
	printf("\n");
	return PASS; 	
}

// ls read test case prints
int read_test_case_LS(){
	TEST_HEADER;
	clear();
	char buffer[5400]; 
	file_open((uint8_t*)("ls"));
	file_read(0, buffer, 5400);	
	int i; 
	for(i = 0; i < 5349; i++){ // Change to see beginning
		if(buffer[i] == NULL){
			continue;
		}
		printf("%c", buffer[i]); 
	}
	printf("\n");
	return PASS; 
}

// Prints directory
int directory_test_case(){
	TEST_HEADER;
	clear();
	directory_open((uint8_t*)("."));
	int directory_close_val;
	int directory_write_val; 
	int i;
	int j;
	int spaces_left = 0;
	int file_size;
	int32_t file_type;
	char buffer[FILENAME_SIZE];
	char buffer0[FILENAME_SIZE]; 
	for(j = 0; j < bootblock->num_dentry; j++){
		directory_read(2, buffer, FILENAME_SIZE);
		printf("File_Name:");
		for(i = 0; i < FILENAME_SIZE; i++){
			if(buffer[i] == NULL){
				spaces_left = FILENAME_SIZE - i;
				break;
			}
			printf("%c", buffer[i]);
			spaces_left = 0;
		}
		for(i = 0; i < spaces_left; i++){
			printf(" ");
		}
		printf(", ");
		file_type = bootblock->direct_entry[j].filetype;
		inode_t* inode_ptr = (inode_t*)(bootblock + bootblock->direct_entry[j].inode_num + 1);
		file_size = inode_ptr->length_B;
		printf("File_type:%d, ", file_type);
		printf("File_size:%d", file_size);
		printf("\n");
	}
	directory_write_val = directory_write(0, buffer0, FILENAME_SIZE);
	directory_close_val = directory_close(0);

	if(directory_write_val == -1 && directory_close_val == 0){
		return PASS; 
	}
	return FAIL;
}

// read_data() buffer null check (works for all functions using read_data())
int test_read_data_bounds(){
	TEST_HEADER;
	unsigned char null_buffer[10];
	inode_t* inode_ptr = (inode_t*)(bootblock + bootblock->direct_entry[1].inode_num + 1);
	return (read_data((uint32_t)inode_ptr, 24, null_buffer, 4) == -1);
}


/* Checkpoint 3 tests */
int syscall_test_open_read(){
	TEST_HEADER;
	int fd;
	int read_ret;
	char buffer[1000];
	fd = open((uint8_t*)("frame0.txt"));
	read_ret = read(fd, buffer, 1000);
	close(fd);
	if(read_ret == -1){
		return FAIL;
	}
	return PASS;
}

int syscall_test_write(){
	TEST_HEADER;
	// int i;
	int fd;
	// int read_ret;
	// char buffer[200];
	char* terminal_buffer[NUM_COLS * NUM_ROWS];
	uint32_t read_bytes = 1;
	uint32_t write_bytes ;
	// for(i = 0; i < 200; i++){
	// 	buffer[i] = 0;
	// }
	fd = open((uint8_t*)("frame0.txt"));
	// printf("BAP\n");
	// printf("%d\n", fd);
	read_bytes = read(fd, terminal_buffer, 200);
	// printf("BAP2\n");
	write_bytes = write(1, terminal_buffer, read_bytes);
	close(fd);
	if(write_bytes == -1){
		return FAIL;
	}
	return PASS;
}

int syscall_test_execute(){
	TEST_HEADER;
	// i
	if(execute((uint8_t*)"shell ") == -1){
		return FAIL;
	}
	return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here	

	// CHECKPOINT 1 TESTS
	// TEST_OUTPUT("div_zero test", div_zero());
	// TEST_OUTPUT("overflow test (iykyk)", overflow_test());
	// TEST_OUTPUT("paging from idt test", paging_exception_idt_test());
	// TEST_OUTPUT("testing garbage values", idt_garbage_value());
	// TEST_OUTPUT("interrupts test", rtc_test_1024hz());
	// TEST_OUTPUT("interrupts test", rtc_test_8192hz());
	// TEST_OUTPUT("interrupts test", rtc_test_2hz());
	// TEST_OUTPUT("interrupts test, out of bounds", rtc_test_out_bounds());
	// TEST_OUTPUT("paging test", page());
	// TEST_OUTPUT("null paging test", null_page());
	// TEST_OUTPUT("out paging test", out_page());
	// TEST_OUTPUT("out pagingF test", out_pageF());
	


	// CHECKPOINT 2 TESTS
	// TEST_OUTPUT("rtc incremenet", rtc_test_increment());
	TEST_OUTPUT("directory read", directory_test_case());
	// TEST_OUTPUT("Terminal Driver test", terminal_rw());
	// TEST_OUTPUT("file open test", file_open_test());
	// TEST_OUTPUT("read data test frame 0", read_test_case_frame0());
	// TEST_OUTPUT("read data test frame 1", read_test_case_frame1());
	// TEST_OUTPUT("read test case longname", read_test_case_longname());
	// TEST_OUTPUT("read test case longname missing_last_letter", read_test_case_longname_missing_last_letter());
	// TEST_OUTPUT("read data test", read_test_case_GREP());
	// TEST_OUTPUT("read data test fish", read_test_case_fish());
	// TEST_OUTPUT("read data test ls", read_test_case_LS());
	// TEST_OUTPUT("read dentry by index test", read_dentry_by_index_test());
	// TEST_OUTPUT("read dentry by index test bounds", read_dentry_by_index_test_bounds());
	// TEST_OUTPUT("read data bound test", test_read_data_bounds());



	// CHECKPOINT 3 TESTS
	// TEST_OUTPUT("syscall open and read test", syscall_test_open_read());
	// TEST_OUTPUT("syscall write test", syscall_test_write());
	// TEST_OUTPUT("execute test", syscall_test_execute());
}
