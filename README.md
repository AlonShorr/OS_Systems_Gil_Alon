# OS Systems Projects

A collection of systems programming projects in C/C++, focused on process management, synchronization, and memory allocation.

## Included Projects
- Smash shell — mini Unix shell with job control and signal handling
- Concurrent bank simulator — multithreaded ATM/bank system with synchronization primitives
- Custom allocator — malloc/free-style allocator using sbrk/brk
  
## Build
Each project can be built separately:

### Smash
```bash
cd 1.Smash
make
```
### Bank
```bash
cd 2.bank
make
```
### Custom Allocator
```bash
cd 3.Custom_Allocator
gcc -Wall -Wextra main.c customAllocator.c -o allocator_test
```
