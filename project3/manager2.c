// Pennington.Jesse
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int virtual_addresses_from_file[65536];
int masked_virtual_addresses[65536];
int page_number_and_offset[2][65536];
int page_table[256];
int TLB[2][16];
int physical_memory[256][256];
