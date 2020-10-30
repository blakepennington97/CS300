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
int frame_count = 0;
int page_count = 0;
int page_fault_count = 0;
FILE *backing_store;




int producePageNumber(int logical_address) {
    printf("Virtual address: %d\n", logical_address);
    int page_number = ((logical_address & 0xFFFF) >> 8);
    int offset = logical_address & 0xFF;
    printf("page#: %d offset: %d\n", page_number, offset);
    return page_number, offset;
}

void getFromPageTable(int page_num, int offset) {
    int frame_number = -1;
    //navigate page_table to see if frame exists
    if (page_table[page_num]) {
        frame_number = page_table[page_num];
    }
    // else if empty, page fault, so read from backing store
    else if (page_table[page_num] == NULL) { //page fault
        readBackingStore(page_num);
        frame_number = frame_count - 1;
        page_fault_count++;
    }
    printf("Page faults: %d\n", page_fault_count);
}

void readBackingStore(int page_num) {
    signed char temp[256];
    //seek from beginning of bin
    if (fseek(backing_store, page_num * 256, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking BACKING_STORE.bin\n");
    }
    //read 256 bytes from bin
    if (fread(temp, sizeof(signed char), 256, backing_store) == 0) {
        fprintf(stderr, "Error reading BACKING_STORE.bin\n");
    }
    
    int i;
    for (i = 0; i < 256; i++) {
        physical_memory[frame_count][i] = temp[i];
    }

    page_table[page_count] = frame_count;
    frame_count++;
    page_count++;
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

    //open BACKING_STORE.bin
    backing_store = fopen("BACKING_STORE.bin", "rb");
    if (backing_store == NULL) {
        fprintf(stderr, "Error opening BACKING_STORE.bin\n");
        exit(0);
    }

    char address[12]; //used to contain a single address
    int logical_address; //used to contain the int version of address
    int page_num, offset;
    int i = 0;
    while (fgets(address, 12, fp) != NULL) {
        printf("%d\n", i);
        i++;
        logical_address = atoi(address);
        page_num, offset = producePageNumber(logical_address);
        getFromPageTable(page_num, offset);
    }


    fclose(fp);
    fclose(backing_store);
    return 0;
}