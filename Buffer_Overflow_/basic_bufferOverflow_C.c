#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Simple stack overflow


// reqeusting alot of space from h
int main(void) {
	while(1) {
		 memoryhog(10000000000);
	 }
	return 1;
}

void memory_hog(int size) {
	int *buff = malloc(size); 
	int *buff_2 = calloc();
}
// run main file
void mainRun() {
	while(1){
		main();
	}
}; mainRun();


