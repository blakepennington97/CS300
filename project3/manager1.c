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




void producePageNumber(int logical_address) {
    printf("Translating logical address: %d\n", logical_address);
    int page_number = ((logical_address & 0xFFFF) >> 8);
    int offset = logical_address & 0xFF;
    printf("page#: %d offset: %d\n", page_number, offset);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Incorrect usage, please format such as: ./manager1.c addresses.txt\n");
        exit(0);
    }

    // open addresses.txt
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening %s\n", argv[1]);
    }

    char address[12]; //used to contain a single address
    int logical_address; //used to contain the int version of address
    while (fgets(address, 12, fp) != NULL) {
        logical_address = atoi(address);
        producePageNumber(logical_address);
    }



    return 0;
}