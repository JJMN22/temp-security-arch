
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

// [Bonus] TODO: define your own buffer size
#define BUFF_SIZE (1<<21)
//#define BUFF_SIZE TODO

// Given the set number, get the offset required to start iterating
size_t get_offset(int set) {
	return (set << 6);
}

void evict(int set, void *hugepage) {
	// Essentially the same as the prime step that the receiver makes
	size_t offset = get_offset(set);
	void *buffer_entry = (void *)((char *)hugepage + offset);

	
	// Set associativity is 4
	// t represents the tag, which shifts by 2^16 (10 set bits + 6 offset)
	// o represents the offset, which shifts by 1
	for (int t=0; t<8; t++) {
		for (int o=0; o<8; o++) {
			*((char *)buffer_entry + t*(2 << 16) + o) = 7;
		}
	}
}

/* The lfence instruction borrowed from Part1 */
static inline void lfence() {
    asm volatile("lfence");
}

// Comparison function to sort and find median later, borrowed from part 1
int compare(const void *p1, const void *p2) {
    uint64_t u1 = *(uint64_t *)p1;
    uint64_t u2 = *(uint64_t *)p2;

    return (int)u1 - (int)u2;
}

int main(int argc, char **argv)
{
    // Allocate a buffer using huge page
    // See the handout for details about hugepage management
    void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    
    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)buf) = 1; // dummy write to trigger page allocation

    printf("Please type a message.\n");

    bool sending = true;
    char text_buf[128];
    fgets(text_buf, sizeof(text_buf), stdin);
	int secret = string_to_int(text_buf);
    while (sending) {
		evict(secret, buf);
    }

    printf("Sender finished.\n");
    return 0;
}


