#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "vmm.h"

#define LOOKUPS 1000
#define BACKING_STORE "BACKING_STORE.bin"

#define TLB_TIME 1
#define DISK_TIME 100
#define MEM_TIME 5

int backing_store_fd;

int main(int argc, char* argv[]) {
    FILE *f = fopen(argv[1], "r");

    // verify file exists
    if (!f) {
        perror("vmm: could not open input file");
        return EXIT_FAILURE;
    }

    // load addresses from file
    address *addr = read_addresses(f, LOOKUPS);
    fclose(f);

    // initialize backing store
    backing_store_fd = open(BACKING_STORE, O_RDONLY);

    // create memory
    memory* mem = create_memory(128, 256);

    // create page table and TLB
    page_table *ptable = create_page_table(128);
    page_table *TLB = create_page_table(16);

    // initialize statistics
    statistics stats;
    stats.count = 0;
    stats.page_fault = 0;
    stats.tlb_hit = 0;
    stats.mem_access = 0;

    // translate
    for (int i = 0; i < LOOKUPS; i++) {
        address physical = translate(addr[i], TLB, ptable, mem, &stats);
        
        // print
        stats.mem_access++;
        print_lookup(addr[i], physical, mem);
    }

    // some stats
    float tlb_hitrate = (float) stats.tlb_hit / stats.count;
    float pf_rate = (float) stats.page_fault / stats.count;
    float ptable_hitrate = 1 - tlb_hitrate - pf_rate;
    int eta = stats.count * TLB_TIME +
                stats.mem_access * MEM_TIME + 
                stats.page_fault * DISK_TIME;

    printf("Number of Translated Addresses = %d\n", stats.count);
    printf("Page Faults = %d\n", stats.page_fault);
    printf("Page Fault Rate = %.3f\n", pf_rate);
    printf("TLB Hits = %d\n", stats.tlb_hit);
    printf("TLB Hit Rate = %.3f\n", tlb_hitrate);
    printf("Total Memory Accesses(Inverted Page Table) = %d\n", stats.mem_access);
    printf("Average Access Time: %d cycles\n", eta);

    // cleanup
    close(backing_store_fd);
    free(addr);
    free_memory(mem);
    free_page_table(ptable);
    free_page_table(TLB);

    return EXIT_SUCCESS;
}