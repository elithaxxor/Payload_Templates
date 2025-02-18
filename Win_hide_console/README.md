'''
## ⚠️ IMPORTANT NOTES

- Duplicate `stdio.h` inclusion is redundant but harmless.
- `windows.h` includes many sub-components (50+ headers).
- Winsock requires initialization via `WSAStartup()`.
- Wininet vs Winsock: High-level vs low-level networking.
- `TCHAR`

'''

/*=======================================================
          🖥️ WINDOWS / C STANDARD LIBRARY IMPORTS
=======================================================*/


/*=======================================================
              ⚠️ IMPORTANT NOTES
=======================================================*/
// - Duplicate stdio.h inclusion is redundant but harmless
// - Windows.h includes many sub-components (50+ headers)
// - Winsock requires initialization via WSAStartup()
// - Wininet vs Winsock: High-level vs low-level networking
// - TCHAR helps create Unicode-aware applications

# Windows/C Standard Library Imports

## 🖥️ WINDOWS / C STANDARD LIBRARY IMPORTS

### Standard Libraries

- `#include <stdio.h>`
  - Standard Input/Output functions (printf, scanf, file I/O)
  - Example:
    
    ```c
    `#include <stdio.h>`
    int main() {
        printf("Hello, World!\n");
        return 0;
    }
    ```c
                    

- `#include <stdlib.h>`
  - General utilities: memory allocation (malloc), system commands, conversions
  - Example:
    ```c
    #include <stdlib.h>
    #include <stdio.h>

    int main() {
        int *arr = (int *)malloc(5 * sizeof(int));
        if (arr == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }
        for (int i = 0; i < 5; i++) {
            arr[i] = i * i;
            printf("arr[%d] = %d\n", i, arr[i]);
        }
        free(arr);
        return 0;
    }
    ```

- `#include <string.h>`
  - String manipulation: strlen, strcpy, memcpy, etc.
  - Example:
    ```c
    #include <string.h>
    #include <stdio.h>

    int main() {
        char str1[20] = "Hello";
        char str2[20];

        strcpy(str2, str1);
        printf("str2: %s\n", str2);

        strcat(str1, " World!");
        printf("str1: %s\n", str1);

        return 0;
    }
    ```

- `#include <sys/stat.h>`
  - File status operations (file size, permissions via stat() function)
  - Example:
    ```c
    #include <sys/stat.h>
    #include <stdio.h>

    int main() {
        struct stat st;
        if (stat("example.txt", &st) == 0) {
            printf("File size: %ld bytes\n", st.st_size);
        } else {
            printf("File not found\n");
        }
        return 0;
    }
    ```

- `#include <sys/types.h>`
  - System data types (used with stat.h and directory operations)
  - Example:
    ```c
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <stdio.h>

    int main() {
        struct stat st;
        if (stat("example.txt", &st) == 0) {
            printf("File size: %ld bytes\n", st.st_size);
        } else {
            printf("File not found\n");
        }
        return 0;
    }
    ```

## 🪟 WINDOWS API CORE IMPORTS

- `#include <windows.h>`
  - MAIN Windows API header (files, processes, memory, devices)
  - Example:
    ```c
    #include <windows.h>
    #include <stdio.h>

    int main() {
        MessageBox(NULL, "Hello, Windows!", "Hello", MB_OK);
        return 0;
    }
    ```

- `#include <tchar.h>`
  - Unicode/ANSI compatibility (TCHAR macros for cross-platform strings)
  - Example:
    ```c
    #include <windows.h>
    #include <tchar.h>

    int _tmain(int argc, _TCHAR* argv[]) {
        MessageBox(NULL, _T("Hello, Windows!"), _T("Hello"), MB_OK);
        return 0;
    }
    ```

- `#include <winuser.h>`
  - User Interface components (windows, messages, input handling)
  - Example:
    ```c
    #include <windows.h>
    #include <winuser.h>

    int main() {
        MessageBox(NULL, "Hello, Windows!", "Hello", MB_OK);
        return 0;
    }
    ```

- `#include <windowsx.h>`
  - Macros for cleaner WinAPI code (GET_X_LPARAM, HandleMessage macros)
  - Example:
    ```c
    #include <windows.h>
    #include <windowsx.h>
    #include <stdio.h>

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_LBUTTONDOWN:
                printf("Mouse clicked at: (%d, %d)\n", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    int main() {
        // Register and create window, message loop, etc.
        return 0;
    }
    ```

## 🌐 NETWORKING/WEB IMPORTS

- `#include <wininet.h>`
  - High-level internet operations (HTTP/FTP requests, URLs)
  - Example:
    ```c
    #include <wininet.h>
    #include <windows.h>
    #include <stdio.h>

    int main() {
        HINTERNET hInternet = InternetOpen("MyAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet) {
            HINTERNET hConnect = InternetOpenUrl(hInternet, "http://www.example.com", NULL, 0, 0, 0);
            if (hConnect) {
                printf("Successfully opened URL\n");
                InternetCloseHandle(hConnect);
            }
            InternetCloseHandle(hInternet);
        }
        return 0;
    }
    ```

- `#include <winsock2.h>`
  - Core networking (Winsock API v2, sockets, TCP/UDP)
  - Example:
    ```c
    #include <winsock2.h>
    #include <stdio.h>

    int main() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            printf("WSAStartup failed\n");
            return 1;
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            printf("Socket creation failed\n");
            WSACleanup();
            return 1;
        }

        // Use the socket...
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    ```

- `#include <ws2tcpip.h>`
  - Modern networking extensions (getaddrinfo, inet_pton)
  - Example:
    ```c
    #include <ws2tcpip.h>
    #include <stdio.h>

    int main() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            printf("WSAStartup failed\n");
            return 1;
        }

        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo("www.example.com", "80", &hints, &res) != 0) {
            printf("getaddrinfo failed\n");
            WSACleanup();
            return 1;
        }

        // Use the result...
        freeaddrinfo(res);
        WSACleanup();
        return 0;
    }
    ```

## 🛠️ ADDITIONAL WINDOWS COMPONENTS

- `#include <winbase.h>` (included via windows.h)
  - File operations, process/thread management
  - Example:
    ```c
    #include <windows.h>
    #include <winbase.h>
    #include <stdio.h>

    int main() {
        HANDLE hFile = CreateFile("example.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            printf("CreateFile failed\n");
            return 1;
        }

        char data[] = "Hello, file!";
        DWORD bytesWritten;
        WriteFile(hFile, data, sizeof(data), &bytesWritten, NULL);
        CloseHandle(hFile);
        return 0;
    }
    ```

- `#include <wingdi.h>` (included via windows.h)
  - Graphics Device Interface (drawing operations)
  - Example:
    ```c
    #include <windows.h>
    #include <wingdi.h>

    int main() {
        HDC hdc = GetDC(NULL);
        Rectangle(hdc, 50, 50, 200, 200);
        ReleaseDC(NULL, hdc);
        return 0;
    }
    ```

- `#include <windef.h>` (included via windows.h)
  - Windows data types and constants
  - Example:
    ```c
    #include <windows.h>
    #include <windef.h>

    int main() {
        HWND hwnd = GetConsoleWindow();
        if (hwnd != NULL) {
            ShowWindow(hwnd, SW_MINIMIZE);
        }
        return 0;
    }
    ```

// ● #include <stdio.h>
//   - Standard Input/Output functions (printf, scanf, file I/O)
//   - Included twice (line 1 and 3) but harmless due to include guards

// ● #include <stdlib.h>
//   - General utilities: memory allocation (malloc), system commands, conversions

// ● #include <string.h>
//   - String manipulation: strlen, strcpy, memcpy, etc.

// ● #include <sys/stat.h>
//   - File status operations (file size, permissions via stat() function)

// ● #include <sys/types.h>
//   - System data types (used with stat.h and directory operations)


/*=======================================================
               🪟 WINDOWS API CORE IMPORTS
=======================================================*/

// ● #include <windows.h>
//   - MAIN Windows API header (files, processes, memory, devices)

// ● #include <tchar.h>
//   - Unicode/ANSI compatibility (TCHAR macros for cross-platform strings)

// ● #include <winuser.h>
//   - User Interface components (windows, messages, input handling)

// ● #include <windowsx.h>
//   - Macros for cleaner WinAPI code (GET_X_LPARAM, HandleMessage macros)


/*=======================================================
               🌐 NETWORKING/WEB IMPORTS
=======================================================*/

// ● #include <wininet.h>
//   - High-level internet operations (HTTP/FTP requests, URLs)

// ● #include <winsock2.h>
//   - Core networking (Winsock API v2, sockets, TCP/UDP)

// ● #include <ws2tcpip.h>
//   - Modern networking extensions (getaddrinfo, inet_pton)


/*=======================================================
              🛠️ ADDITIONAL WINDOWS COMPONENTS
=======================================================*/

// ● #include <winbase.h> (included via windows.h)
//   - File operations, process/thread management

// ● #include <wingdi.h> (included via windows.h)
//   - Graphics Device Interface (drawing operations)

// ● #include <windef.h> (included via windows.h)
//   - Windows data types and constants
