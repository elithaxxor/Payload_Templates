

/*=======================================================
          🖥️ WINDOWS / C STANDARD LIBRARY IMPORTS
=======================================================*/

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


/*=======================================================
              ⚠️ IMPORTANT NOTES
=======================================================*/
// - Duplicate stdio.h inclusion is redundant but harmless
// - Windows.h includes many sub-components (50+ headers)
// - Winsock requires initialization via WSAStartup()
// - Wininet vs Winsock: High-level vs low-level networking
// - TCHAR helps create Unicode-aware applications
