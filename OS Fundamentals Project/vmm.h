#include <stdio.h>
#include "structs.h"

address *read_addresses(FILE *stream, int n);

memory *create_memory(int memsize, int framesize);
void free_memory(memory* mem);
page_table *create_page_table(int size);
void free_page_table(page_table* ptable);

address translate(address logical, page_table *TLB, page_table *ptable, memory* mem, statistics *stats);
int replace_page(int page, int frame, memory *mem);
void print_lookup(address logical, address physical, memory* mem);