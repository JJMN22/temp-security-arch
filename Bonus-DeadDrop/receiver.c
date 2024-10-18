#include"util.h"
#include <stdio.h>
#include <stdlib.h>
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUFF_SIZE (1<<21)
#define L3_THRESHOLD 41

// Comparison function to sort and find median later, borrowed from part 1
int compare(const void *p1, const void *p2) {
    uint64_t u1 = *(uint64_t *)p1;
    uint64_t u2 = *(uint64_t *)p2;

    return (int)u1 - (int)u2;
}

/* The lfence instruction borrowed from Part1 */
static inline void lfence() {
    asm volatile("lfence");
}

// Given the set number, get the offset required to start iterating
size_t get_offset(int set) {
	return (set << 6);
}

void prime(int set, void *hugepage) {
	size_t offset = get_offset(set);
	void *buffer_entry = (void *)((char *)hugepage + offset);
	
	// Set associativity is 4. We go through 4 different tag bits and 
	// write 8 values (8 offsets) each. 
	for (int t=0; t<4; t++) {
		for (int o=0; o<8; o++) {
			*((char *)buffer_entry + t*(2 << 16) + o) = 7;
		}
	}
}

int probe(int set, void *hugepage){
	// Just measure the latency a single time. We'll find the correct
	// times statistically.
	size_t offset = get_offset(set);
	void *buffer_entry = (void *)((char *)hugepage + offset);
	int access_time = measure_one_block_access_time((uint64_t)buffer_entry);
	return access_time;
}

int guess_secret(void *hugepage){

	// Create a 2D array to measure the access times
	int num_sets = 256;
	int num_measurements = 50;
	int access_times[num_sets][num_measurements];

	// Run prime and probe on each set
	for (int i=0; i<num_measurements; i++) {
		for (int s=0; s<num_sets; s++) {
			prime(s, hugepage);
			prime(s, hugepage);
			lfence();

			int probe_cycles = probe(s, hugepage);
			access_times[s][i] = probe_cycles;
		}
	}

	// Sort each array 
	for (int s=0; s<num_sets; s++) {
		qsort(access_times[s], num_measurements, sizeof(int), compare);
	}
	
	// Grab the median time of each, and see if it is over the threshold and that it is unique
	int secret = -1;
	for (int s=0; s<num_sets; s++) {
		if (access_times[s][num_measurements/2] >= L3_THRESHOLD) {
			if (secret != -1) {
				// Something went wrong, we have two readings over the threshold
				return -1;
			}
			// Otherwise set the secret to this set value.
			secret = s;
		}
	}
	return secret;
}

int main(int argc, char **argv)
{
    void *hugepage= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    if (hugepage == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    *((char *)hugepage) = 1; // dummy write to trigger page allocation

    printf("Please press enter.\n");

    char text_buf[2];
    fgets(text_buf, sizeof(text_buf), stdin);

    printf("Receiver now listening.\n");
	int secret = -1;
    bool listening = true;
    while (listening) {
		secret = guess_secret(hugepage);
		if (secret != -1) {
			listening = false;
		}
    }
	printf("The secret is: %d\n", secret);
    printf("Receiver finished.\n");

    return 0;
}


