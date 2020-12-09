#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "vmm.h"

extern int backing_store_fd;
// read addresses to query into an array for ease
address *read_addresses(FILE *stream, int n) {    
    address *addr = malloc(sizeof(address) * n);
    int i = 0;

    // read lines until EOF
    char line[20];
    while (fgets(line, 19, stream)) {
        // convert to 16 bit after masking the rightmost(little endian) 16-bits only
        uint16_t l = (uint16_t) strtol(line, NULL, 10) & 0xffff;
        
        // separate page and offset
        addr[i].page = l >> 8;
        addr[i].offset = l & 0x00ff;

        i++;
    }

    return addr;
}

memory *create_memory(int memsize, int framesize) {
    // allocate container
    memory *mem = malloc(sizeof(memory));
    mem->mem_size = memsize;

    // allocate frames containers
    mem->frames = malloc(sizeof(frame) * memsize);

    // allocate respective frames iteratively
    for (int i = 0; i < memsize; i++) {
        mem->frames[i].frame_size = framesize;
        mem->frames[i].bytes = malloc(framesize);
    }
        
    return mem;
}
void free_memory(memory* mem) {
    // free individual frames
    for (int i = 0; i < mem->mem_size; i++) {
        free(mem->frames[i].bytes);
    }

    // free frame array
    free(mem->frames);

    // free memory container
    free(mem);
}

page_table *create_page_table(int size) {
    // allocate page table container
    page_table *ptable = malloc(sizeof(ptable));
    ptable->size = size;

    // allocate array for size entries
    ptable->entries = malloc(sizeof(page_table_entry) * size);

    // initialize all page entries to -1 to indicate empty/invalid
    for (int i = 0; i < size; i++)
        ptable->entries[i].page = -1;

    return ptable;
}
void free_page_table(page_table* ptable) {
    free(ptable->entries);
    free(ptable);
}

address translate(address logical, page_table *TLB, page_table *ptable, memory* mem, statistics *stats) {
    address physical;
    physical.offset = logical.offset;   // offset doesn't change

    stats->count++;

    int minTLB_lu = stats->count;
    int TLB_index = 0;
    int minptable_lu = stats->count;
    int ptable_index = 0;

    // check for TLB hit
    for (int i = 0; i < TLB->size; i++) {
        // TLB hit
        if (TLB->entries[i].page == logical.page) {
            stats->tlb_hit++;
            
            // translate
            physical.page = TLB->entries[i].frame;
            
            // update TLB
            TLB->entries[i].last_used = stats->count;
            // update page table
            ptable->entries[physical.page].last_used = stats->count;

            return physical;
        }

        // can find out TLB index to replace right now too
        if (TLB->entries[i].last_used < minTLB_lu) {
            minTLB_lu = TLB->entries[i].last_used;
            TLB_index = i;
        }
            
    }

    // check page table
    for (int i = 0; i < ptable->size; i++) {
        stats->mem_access++;
        // page hit
        if (ptable->entries[i].page == logical.page) {
            // translate
            physical.page = i;

            // update TLB
            TLB->entries[TLB_index].page = ptable->entries[i].page;
            TLB->entries[TLB_index].frame = i;
            TLB->entries[TLB_index].last_used = stats->count;
            
            // update page table
            ptable->entries[i].last_used = stats->count;
            
            return physical;
        }

        // can find out LRU index here too
        if (ptable->entries[i].last_used < minptable_lu) {
            minptable_lu = ptable->entries[i].last_used;
            ptable_index = i;
        }
            
    }

    // page fault
    stats->page_fault++;
    physical.page = replace_page(logical.page, ptable_index, mem);

    // update page table
    ptable->entries[ptable_index].page = logical.page;
    ptable->entries[ptable_index].last_used = stats->count;

    // update TLB
    TLB->entries[TLB_index].page = logical.page;
    TLB->entries[TLB_index].frame = ptable_index;
    TLB->entries[TLB_index].last_used = stats->count;

    return physical;
}

int replace_page(int page, int frame, memory *mem) {
    // replace page into mem
    lseek(backing_store_fd, page * mem->frames[frame].frame_size, SEEK_SET);
    read(backing_store_fd, mem->frames[frame].bytes, mem->frames[frame].frame_size);

    return frame;
}

void print_lookup(address logical, address physical, memory* mem) {
    int logical_int, physical_int;

    logical_int = (logical.page << 8) + logical.offset;
    physical_int = (physical.page << 8) + physical.offset;

    signed char val = mem->frames[physical.page].bytes[physical.offset];
    
    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_int, physical_int, val); 
}