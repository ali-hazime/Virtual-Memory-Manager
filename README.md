# Virtual-Memory-Manager
A virtual memory manager program, this was the final project for an Operating Systems Fundamentals class.
The program translates logical to physical address for a virtual address space of 2<sub>16</sub> = 65,536 bytes. The program reads from a file containing logical
addresses and, using a TLB as well as a page table, translates each logical address to its corresponding physical address and outputs the value of the byte stored at the translated physical address.

How to Run:
To compile the code, use the command:
gcc main.c vmm.c

To run, type
./a.out addresses.txt

Optionally sending the output to out.txt by using
./a.out addresses.txt > out.txt
