


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple stack overflow
	/* "System Instability: The program will eventually consume all available RAM and swap space, Performance Degradation: The system may become unresponsive  terminate processes to free memory, Crash The program ( system) will crash when memory is fully exhausted.")
	("good luck") */


print("[!] System Instability: The program will eventually consume all available RAM and swap space, Performance Degradation: The system may become unresponsive  terminate processes to free memory, Crash The program ( system) will crash when memory is fully exhausted.");
print("[-] good luck");

void memory_hog(size_t size) {
    // 1. init Static variable to remember the previous difference across calls
    static ptrdiff_t last_diff = 0;
	
	// 2. init mallac and calloc buffers, if null, then exit program. 
		//  -->  [allocates] space (space in size paramater) to the heap but does noto free it
		// [Calloc] will put the space all to 0's whereas malloc will leave the garbage behind

    int *buff = (int *)malloc(size);
    int *buff_2 = (int *)calloc(size, sizeof(int));
	if (buff == NULL || buff_2 == NULL) { printf"[-] failed to memaloc and calloc, exiting "; exit(1); } 
	
	else {
	// 3. Update last_diff for the next iteration
		printf("[*] Old difference: %td, New difference: %td\n", last_diff, new_diff);
        ptrdiff_t new_diff = buff_2 - buff;
        last_diff = new_diff;
    }

    // Free the allocated memory to avoid leaks (though the loop never ends)
    free(buff);
    free(buff_2);
}

int main(void) {
	print("[!] Initiating 10 GB per iteration, with 3 second intervals, good luck");
	
    while(1) {
        memory_hog(10000000000);
		sleep(3); 
    }
    return 1;
}

// Run main file
void mainRun() {
	printf("[+] Simple buffer stack overflow, using malloc"); 
    while(1) {
        main();
    }
}
mainRun();
