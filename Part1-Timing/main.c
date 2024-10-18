#include "utility.h"


#ifndef VISUAL
#define PRINT_FUNC print_results_plaintext
#else
#define PRINT_FUNC print_results_for_visualization
#endif

#define LINE_SIZE 64
// [1.2] TODO: Uncomment the following lines and fill in the correct size
#define L1_SIZE 32768
#define L2_SIZE 262144
#define L3_SIZE 6291456
// #define BUFF_SIZE 64
 
int main (int ac, char **av) {

    // create 4 arrays to store the latency numbers
    // the arrays are initialized to 0
    uint64_t dram_latency[SAMPLES] = {0};
    uint64_t l1_latency[SAMPLES] = {0};
    uint64_t l2_latency[SAMPLES] = {0};
    uint64_t l3_latency[SAMPLES] = {0};

    // A temporary variable we can use to load addresses
    uint8_t tmp;

    // Allocate a buffer of LINE_SIZE Bytes
    // The volatile keyword tells the compiler to not put this variable into a
    // register -- it should always try to be loaded from memory / cache.
    volatile uint8_t *target_buffer = (uint8_t *)malloc(LINE_SIZE);

    if (NULL == target_buffer) {
        perror("Unable to malloc");
        return EXIT_FAILURE;
    }

    // [1.2] TODO: Uncomment the following line to allocate a buffer of a size
    // of your chosing. This will help you measure the latencies at L2 and L3.
    //int buff_size = 128;
    //volatile uint8_t *eviction_buffer = (uint8_t *)malloc(buff_size);

    // Example: Measure L1 access latency, store results in l1_latency array
    for (int i=0; i<SAMPLES; i++){
        // Step 1: bring the target cache line into L1 by simply accessing
        //         the line
        tmp = target_buffer[0];

        // Step 2: measure the access latency
        l1_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.2] TODO: Measure DRAM Latency, store results in dram_latency array
    // ======
    //
	for (int i=0; i<SAMPLES; i++){
		clflush((void*)&target_buffer[0]);
		lfence();
		dram_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
	}
	
    // ======
    // [1.2] TODO: Measure L2 Latency, store results in l2_latency array
    // ======
    //
	for (int i=0; i<SAMPLES; i++) {
		volatile uint8_t *evict_buffer = (uint8_t *)malloc(2*L1_SIZE);

		// Flush value and read to make sure it is in all caches
		clflush((void*)&target_buffer[0]);
		lfence();
		tmp = target_buffer[0];

		// Fill in your eviction buffer which is enough to evict L1
		for (int i=0; i<2*L1_SIZE; i++){
			evict_buffer[i] = i;
		}
		lfence();

		// Now access your target buffer again
        l2_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
		free((void *)evict_buffer);
	}

    // ======
    // [1.2] TODO: Measure L3 Latency, store results in l3_latency array
    // ======
    //
	for (int i=0; i<SAMPLES; i++) {
		volatile uint8_t *evict_buffer = (uint8_t *)malloc(2*L2_SIZE);

		// Flush value and read to make sure it is in all caches
		clflush((void*)&target_buffer[0]);
		lfence();
		tmp = target_buffer[0];

		// Fill in your eviction buffer which is enough to evict L2
		for (int i=0; i<2*L2_SIZE; i++){
			evict_buffer[i] = i;
		}
		lfence();

		// Now access your target buffer again
        l3_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
		free((void *)evict_buffer);
	}
	


    // Print the results to the screen
    // When compile to main and used by `make run`,
    // it uses print_results_plaintext
    // When compile to main-visual and used by `run.py`,
    // it uses print_results_for_visualization
    PRINT_FUNC(dram_latency, l1_latency, l2_latency, l3_latency);

    free((uint8_t *)target_buffer);

    // [1.2] TODO: Uncomment this line once you uncomment the eviction_buffer
    //             creation line
    //free((uint8_t *)eviction_buffer);
    return 0;
}

