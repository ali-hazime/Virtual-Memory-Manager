#include <stdint.h>

typedef struct {
    uint8_t page;
    uint8_t offset;
} address;

typedef struct {
    int frame_size;
    signed char *bytes;
} frame;

typedef struct {
    int mem_size;
    frame *frames;
} memory;

typedef struct {
    int page;
    int frame;      // note: this is redundant for page tables as ith entry's frame
                    // is always i but required for TLB. We could just implement
                    // the page table to be an array of integers but it's highly unnecessary
                    // for this task to make two different structs.
    int last_used;  // for LRU
} page_table_entry;

typedef struct {
    int size;
    page_table_entry *entries;
} page_table;

typedef struct {
    int count;
    int tlb_hit;
    int page_fault;
    int mem_access;
} statistics;