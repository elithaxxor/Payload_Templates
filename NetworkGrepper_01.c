#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <time.h>

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
    }

    #ifdef _WIN32
        _pclose(fp);
    #else
        pclose(fp);
    #endif

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
    #elif defined(__APPLE__)
        char* mac_ver = execute_command("sw_vers -productVersion");
        if (mac_ver) {
            // Remove newlines
            mac_ver[strcspn(mac_ver, "\r\n")] = 0;
            printf("macOS Version: %s%s%s\n", COLOR_GREEN, mac_ver, COLOR_NC);
            free(mac_ver);
        }
    #else
        char* linux_ver = execute_command("cat /etc/issue | head -n 1");
        if (linux_ver) {
            // Remove newlines
            linux_ver[strcspn(linux_ver, "\r\n")] = 0;
            printf("Linux Distribution: %s%s%s\n", COLOR_GREEN, linux_ver, COLOR_NC);
            free(linux_ver);
        }
    #endif

    char* hostname = execute_command(
        #ifdef _WIN32
            "hostname"
        #else
            "hostname"
        #endif
    );

    if (hostname) {
        // Remove newlines
        hostname[strcspn(hostname, "\r\n")] = 0;
        printf("Hostname: %s%s%s\n", COLOR_GREEN, hostname, COLOR_NC);
        free(hostname);
    }

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
}
