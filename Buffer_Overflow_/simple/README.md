Sure, here's the reformatted text as a README.md file:


# Memory Hog (advanced version) 
The provided code attempts to perform stack overflow operations and allocate a significant amount of space in the heap. Here's a breakdown of the functions and their roles:

## `main()` Function

It runs an infinite loop, repeatedly calling `memory_hog()` with a large size parameter, `10,000,000,000` (10 billion), to allocate memory.

## `memory_hog(int size)` Function

It intends to allocate memory on the heap using `malloc` and `calloc`. However, there are some issues:

- `malloc` is called with the `size` parameter, which allocates memory but leaves it uninitialized.
- The `calloc` call is not completed, so it will result in a syntax error.
- Memory allocated by `malloc` and `calloc` is not freed, leading to potential memory leaks.

## `mainRun()` Function

It runs an infinite loop, repeatedly calling the `main()` function.

## Consequences

- **System Instability**: The program will eventually consume all available RAM and swap space.
- **Performance Degradation**: The system may become unresponsive or terminate processes to free memory.
- **Crash**: The program (or system) will crash when memory is fully exhausted.

## Usage (For Educational Purposes Only)

### Compile

```bash
gcc memory_hog.c -o memory_hog
```

### Run (Caution: May freeze/crash your system)

```bash
./memory_hog
```

## Warnings

- **Do not run this code on production systems or critical machines.**
- **Test in a controlled environment (e.g., virtual machine).**