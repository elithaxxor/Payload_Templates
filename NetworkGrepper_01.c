#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h> // Grabs system info
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h> // For bool type

// TODO: Run across platfors
// TODO: Request user if they want automized mac / ip randomization
// TODO: Make it so it logs, and hides in useres desktop (hidden)
// TODO: Reverse TCP-shell
// TODO: Find logs and password hashes and store them
// TODO: Link to NetVision for IOT Grepping

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <iphlpapi.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "iphlpapi.lib")

    #define OS_TYPE "Windows"
#else

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #ifdef __APPLE__
        #define OS_TYPE "Darwin"
    #else
        #define OS_TYPE "Linux"
    #endif
#endif

// Define colors for terminal output
#define COLOR_GREEN "\033[92m"
#define COLOR_RED "\033[91m"
#define COLOR_YELLOW "\033[93m"
#define COLOR_BLUE "\033[94m"
#define COLOR_MAGENTA "\033[95m"
#define COLOR_CYAN "\033[96m"
#define COLOR_WHITE "\033[97m"
#define COLOR_NC "\033[0m"
#define COLOR_BOLD "\033[1m"


// #TODO Add functionality to change interal IP to proxy
// #TODO Add logging and print
// #TODO Finish Proxy Server

// Define a constant for the separator
#define SEPARATOR "==================================================================="
// Define a constant for the menu title


char* get_internal_ip();
char* get_external_ip();
char* get_current_mac(const char* interface);
void renew_ip();
char* change_mac(const char* interface);
void revert_mac(const char* interface);
//void* populate_sysinfo(Darwin_SysInfo* sysinfo);
// Function to clear the screen (cross-platform)
void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Function to pause for user input (cross-platform)
void pause_me() {
    printf("\n%sPress Enter to continue...%s", COLOR_YELLOW, COLOR_NC);
    getchar();
    // Consume any additional characters
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Function to execute a command and return its output
char* execute_command(const char* command) {
    FILE* fp;
    char* output = malloc(4096); // Allocate enough space for command output
    char buffer[1024];
    char* result = NULL;
    size_t size = 0;
   // free(output);

    if (output == NULL) {
        fprintf(stderr, "%sMemory allocation failed%s\n", COLOR_RED, COLOR_NC);
        return NULL;
    }

    #ifdef _WIN32
        fp = _popen(command, "r");
    #else
        fp = popen(command, "r");
    #endif

    if (fp == NULL) {
        printf("%sError executing command: %s%s\n", COLOR_RED, command, COLOR_NC);
        free(output);
        return NULL;
    }

    output[0] = '\0'; // Initialize empty string
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strcat(output, buffer);
        size_t len = strlen(buffer);
        char* temp = realloc(result, size + len + 1);
        if (!temp) {
            fprintf(stderr, COLOR_RED "Error: Memory allocation failed\n" COLOR_NC);
            free(result);
            //pclose(pipe*);
            return NULL;
        }
        result = temp;
        strcpy(result + size, buffer);
        size += len;
    }



    #ifdef _WIN32
        _pclose(fp);
    #else
        pclose(fp);
    #endif
   // free(output);
    free(result);
    return output;
}

// Function to get internal IP address
char* get_internal_ip() {
    char* ip = malloc(16); // IPv4 address is at most 15 chars + null terminator

    if (ip == NULL) {
        fprintf(stderr, "%sMemory allocation failed%s\n", COLOR_RED, COLOR_NC);
        return NULL;
    }

    #ifdef _WIN32
        // Windows implementation
        char* output = execute_command("ipconfig");
        if (output == NULL) {
            free(ip);
            return NULL;
        }

        regex_t regex;
        regmatch_t matches[2];

        if (regcomp(&regex, "IPv4 Address[^\\n:]*: ([\\d.]+)", REG_EXTENDED) != 0) {
            printf("%sRegex compilation failed%s\n", COLOR_RED, COLOR_NC);
            free(ip);
            free(output);
            return NULL;
        }

        if (regexec(&regex, output, 2, matches, 0) == 0) {
            int start = matches[1].rm_so;
            int end = matches[1].rm_eo;
            strncpy(ip, output + start, end - start);
            ip[end - start] = '\0';
        } else {
            free(ip);
            ip = NULL;
        }

        regfree(&regex);
        free(output);
    #elif defined(__APPLE__)
        // macOS implementation
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
            perror("getifaddrs");
            free(ip);
            return NULL;
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL)
                continue;

            family = ifa->ifa_addr->sa_family;

            if (family == AF_INET &&
                strcmp(ifa->ifa_name, "lo0") != 0 &&
                (strncmp(ifa->ifa_name, "en", 2) == 0 || strncmp(ifa->ifa_name, "eth", 3) == 0)) {
                s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                               host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    free(ip);
                    freeifaddrs(ifaddr);
                    return NULL;
                }

                strcpy(ip, host);
                freeifaddrs(ifaddr);
                return ip;
            }
        }

        free(ip);
        freeifaddrs(ifaddr);
        ip = NULL;
    #else
        // Linux implementation
        char* output = execute_command("hostname -I");
        if (output == NULL) {
            free(ip);
            return NULL;
        }

        char* first_ip = strtok(output, " \t\n");
        if (first_ip != NULL) {
            strcpy(ip, first_ip);
        } else {
            free(ip);
            ip = NULL;
        }

        free(output);
    #endif

    return ip;
}

// Function to get external IP address
char* get_external_ip() {
    char* ip = malloc(16); // IPv4 address is at most 15 chars + null terminator

    if (ip == NULL) {
        fprintf(stderr, "%sMemory allocation failed%s\n", COLOR_RED, COLOR_NC);
        return NULL;
    }

    #ifdef _WIN32
        char* output = execute_command("curl -s https://api.ipify.org");
    #else
        char* output = execute_command("curl -s https://api.ipify.org");
    #endif

    if (output == NULL || strlen(output) == 0 || strlen(output) > 15) {
        printf("%sError fetching external IP%s\n", COLOR_RED, COLOR_NC);
        free(ip);
        if (output) free(output);
        return NULL;
    }

    strcpy(ip, output);
    // Remove any newline character
    ip[strcspn(ip, "\r\n")] = 0;
    free(output);
    return ip;
}

// Function to renew IP address
void renew_ip() {
    printf("%sRenewing IP address...%s\n", COLOR_CYAN, COLOR_NC);

    #ifdef _WIN32
        system("ipconfig /release");
        system("ipconfig /renew");
    #elif defined(__APPLE__)
        // On macOS, this is a bit more complex and requires sudo
        printf("%sOn macOS, you need to run the following commands with sudo:%s\n", COLOR_YELLOW, COLOR_NC);
        printf("sudo ifconfig en0 down && sudo ifconfig en0 up\n");
    #else
        // Linux
        system("dhclient -r");
        system("dhclient");
    #endif

    printf("%sIP renewal completed.%s\n", COLOR_GREEN, COLOR_NC);
}

// Function to get current MAC address
char* get_current_mac(const char* interface) {
    char* mac = malloc(18); // MAC address is 17 chars + null terminator
    char command[100];

    if (mac == NULL) {
        fprintf(stderr, "%sMemory allocation failed%s\n", COLOR_RED, COLOR_NC);
        return NULL;
    }

    #ifdef _WIN32
        sprintf(command, "getmac /v /fo csv | findstr \"%s\"", interface);
        char* output = execute_command(command);
        if (output == NULL) {
            free(mac);
            return NULL;
        }

        regex_t regex;
        regmatch_t matches[2];

        if (regcomp(&regex, "([0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2})", REG_EXTENDED | REG_ICASE) != 0) {
            printf("%sRegex compilation failed%s\n", COLOR_RED, COLOR_NC);
            free(mac);
            free(output);
            return NULL;
        }

        if (regexec(&regex, output, 2, matches, 0) == 0) {
            int start = matches[1].rm_so;
            int end = matches[1].rm_eo;
            strncpy(mac, output + start, end - start);
            mac[end - start] = '\0';
        } else {
            free(mac);
            mac = NULL;
        }

        regfree(&regex);
        free(output);
    #elif defined(__APPLE__)
        sprintf(command, "ifconfig %s | grep ether", interface);
        char* output = execute_command(command);
        if (output == NULL) {
            free(mac);
            return NULL;
        }

        regex_t regex;
        regmatch_t matches[2];

        if (regcomp(&regex, "ether\\s+([0-9a-fA-F:]+)", REG_EXTENDED) != 0) {
            printf("%sRegex compilation failed%s\n", COLOR_RED, COLOR_NC);
            free(mac);
            free(output);
            return NULL;
        }

        if (regexec(&regex, output, 2, matches, 0) == 0) {
            int start = matches[1].rm_so;
            int end = matches[1].rm_eo;
            strncpy(mac, output + start, end - start);
            mac[end - start] = '\0';
        } else {
            free(mac);
            mac = NULL;
        }

        regfree(&regex);
        free(output);
    #else
        // Linux
        sprintf(command, "ifconfig %s | grep ether", interface);
        char* output = execute_command(command);
        if (output == NULL) {
            free(mac);
            return NULL;
        }

        regex_t regex;
        regmatch_t matches[2];

        if (regcomp(&regex, "ether\\s+([0-9a-fA-F:]+)", REG_EXTENDED) != 0) {
            printf("%sRegex compilation failed%s\n", COLOR_RED, COLOR_NC);
            free(mac);
            free(output);
            return NULL;
        }

        if (regexec(&regex, output, 2, matches, 0) == 0) {
            int start = matches[1].rm_so;
            int end = matches[1].rm_eo;
            strncpy(mac, output + start, end - start);
            mac[end - start] = '\0';
        } else {
            free(mac);
            mac = NULL;
        }

        regfree(&regex);
        free(output);
    #endif

    return mac;
}

// Function to change MAC address
char* change_mac(const char* interface) {
    char* new_mac = malloc(18); // MAC address is 17 chars + null terminator
    char command[256];

    if (new_mac == NULL) {
        fprintf(stderr, "%sMemory allocation failed%s\n", COLOR_RED, COLOR_NC);
        return NULL;
    }

    // Generate a random MAC address
    srand(time(NULL));
    sprintf(new_mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            rand() % 256, rand() % 256, rand() % 256,
            rand() % 256, rand() % 256, rand() % 256);

    printf("%sChanging MAC address to %s...%s\n", COLOR_CYAN, new_mac, COLOR_NC);

    #ifdef _WIN32
        printf("%sOn Windows, MAC address changing requires specialized tools.%s\n", COLOR_YELLOW, COLOR_NC);
        printf("%sYou can try using external tools like TMAC or Advanced IP Scanner.%s\n", COLOR_YELLOW, COLOR_NC);
    #elif defined(__APPLE__)
        // macOS requires ifconfig for MAC changing and needs sudo
        printf("%sOn macOS, you need to run the following commands with sudo:%s\n", COLOR_YELLOW, COLOR_NC);
        sprintf(command, "sudo ifconfig %s ether %s", interface, new_mac);
        printf("%s\n", command);
    #else
        // Linux uses macchanger
        sprintf(command, "macchanger -m %s %s", new_mac, interface);
        system(command);
    #endif

    return new_mac;
}

// Function to revert MAC address to the permanent MAC address
void revert_mac(const char* interface) {
    printf("%sReverting MAC address to permanent MAC address...%s\n", COLOR_CYAN, COLOR_NC);

    #ifdef _WIN32
        printf("%sOn Windows, MAC address changing requires specialized tools.%s\n", COLOR_YELLOW, COLOR_NC);
        printf("%sYou can try using external tools like TMAC or Advanced IP Scanner.%s\n", COLOR_YELLOW, COLOR_NC);
    #elif defined(__APPLE__)
        printf("%sOn macOS, you need to restart the network interface:%s\n", COLOR_YELLOW, COLOR_NC);
        printf("sudo ifconfig %s down && sudo ifconfig %s up\n", interface, interface);
    #else
        char command[100];
        sprintf(command, "macchanger -p %s", interface);
        system(command);
    #endif

    printf("%sMAC address reverted successfully!%s\n", COLOR_GREEN, COLOR_NC);
}

// Function to get default network interface
char* get_default_interface() {
    char* interface = malloc(16); // Interface name is at most 15 chars + null terminator

    if (interface == NULL) {
        fprintf(stderr, "%sMemory allocation failed%s\n", COLOR_RED, COLOR_NC);
        return NULL;
    }

    #ifdef _WIN32
        strcpy(interface, "Ethernet");
    #elif defined(__APPLE__)
        strcpy(interface, "en0");
    #else
        // Linux
        char* output = execute_command("route | grep default | awk '{print $8}'");
        if (output == NULL || strlen(output) == 0) {
            // Try another approach
            free(output);
            output = execute_command("ip route | grep default | awk '{print $5}'");
        }

        if (output == NULL || strlen(output) == 0) {
            free(interface);
            if (output) free(output);
            strcpy(interface, "eth0"); // Default fallback
            return interface;
        }

        strcpy(interface, output);
        // Remove any newline character
        interface[strcspn(interface, "\r\n")] = 0;
        free(output);
    #endif

    return interface;
}

// Function to show network information
void show_network_info() {
    clear_screen();

    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);
    printf("%s%s NETWORK INFORMATION %s\n", COLOR_BLUE, COLOR_BOLD, COLOR_NC);
    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

    // Get default interface
    char* interface = get_default_interface();
    printf("Default Interface: %s%s%s\n", COLOR_GREEN, interface, COLOR_NC);

    // Get current MAC address
    char* current_mac = get_current_mac(interface);
    printf("Current MAC address: %s%s%s\n", COLOR_GREEN, current_mac ? current_mac : "Unknown", COLOR_NC);

    // Get internal IP address
    char* internal_ip = get_internal_ip();
    if (internal_ip) {
        printf("Internal IP address: %s%s%s\n", COLOR_GREEN, internal_ip, COLOR_NC);
        free(internal_ip);
    } else {
        printf("Internal IP address: %sUnknown%s\n", COLOR_RED, COLOR_NC);
    }

    // Get external IP address
    char* external_ip = get_external_ip();
    if (external_ip) {
        printf("External IP address: %s%s%s\n", COLOR_GREEN, external_ip, COLOR_NC);
    } else {
        printf("External IP address: %sUnknown%s\n", COLOR_RED, COLOR_NC);
    }

    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

    // Free allocated memory
    if (interface) free(interface);
    if (current_mac) free(current_mac);
    if (internal_ip) free(internal_ip);
    if (external_ip) free(external_ip);

    pause_me();
}

// Function to handle MAC address operations
void mac_address_operations() {
    clear_screen();
    int choice;
    char* interface = get_default_interface();

    while (1) {
        clear_screen();
        printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);
        printf("%s%s MAC ADDRESS OPERATIONS %s\n", COLOR_MAGENTA, COLOR_BOLD, COLOR_NC);
        printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

        printf("1. %sShow Current MAC Address%s\n", COLOR_CYAN, COLOR_NC);
        printf("2. %sChange MAC Address%s\n", COLOR_CYAN, COLOR_NC);
        printf("3. %sRevert to Permanent MAC Address%s\n", COLOR_CYAN, COLOR_NC);
        printf("4. %sReturn to Main Menu%s\n", COLOR_YELLOW, COLOR_NC);

        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer on invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        // Consume newline
        getchar();

        switch (choice) {
            case 1: {
                char* current_mac = get_current_mac(interface);
                printf("\nCurrent MAC address on %s: %s%s%s\n",
                       interface, COLOR_GREEN, current_mac ? current_mac : "Unknown", COLOR_NC);
                if (current_mac) free(current_mac);
                pause_me();
                break;
            }
            case 2: {
                printf("\n%sChanging MAC address for interface %s%s\n\n", COLOR_CYAN, interface, COLOR_NC);
                char* new_mac = change_mac(interface);
                if (new_mac) {
                    printf("\nNew MAC address: %s%s%s\n", COLOR_GREEN, new_mac, COLOR_NC);
                    free(new_mac);
                } else {
                    printf("\n%sFailed to change MAC address%s\n", COLOR_RED, COLOR_NC);
                }
                pause_me();
                break;
            }
            case 3:
                revert_mac(interface);
                pause_me();
                break;
            case 4:
                if (interface) free(interface);
                return;
            default:
                printf("\n%sInvalid choice. Please try again.%s\n", COLOR_RED, COLOR_NC);
                pause_me();
        }
    }
}

// Function to handle IP address operations
void ip_address_operations() {
    clear_screen();
    int choice;

    while (1) {
        clear_screen();
        printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);
        printf("%s%s IP ADDRESS OPERATIONS %s\n", COLOR_CYAN, COLOR_BOLD, COLOR_NC);
        printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

        printf("1. %sShow Current IP Addresses%s\n", COLOR_CYAN, COLOR_NC);
        printf("2. %sRenew IP Address%s\n", COLOR_CYAN, COLOR_NC);
        printf("3. %sReturn to Main Menu%s\n", COLOR_YELLOW, COLOR_NC);

        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer on invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        // Consume newline
        getchar();

        switch (choice) {
            case 1: {
                char* internal_ip = get_internal_ip();
                char* external_ip = get_external_ip();

                printf("\nInternal IP address: %s%s%s\n",
                       COLOR_GREEN, internal_ip ? internal_ip : "Unknown", COLOR_NC);
                printf("External IP address: %s%s%s\n",
                       COLOR_GREEN, external_ip ? external_ip : "Unknown", COLOR_NC);

                if (internal_ip) free(internal_ip);
                if (external_ip) free(external_ip);

                pause_me();
                break;
            }
            case 2:
                renew_ip();
                pause_me();
                break;
            case 3:
                return;
            default:
                printf("\n%sInvalid choice. Please try again.%s\n", COLOR_RED, COLOR_NC);
                pause_me();
        }
    }
}
/*==========================================================================================*/
// * Checks architecture and compiler
#ifdef _WIN32
    printf("[+] Windows System Architecture: %s%s%s\n", COLOR_GREEN, sizeof(void*) == 8 ? "x64" : "x86", COLOR_NC);

#if defined(__MINGW32__) || defined(__MINGW64__)
    printf("[+] Windows Compiler: %s%s%s\n", COLOR_GREEN, "MinGW", COLOR_NC);
#elif defined(_MSC_VER)
    printf("[+] Windows Compiler: %s%s%s\n", COLOR_GREEN, "MSVC", COLOR_NC);
#endif
#endif


/*==========================================================================================*/

// [DARWIN]--> Struct to hold darwin info.
    // Structure to hold Darwin system information
    typedef struct {
        // System Version Information
        char* product_version;
        char* build_version;
        char* product_name;
        char* architecture;
        char* os_version;
        char* kernel_version;

        // Host and User Information
        char* computer_name;
        char* hostname;
        char* user_name;

        // Hardware Information
        char* model;
        char* serial_number;
        char* hardware_uuid;
        char* cpu_info;

        char* num_cpu_cores;
        char* memory_size;
        char* memory_info;
        char* disk_info;

        // Kernel Information
        // char* kernel_version;

        // System Status
        char* uptime;
        char* load_average;
        char* swap_usage;
        char* available_disk_space;
        char* cpu_temperature;
        char* battery_percentage;


        // Network Information
        char* ip_address;
        char* gateway;
        char* internal_ip;
        char* external_ip;
        char* mac_address;
    } Darwin_SysInfo;

#define MENU_TITLE "Darwin System Information Tool"

// Function to populate the Darwin_SysInfo structure
void populate_sysinfo(Darwin_SysInfo* sysinfo) {
    if (sysinfo == NULL)
    {
        printf("[-] Null error in Darwin. ");
        return; // Safety check
    }
    sysinfo->product_version = execute_command("sw_vers -productVersion");
    sysinfo->build_version = execute_command("sw_vers -buildVersion");
    sysinfo->product_name = execute_command("sw_vers -productName");
    sysinfo->computer_name = execute_command("scutil --get ComputerName");
    sysinfo->user_name = execute_command("scutil --get UserName");
    sysinfo->model = execute_command("sysctl -n hw.model");
    //sysinfo->serial_number = execute_command("system_profiler SPHardwareDataType | awk '/Serial/ {print $4}'");
    // sysinfo->hardware_uuid = execute_command("system_profiler SPHardwareDataType | awk '/Hardware UUID/ {print $3}'");
    sysinfo->architecture = execute_command("uname -m");
    sysinfo->kernel_version = execute_command("uname -r");
    sysinfo->uptime = execute_command("uptime");
    sysinfo->load_average = execute_command("sysctl -n vm.loadavg");
    sysinfo->num_cpu_cores = execute_command("sysctl -n hw.ncpu");
    sysinfo->memory_size = execute_command("sysctl -n hw.memsize");
    sysinfo->swap_usage = execute_command("sysctl -n vm.swapusage");
//  sysinfo->available_disk_space = execute_command("df -h / | awk '/\// {print $4}'");
    sysinfo->cpu_temperature = execute_command("sysctl -n machdep.xcpm.cpu_thermal_level");
    sysinfo->battery_percentage = execute_command("pmset -g batt | grep -o '[0-9]*%'");
    sysinfo->ip_address = execute_command("ifconfig en0 | grep 'inet '");
    sysinfo->gateway = execute_command("netstat -nr | grep 'default '");

    sysinfo->kernel_version = execute_command("uname -r");
    sysinfo->uptime = execute_command("uptime");
    sysinfo->load_average = execute_command("sysctl -n vm.loadavg");
    sysinfo->num_cpu_cores = execute_command("sysctl -n hw.ncpu");
    sysinfo->memory_size = execute_command("sysctl -n hw.memsize");

}

// Function to free the allocated memory
void free_sysinfo(Darwin_SysInfo* sysinfo) {
     if (sysinfo == NULL) return;

    free(sysinfo->product_version);
    free(sysinfo->build_version);
    free(sysinfo->product_name);
    free(sysinfo->computer_name);
    free(sysinfo->user_name);
    free(sysinfo->model);
    free(sysinfo->serial_number);
    free(sysinfo->hardware_uuid);
    free(sysinfo->architecture);
    free(sysinfo->kernel_version);
    free(sysinfo->uptime);
    free(sysinfo->load_average);
    free(sysinfo->num_cpu_cores);
    free(sysinfo->memory_size);
    free(sysinfo->swap_usage);
    free(sysinfo->available_disk_space);
    free(sysinfo->cpu_temperature);
    free(sysinfo->battery_percentage);
    free(sysinfo->ip_address);
    free(sysinfo->gateway);

    // Set all pointers to NULL after freeing, to prevent double frees
    sysinfo->product_version = NULL;
    sysinfo->build_version = NULL;
    sysinfo->product_name = NULL;
    sysinfo->computer_name = NULL;
    sysinfo->user_name = NULL;
    sysinfo->model = NULL;
    sysinfo->serial_number = NULL;
    sysinfo->hardware_uuid = NULL;
    sysinfo->architecture = NULL;
    sysinfo->kernel_version = NULL;
    sysinfo->uptime = NULL;
    sysinfo->load_average = NULL;
    sysinfo->num_cpu_cores = NULL;
    sysinfo->memory_size = NULL;
    sysinfo->swap_usage = NULL;
    sysinfo->available_disk_space = NULL;
    sysinfo->cpu_temperature = NULL;
    sysinfo->battery_percentage = NULL;
    sysinfo->ip_address = NULL;
    sysinfo->gateway = NULL;
}

void print_sysinfo(const Darwin_SysInfo* sysinfo) {
    if (sysinfo == NULL) return; // Safety check

    printf("--------------------------------------------------\n");
    printf("%-30s | %s\n", "Category", "Value");
    printf("--------------------------------------------------\n");

    // System Version Information
    if (sysinfo->product_name) printf("%-30s | %s\n", "Product Name", sysinfo->product_name);
    if (sysinfo->product_version) printf("%-30s | %s\n", "Product Version", sysinfo->product_version);
    if (sysinfo->build_version) printf("%-30s | %s\n", "Build Version", sysinfo->build_version);
    printf("--------------------------------------------------\n");

    // Host and User Information
    if (sysinfo->computer_name) printf("%-30s | %s\n", "Computer Name", sysinfo->computer_name);
    if (sysinfo->user_name) printf("%-30s | %s\n", "User Name", sysinfo->user_name);
    printf("--------------------------------------------------\n");

    // Hardware Information
    if (sysinfo->model) printf("%-30s | %s\n", "Model", sysinfo->model);
    if (sysinfo->serial_number) printf("%-30s | %s\n", "Serial Number", sysinfo->serial_number);
    if (sysinfo->hardware_uuid) printf("%-30s | %s\n", "Hardware UUID", sysinfo->hardware_uuid);
    if (sysinfo->architecture) printf("%-30s | %s\n", "Architecture", sysinfo->architecture);
    if (sysinfo->num_cpu_cores) printf("%-30s | %s\n", "CPU Cores", sysinfo->num_cpu_cores);

    if (sysinfo->memory_size) {
        long long bytes = atoll(sysinfo->memory_size);
        double gb = (double)bytes / (1024 * 1024 * 1024);
        printf("%-30s | %.2f GB\n", "Memory Size", gb);
    }

    printf("--------------------------------------------------\n");

    // Kernel Information
    if (sysinfo->kernel_version) printf("%-30s | %s\n", "Kernel Version", sysinfo->kernel_version);
    printf("--------------------------------------------------\n");

    // System Status
    if (sysinfo->uptime) printf("%-30s | %s\n", "Uptime", sysinfo->uptime);
    if (sysinfo->load_average) {
        // Parse load averages (assuming space-separated)
        char* load_copy = strdup(sysinfo->load_average); // Create a copy
        if (load_copy) {
            char* token = strtok(load_copy, " ");
            printf("%-30s | ", "Load Average");
            if (token) printf("1 min: %s", token);
            token = strtok(NULL, " ");
            if (token) printf(", 5 min: %s", token);
            token = strtok(NULL, " ");
            if (token) printf(", 15 min: %s", token);
            printf("\n");
            free(load_copy); // Free the copy
        }
    }

    if (sysinfo->swap_usage) printf("%-30s | %s\n", "Swap Usage", sysinfo->swap_usage);
    if (sysinfo->available_disk_space) printf("%-30s | %s\n", "Available Disk Space", sysinfo->available_disk_space);
    if (sysinfo->cpu_temperature) printf("%-30s | %s\n", "CPU Temperature", sysinfo->cpu_temperature);
    if (sysinfo->battery_percentage) printf("%-30s | %s\n", "Battery Percentage", sysinfo->battery_percentage);
    printf("--------------------------------------------------\n");

    // Network Information
    if (sysinfo->ip_address) printf("%-30s | %s\n", "IP Address", sysinfo->ip_address);
    if (sysinfo->gateway) printf("%-30s | %s\n", "Gateway", sysinfo->gateway);

    printf("--------------------------------------------------\n");
}

/*==========================================================================================*/


// Function to handle system information
void system_information() {
    clear_screen();

    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);
    printf("%s%s SYSTEM INFORMATION %s\n", COLOR_YELLOW, COLOR_BOLD, COLOR_NC);
    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

    printf("Operating System: %s%s%s\n", COLOR_GREEN, OS_TYPE, COLOR_NC);

    #ifdef _WIN32
        char* win_ver = execute_command("ver");
        if (win_ver) {
            // Remove newlines
            win_ver[strcspn(win_ver, "\r\n")] = 0;
            printf("Windows Version: %s%s%s\n", COLOR_GREEN, win_ver, COLOR_NC);
            free(win_ver);
        }
#endif

#ifdef __APPLE__
        char* mac_ver = execute_command("sw_vers -productVersion");
        free(mac_ver);
        char* mac_build = execute_command("sw_vers -buildVersion");
        char*  mac_name = execute_command("sw_vers -productName");
        char*  mac_host = execute_command("scutil --get ComputerName");
        char* mac_user = execute_command("scutil --get UserName");
        char* mac_model = execute_command("sysctl -n hw.model");
        char* mac_serial = execute_command("system_profiler SPHardwareDataType | awk '/Serial/ {print $4}'");
        char* mac_uuid = execute_command("system_profiler SPHardwareDataType | awk '/Hardware UUID/ {print $3}'");
        char* mac_arch = execute_command("uname -m");
        char* mac_kernel = execute_command("uname -r");
        char* mac_uptime = execute_command("uptime");
        char* mac_load = execute_command("sysctl -n vm.loadavg");
        char* mac_cpu = execute_command("sysctl -n hw.ncpu");
        char* mac_mem = execute_command("sysctl -n hw.memsize");
        char* mac_swap = execute_command("sysctl -n vm.swapusage");
        char* mac_disk = execute_command("df -h / | awk '//// {print $4}'");
        char* mac_temp = execute_command("sysctl -n machdep.xcpm.cpu_thermal_level");
        char* mac_battery = execute_command("pmset -g batt | grep -o '[0-9]*%'");
        char* mac_network = execute_command("ifconfig en0 | grep 'inet ' | awk '{print $2}'");
        char* mac_gateway = execute_command("netstat -nr | grep 'default' | awk '{print $2}'");

    if (mac_ver) {
        mac_ver[strcspn(mac_ver, "\r\n")] = 0;
        printf("[+] macOS Version: %s%s%s\n", COLOR_GREEN, mac_ver, COLOR_NC);
        free(mac_ver);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve macOS version\n" COLOR_NC);
    }

    if (mac_build) {
        mac_build[strcspn(mac_build, "\r\n")] = 0;
        printf("[+] Build Version: %s%s%s\n", COLOR_GREEN, mac_build, COLOR_NC);
        free(mac_build);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve build version\n" COLOR_NC);
    }

    if (mac_name) {
        mac_name[strcspn(mac_name, "\r\n")] = 0;
        printf("[+] Product Name: %s%s%s\n", COLOR_GREEN, mac_name, COLOR_NC);
        free(mac_name);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve product name\n" COLOR_NC);
    }

    if (mac_host) {
        mac_host[strcspn(mac_host, "\r\n")] = 0;
        printf("[+] Computer Name: %s%s%s\n", COLOR_GREEN, mac_host, COLOR_NC);
        free(mac_host);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve computer name\n" COLOR_NC);
    }

    if (mac_user) {
        mac_user[strcspn(mac_user, "\r\n")] = 0;
        printf("[+] User Name: %s%s%s\n", COLOR_GREEN, mac_user, COLOR_NC);
        free(mac_user);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve user name\n" COLOR_NC);
    }

    if (mac_model) {
        mac_model[strcspn(mac_model, "\r\n")] = 0;
        printf("[+] Model: %s%s%s\n", COLOR_GREEN, mac_model, COLOR_NC);
        free(mac_model);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve model\n" COLOR_NC);
    }

    if (mac_serial) {
        mac_serial[strcspn(mac_serial, "\r\n")] = 0;
        printf("[+] Serial Number: %s%s%s\n", COLOR_GREEN, mac_serial, COLOR_NC);
        free(mac_serial);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve serial number\n" COLOR_NC);
    }

    if (mac_uuid) {
        mac_uuid[strcspn(mac_uuid, "\r\n")] = 0;
        printf("[+] Hardware UUID: %s%s%s\n", COLOR_GREEN, mac_uuid, COLOR_NC);
        free(mac_uuid);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve hardware UUID\n" COLOR_NC);
    }

    if (mac_arch) {
        mac_arch[strcspn(mac_arch, "\r\n")] = 0;
        printf("[+] Architecture: %s%s%s\n", COLOR_GREEN, mac_arch, COLOR_NC);
        free(mac_arch);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve architecture\n" COLOR_NC);
    }

    if (mac_kernel) {
        mac_kernel[strcspn(mac_kernel, "\r\n")] = 0;
        printf("[+] Kernel Version: %s%s%s\n", COLOR_GREEN, mac_kernel, COLOR_NC);
        free(mac_kernel);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve kernel version\n" COLOR_NC);
    }

    if (mac_uptime) {
        mac_uptime[strcspn(mac_uptime, "\r\n")] = 0;
        printf("[+] Uptime: %s%s%s\n", COLOR_GREEN, mac_uptime, COLOR_NC);
        free(mac_uptime);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve uptime\n" COLOR_NC);
    }

    if (mac_load) {
        mac_load[strcspn(mac_load, "\r\n")] = 0;
        printf("[+] Load Average: %s%s%s\n", COLOR_GREEN, mac_load, COLOR_NC);
        free(mac_load);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve load average\n" COLOR_NC);
    }

    if (mac_cpu) {
        mac_cpu[strcspn(mac_cpu, "\r\n")] = 0;
        printf("[+] CPU Count: %s%s%s\n", COLOR_GREEN, mac_cpu, COLOR_NC);
        free(mac_cpu);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve CPU count\n" COLOR_NC);
    }

    if (mac_mem) {
        mac_mem[strcspn(mac_mem, "\r\n")] = 0;
        printf("[+] Memory Size: %s%s%s bytes\n", COLOR_GREEN, mac_mem, COLOR_NC);
        free(mac_mem);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve memory size\n" COLOR_NC);
    }

    if (mac_swap) {
        mac_swap[strcspn(mac_swap, "\r\n")] = 0;
        printf("[+] Swap Usage: %s%s%s\n", COLOR_GREEN, mac_swap, COLOR_NC);
        free(mac_swap);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve swap usage\n" COLOR_NC);
    }

    if (mac_disk) {
        mac_disk[strcspn(mac_disk, "\r\n")] = 0;
        printf("[+] Disk Space Available: %s%s%s\n", COLOR_GREEN, mac_disk, COLOR_NC);
        free(mac_disk);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve disk space\n" COLOR_NC);
    }

    if (mac_temp) {
        mac_temp[strcspn(mac_temp, "\r\n")] = 0;
        printf("[+] CPU Thermal Level: %s%s%s\n", COLOR_GREEN, mac_temp, COLOR_NC);
        free(mac_temp);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve CPU thermal level\n" COLOR_NC);
    }

    if (mac_battery) {
        mac_battery[strcspn(mac_battery, "\r\n")] = 0;
        printf("[+] Battery Percentage: %s%s%s\n", COLOR_GREEN, mac_battery, COLOR_NC);
        free(mac_battery);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve battery percentage\n" COLOR_NC);
    }

    if (mac_network) {
        mac_network[strcspn(mac_network, "\r\n")] = 0;
        printf("[+] Network IP Address: %s%s%s\n", COLOR_GREEN, mac_network, COLOR_NC);
        free(mac_network);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve network IP address\n" COLOR_NC);
    }

    if (mac_gateway) {
        mac_gateway[strcspn(mac_gateway, "\r\n")] = 0;
        printf("[+] Gateway IP Address: %s%s%s\n", COLOR_GREEN, mac_gateway, COLOR_NC);
        free(mac_gateway);
    } else {
        fprintf(stderr, COLOR_RED "Error: Unable to retrieve gateway IP address\n" COLOR_NC);
    }


#ifdef __linux__
    char* linux_ver = execute_command("cat /etc/issue | head -n 1");
    char* linux_host = execute_command("hostname");
    char* linux_user = execute_command("whoami");
    char* linux_model = execute_command("uname -m");
         -r");
    char* linux_uptime = execute_command("uptime -p");
    char* linux_load = execute_command("uptime | awk '{print $10 $11 $12}'");
    char* linux_cpu = execute_command("nproc");
    char* linux_mem = execute_command("free -h | awk '/Mem:/ {print $2}'");
    char* linux_swap = execute_command("free -h | awk '/Swap:/ {print $2}'");
    char* linux_disk = execute_command("df -h / | awk '// {print $4}'");
    char* linux_temp = execute_command("sensors | grep 'Package id 0' | awk '{print $4}'");
    char* linux_battery = execute_command("acpi -b | grep 'Battery 0' | awk '{
        print $4}'");
    char* linux_network = execute_command("hostname -I | awk '{print $1}'");
    char* linux_gateway = execute_command("ip route | grep 'default' | awk '{print $3}'");

    if (linux_ver) {
        // Remove newlines
        linux_ver[strcspn(linux_ver, "\r\n")] = 0;
        printf("Linux Distribution: %s%s%s\n", COLOR_GREEN, linux_ver, COLOR_NC);
        // Conditional prints with formatting
    if (linux_ver) {
        linux_ver[strcspn(linux_ver, "\r\n")] = 0;
        printf("[+] Linux Distribution: %s%s%s\n", COLOR_GREEN, linux_ver, COLOR_NC);
        free(linux_ver);
    }

if (linux_host) {
    linux_host[strcspn(linux_host, "\r\n")] = 0;
    printf("[+] Hostname: %s%s%s\n", COLOR_GREEN, linux_host, COLOR_NC);
    free(linux_host);
}

if (linux_user) {
    linux_user[strcspn(linux_user, "\r\n")] = 0;
    printf("[+] Current User: %s%s%s\n", COLOR_GREEN, linux_user, COLOR_NC);
    free(linux_user);
}

if (linux_model) {
    linux_model[strcspn(linux_model, "\r\n")] = 0;
    printf("[+] System Model: %s%s%s\n", COLOR_GREEN, linux_model, COLOR_NC);
    free(linux_model);
}

if (linux_arch) {
    linux_arch[strcspn(linux_arch, "\r\n")] = 0;
    printf("[+] Architecture: %s%s%s\n", COLOR_GREEN, linux_arch, COLOR_NC);
    free(linux_arch);
}

if (linux_kernel) {
    linux_kernel[strcspn(linux_kernel, "\r\n")] = 0;
    printf("[+] Kernel Version: %s%s%s\n", COLOR_GREEN, linux_kernel, COLOR_NC);
    free(linux_kernel);
}

if (linux_uptime) {
    linux_uptime[strcspn(linux_uptime, "\r\n")] = 0;
    printf("[+] Uptime: %s%s%s\n", COLOR_GREEN, linux_uptime, COLOR_NC);
    free(linux_uptime);
}

if (linux_load) {
    linux_load[strcspn(linux_load, "\r\n")] = 0;
    printf("[+] Load Average: %s%s%s\n", COLOR_GREEN, linux_load, COLOR_NC);
    free(linux_load);
}

if (linux_cpu) {
    linux_cpu[strcspn(linux_cpu, "\r\n")] = 0;
    printf("[+] CPU Cores: %s%s%s\n", COLOR_GREEN, linux_cpu, COLOR_NC);
    free(linux_cpu);
}

if (linux_mem) {
    linux_mem[strcspn(linux_mem, "\r\n")] = 0;
    printf("[+] Total Memory: %s%s%s\n", COLOR_GREEN, linux_mem, COLOR_NC);
    free(linux_mem);
}

if (linux_swap) {
    linux_swap[strcspn(linux_swap, "\r\n")] = 0;
    printf("[+] Total Swap: %s%s%s\n", COLOR_GREEN, linux_swap, COLOR_NC);
    free(linux_swap);
}

if (linux_disk) {
    linux_disk[strcspn(linux_disk, "\r\n")] = 0;
    printf("[+] Disk Space Available: %s%s%s\n", COLOR_GREEN, linux_disk, COLOR_NC);
    free(linux_disk);
}

if (linux_temp) {
    linux_temp[strcspn(linux_temp, "\r\n")] = 0;
    printf("[+] CPU Temperature: %s%s%s\n", COLOR_GREEN, linux_temp, COLOR_NC);
    free(linux_temp);
}

if (linux_battery) {
    linux_battery[strcspn(linux_battery, "\r\n")] = 0;
    printf("[+] Battery Level: %s%s%s\n", COLOR_GREEN, linux_battery, COLOR_NC);
    free(linux_battery);
}

if (linux_network) {
    linux_network[strcspn(linux_network, "\r\n")] = 0;
    printf("[+] Network IP Address: %s%s%s\n", COLOR_GREEN, linux_network, COLOR_NC);
    free(linux_network);
}

if (linux_gateway)
    {
        linux_gateway[strcspn(linux_gateway, "\r\n")] = 0;
        printf("[+] Default Gateway: %s%s%s\n", COLOR_GREEN, linux_gateway, COLOR_NC);
        free(linux_gateway);
    }
}
    #endif
#endif
    //
    // char* load = execute_command(
    //     #ifdef _WIN32
    //         "wmic cpu get loadpercentage"
    //     #else
    //         "cat /proc/loadavg | awk '{print $1, $2, $3}'"
    //     #endif
    // );
    //
    // char* cpu = execute_command(
    //     #ifdef _WIN32
    //         "wmic cpu get name"
    //     #else
    //         "cat /proc/cpuinfo | grep 'model name' | uniq"
    //     #endif
    // );
    //
    // char* mem = execute_command(
    //     #ifdef _WIN32
    //         "wmic OS get FreePhysicalMemory /Value"
    //     #else
    //         "free -h | grep Mem | awk '{print $2}'"
    //
    //     #else
    //         "uptime
    //     #else
    //         "hostname"
    //
    //     #endif
    // );

    // if (hostname) {
    //     // Remove newlines
    //     hostname[strcspn(hostname, "\r\n")] = 0;
    //     printf("Hostname: %s%s%s\n", COLOR_GREEN, hostname, COLOR_NC);
    //     free(hostname);
    // }

    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

    pause_me();
}

// Function to handle root check
int check_is_root() {
    #ifdef _WIN32
        // Windows implementation
        char* output = execute_command("net session >nul 2>&1 && echo admin || echo not_admin");
        if (output == NULL) {
            return 0;
        }

        int is_admin = (strstr(output, "admin") != NULL);
        free(output);
        return is_admin;
    #else
        // Unix-like systems
        return (geteuid() == 0);
    #endif
}

// Function to display about information
void show_about() {
    clear_screen();

    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);
    printf("%s%s ABOUT THIS TOOL %s\n", COLOR_MAGENTA, COLOR_BOLD, COLOR_NC);
    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

    printf("%sNetwork Tool v1.0%s\n\n", COLOR_BOLD, COLOR_NC);
    printf("This is a cross-platform network utility tool that provides\n");
    printf("functionality for managing network interfaces, IP addresses,\n");
    printf("and MAC addresses on Windows, Linux, and macOS.\n\n");

    printf("%sFeatures:%s\n", COLOR_CYAN, COLOR_NC);
    printf("- Display network information\n");
    printf("- Manage IP addresses (view, renew)\n");
    printf("- Manage MAC addresses (view, change, revert)\n");
    printf("- Display system information\n\n");

    printf("%sNote:%s Some operations require administrative/root privileges.\n", COLOR_YELLOW, COLOR_NC);

    printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

    pause_me();
}

// Main function
int main(int argc, char* argv[])
{
    // Check for command-line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "info") == 0) {
            show_network_info();
            return 0;
        } else if (strcmp(argv[1], "renew") == 0) {
            renew_ip();
            return 0;
        } else if (strcmp(argv[1], "change") == 0) {
            char* interface = get_default_interface();
            char* new_mac = change_mac(interface);
            if (new_mac) {
                printf("New MAC address: %s%s%s\n", COLOR_GREEN, new_mac, COLOR_NC);
                free(new_mac);
            }
            free(interface);
            return 0;
        } else if (strcmp(argv[1], "revert") == 0) {
            char* interface = get_default_interface();
            revert_mac(interface);
            free(interface);
            return 0;
        } else if (strcmp(argv[1], "about") == 0) {
            show_about();
            return 0;
        }
    }

    // Check if running as root/admin for certain operations
    int is_root = check_is_root();
    if (!is_root) {
        printf("%sWarning: Some operations may require administrative/root privileges.%s\n", COLOR_YELLOW, COLOR_NC);
        printf("%sConsider running this tool with elevated privileges.%s\n\n", COLOR_YELLOW, COLOR_NC);
    }

    int choice;

    while (1) {
        clear_screen();

        // Display ASCII art header
        printf("%s", COLOR_CYAN);
        printf(" _   _      _                      _      _____           _ \n");
        printf("| \\ | | ___| |___      _____  _ __| | __ |_   _|__   ___ | |\n");
        printf("|  \\| |/ _ \\ __\\ \\ /\\ / / _ \\| '__| |/ /   | |/ _ \\ / _ \\| |\n");
        printf("| |\\  |  __/ |_ \\ V  V / (_) | |  |   <    | | (_) | (_) | |\n");
        printf("|_| \\_|\\___|\\__| \\_/\\_/ \\___/|_|  |_|\\_\\   |_|\\___/ \\___/|_|\n");
        printf("%s\n", COLOR_NC);

        printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);
        printf("%s%s MAIN MENU %s\n", COLOR_BOLD, COLOR_BLUE, COLOR_NC);
        printf("%s%s%s\n", COLOR_GREEN, SEPARATOR, COLOR_NC);

        printf("1. %sNetwork Information%s\n", COLOR_CYAN, COLOR_NC);
        printf("2. %sIP Address Operations%s\n", COLOR_CYAN, COLOR_NC);
        printf("3. %sMAC Address Operations%s\n", COLOR_CYAN, COLOR_NC);
        printf("4. %sSystem Information%s\n", COLOR_CYAN, COLOR_NC);
        printf("5. %sAbout%s\n", COLOR_CYAN, COLOR_NC);
        printf("6. %sExit%s\n", COLOR_RED, COLOR_NC);

        printf("\n%sDetected OS: %s%s%s\n", COLOR_YELLOW, COLOR_GREEN, OS_TYPE, COLOR_NC);
        printf("%sRunning as %s%s%s\n", COLOR_YELLOW,
               is_root ? COLOR_GREEN : COLOR_RED,
               is_root ? "Administrator/Root" : "Regular User",
               COLOR_NC);

        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer on invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        // Consume newline
        getchar();

        switch (choice) {
        case 1:
            show_network_info();
            break;
        case 2:
            ip_address_operations();
            break;
        case 3:
            mac_address_operations();
            break;
        case 4:
            system_information();
            break;
        case 5:
            show_about();
            break;
        }
    }
};