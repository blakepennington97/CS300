// Pennington.Jesse
// Doesn't use page replacements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define FRAME_SIZE
#define FRAME_COUNT 256

int page_table[PAGE_TABLE_SIZE][2];
int TLB[16][2];
int physical_memory[256][256];
int frame_count = 0;
int available_frame = 0;
int available_page = 0;
int page_count = 0;
int page_fault_count = 0;
int TLB_count = 0;
int TLB_hits = 0;
int TLB_hit = 0; //false
int clock = 0;
int timer[FRAME_COUNT];
FILE *backing_store;

void insertTLB(int page_num, int frame_num) {
    int i;

    for (i = 0; i < TLB_count; i++) {
        if (TLB[i][0] == page_num) break;
    }

    if (TLB_count >= TLB_SIZE) TLB_count = 0;

    TLB[TLB_count][0] = page_num;
    TLB[TLB_count][1] = frame_num;
    TLB_count++;
}



void readBackingStore(int page_num) {
    //seek from beginning of bin
    if (fseek(backing_store, page_num * 256, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking BACKING_STORE.bin\n");
    }

    signed char temp[256];
    //read 256 bytes from bin
    if (fread(temp, sizeof(signed char), 256, backing_store) == 0) {
        fprintf(stderr, "Error reading BACKING_STORE.bin\n");
    }

    // protect against out of bounds frame number
    if (available_frame >= FRAME_COUNT) available_frame = 0;
    
    int i;
    for (i = 0; i < 256; i++) {
        physical_memory[available_frame][i] = temp[i];
    }

    // is this FOR loop necessary????
    for (i = 0; i < PAGE_TABLE_SIZE; i++) {
        if (page_table[i][1] == available_frame) {
            page_table[i][1] = -1;
        }
    }



    page_table[available_page][0] = page_num;
    page_table[available_page][1] = available_frame;
    available_frame++;
    available_page++;

}


void producePageNumber(int logical_address, int *page_number, int *offset) {
    //printf("page#: %d offset: %d\n", page_num, offset);
    printf("Virtual address: %d ", logical_address);
    *page_number = ((logical_address & 0xFFFF) >> 8);
    *offset = logical_address & 0xFF;
}

void consultPageTable(int page_num, int offset) {
    int frame_number = -1;
    TLB_hit = 0;
    int i;
    // attempt to obtain frame number from TLB
    for (i = 0; i < TLB_SIZE; i++) {
        if (TLB[i][0] == page_num) {
            //printf("TLB HIT\n");
            frame_number = TLB[i][1];
            TLB_hits++;
            TLB_hit = 1;
            break;
        }
    }

    // if not found in TLB, attempt to obtain frame number from the page table
    if (frame_number == -1) {
        for (i = 0; i <= available_page; i++) {
            if (page_table[i][0] == page_num) {
                frame_number = page_table[i][1];
                //printf("TLB MISS w/o NO PAGE FAULT\n");
            }
        }
    }
    
     // if still not found, read from backing store and page fault & FIFO
    if (frame_number == -1) {
        readBackingStore(page_num);
        page_fault_count++;
        frame_number = available_frame - 1;
        //printf("TLB MISS w/ PAGE FAULT\n");
    }

    // insert page number and frame into TLB
    if (!TLB_hit) insertTLB(page_num, frame_number);
    int physical_address = (frame_number << 8) | offset;

    clock++;
    timer[frame_number] = clock;

    printf("Physical address: %d Value: %d\n", physical_address, physical_memory[frame_number][offset]);
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

    //init page_table
    // int i;
    // for (i = 0; i < 256; i++) {
    //     page_table[i] = -1;
    // }
    int addresses_counted = 0;
    char address[10]; //used to contain a single address
    int logical_address; //used to contain the int version of address
    int page_num, offset;
    while (fgets(address, 10, fp) != NULL) {
        logical_address = atoi(address);
        producePageNumber(logical_address, &page_num, &offset);
        consultPageTable(page_num, offset);
        addresses_counted++;
        printf("%d\n", addresses_counted);
    }

    printf("Number of Translated Addresses = %d\n", addresses_counted);
    printf("Page Faults = %d\n", page_fault_count);
    float page_fault_rate = (double)page_fault_count / addresses_counted;
    printf("Page Fault Rate = %.3f\n", page_fault_rate);
    printf("TLB Hits = %d\n", TLB_hits);
    float TLB_hit_rate = (double)TLB_hits / addresses_counted;
    printf("TLB Hit Rate = %.3f\n", TLB_hit_rate);

    fclose(fp);
    fclose(backing_store);
    return 0;
}