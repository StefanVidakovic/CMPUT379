#include "valws379.h"

#define MAX_WS UINT64_MAX // Max Windowsize
#define MIN_PAGE_SIZE 16 // Minimum pagesize
#define MAX_PAGE_SIZE 65536 // Maximum pagesize

int isPowerOfTwo(uint32_t x) {
	return ((x != 0) && !(x & (x - 1)));
}

int main(int argc, char **argv) {

	int ignore_instructions = 0;

	if (argc == 4) { // if there is an -i
		if (argv[argc - 3] == "-i") { // Third to last argument
			ignore_instructions = 1;
		}
	}

	else if (argc == 3) { // if there is no -i
	}

	else {
		printf("Wrong number of arguments given, please enter \n");
		printf("the optional parameter -i \n");
		printf("the page size a power of 2 from 16 to 65,536 \n");
		printf("and the windowsize, a positive 64 bit int.\n");
		return -1;
	}

	char *str, *end; // end should be a null pointer
	end = NULL;

	uint32_t pagesize = strtoumax(argv[argc - 2],&end,10); // Second to last argument

	str = argv[argc - 1]; // last argument
	uint64_t windowsize;
	int errno = 0;
	windowsize = strtoull(str, &end, 10);
	printf("Windowsize: %d\n",windowsize);
	if (windowsize == 0 && end == str) {
		printf("The windowsize was not a number.\n");
		return -1;
	}

	else if (windowsize == ULLONG_MAX && errno) {
		printf("The value of windowsize does not fit in unsigned long long.\n");
		return -1;
	}
	else if (*end) {
		printf("Windowsize began with a number but has junk left over at the end.\n");
		return -1;
	}
	if (pagesize < MIN_PAGE_SIZE) { // Check if pagesize is less than MIN_PAGE_SIZE
		printf("The pagesize was less than MIN_PAGE_SIZE\n");
		return -1;
	}
	if (pagesize > MAX_PAGE_SIZE) { // Check if pagesize is greater than MAX_PAGE_SIZE
		printf("The pagesize was greater than MAX_PAGE_SIZE\n");
		return -1;
	}
	if (!isPowerOfTwo(pagesize)) { // Check if pagesize is a power of 2
		printf("The pagesize was not a power of 2.\n");
		return -1;
	}
	/* Linked list initialization*/
	struct node base;
	base.next = NULL;

	/* Initialization for getline()*/
	uint32_t num_unique_pages;
	char *buffer = NULL;
	size_t buffer_size = sizeof(char)*1024; // Make sure we have lots of room in our buf
	char address[9]; // 8 char + 1 null terminator
	uint32_t address_num;
	buffer = (char *)malloc(buffer_size * sizeof(char));

	if (buffer == NULL) {
		perror("Unable to alocate buffer");
		exit(1);
	}

	/* Initialization of Hashmap*/
	int num_pages;
	int * hashmap;
	hashmap = malloc((UINT32_MAX / pagesize + 1) * sizeof(num_pages));
	for (int i = 0; i < (UINT32_MAX / pagesize); i++) {
		hashmap[i] = 0;
	}

	uint64_t count = 0;
	while (getline(&buffer, &buffer_size, stdin) != -1) {
		if (ignore_instructions == 1) {
			if (strcmp(&buffer[0], "I") != 0) {
				strncpy(address, buffer + 3, 8); // Add 3 to get to the start of the 8 char address
				address_num = strtoumax(address, &end, 16);
			}
			else if (strcmp(&buffer[0], "I") == 0) {
				continue;
			}
		}
		if (ignore_instructions == 0) {
			strncpy(address, buffer + 3, 8); // Add 3 to get to the start of the 8 char address
			address_num = strtoumax(address, &end, 16);
			// printf("Address: %s\n",address);
		}
		count++;
		// if the number entries in the linked list is greater than windowsize
		if (count > windowsize) {
			uint32_t popped = linkedlistPop(&base);
			hashmap[popped / pagesize] = hashmap[popped / pagesize] - 1;
			if (hashmap[popped / pagesize] == 0) {
				num_unique_pages--;
			}
		}

		linkedlistPush(&base, address_num);

		if (hashmap[address_num / pagesize] == 0) {
			num_unique_pages++;
			hashmap[address_num / pagesize] = 1;
		}
		else {
			hashmap[address_num / pagesize] = hashmap[address_num / pagesize] + 1;
		}
		printf("%u\n",num_unique_pages);
	}

}
