#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple stack overflow


// Function to generate a random number between min and max [returns LONG LONG]
long long generate_random_number(long long min, long long max) {
    long long rand_large = ((long long)rand() << 32) | rand(); // Combine two rand() calls
	printf("\n [+] large number generated:" %l$l$l); 
    return min + (rand_large % (max - min + 1));
}


// Requesting a lot of space from heap
void memory_hog(int size) {
    int *buff = (int *)malloc(size);
    int *buff_2 = (int *)malloc(size);
	
	
    // Note: Need to free memory to avoid memory leaks
    free(buff);
    free(buff_2);
}

int main(void) {
	
	int count = 0 
    while(1)
	{
	count += 1 
	srand(time(NULL));
	long long min_value = 1000000;
    long long max_value = 10000000000;
	long long large_random_number = generate_random_number(min_value, max_value);
	long long large_random_data = memory_hog(large_random_number);
	printf("[+] Attempt " #{count} " --large random numbe ");
    }
    return 1;
}

// Run main file
void mainRun() {
    while(1) {
        main();
    }
}
mainRun();
