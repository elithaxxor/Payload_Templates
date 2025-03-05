#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <regex.h>
#include <errno.h>
#include <sys/utsname.h> // For platform detection
#include <curl/curl.h>   // For external IP (using libcurl, install it: sudo apt-get install libcurl4-openssl-dev or similar)

#ifdef _WIN32
#include <windows.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/wait.h>
#endif

// [Color Schema] output
struct colors {
    char *GREEN;
    char *RED;
    char *YELLOW;
    char *NC;
};
struct colors color_codes = {
    "\033[92m",
    "\033[91m",
    "\033[93m",
    "\033[0m"
};

// Function to execute shell commands and get output
// 1.Shell uses popen() to pipe comands & read in 128 byte buffers
// 2.Dynamically allot memory to complete useres input, and return data (data type error handling may be needed
char* execute_command(const char* command) {
    char buffer[128];
    char* result = NULL;
    size_t result_len = 0;

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        perror("popen failed");
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t chunk_len = strlen(buffer);
        char* temp = realloc(result, result_len + chunk_len + 1);
        if (temp == NULL) {
            perror("realloc failed");
            if (result) free(result);
            pclose(pipe);
            return NULL;
        }
        result = temp;
        strcpy(result + result_len, buffer);
        result_len += chunk_len;
    }

    pclose(pipe);
    if (result) {
        result[result_len] = '\0'; // Ensure null termination
    }
    return result;
}
