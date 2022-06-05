#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define SIZE_PAGE_TABLE 256
#define SIZE_MEMORY 128
#define SIZE_TLB 16

#define IN_MEMORY 1
#define NOT_IN_MEMORY 0

typedef struct page_table_args{
	int indexMemory;
	int isCreated;
    int uses;
}Page_table_args;

typedef struct memoryStack{
    char bin[SIZE_MEMORY];
} MemoryStack;

typedef struct tlb{
    int page;
    int uses;
} TLB;


Page_table_args pageTable[SIZE_PAGE_TABLE];
MemoryStack memoryStack[SIZE_MEMORY];
TLB tlb[SIZE_TLB];


unsigned int getOffset(int decimal);
unsigned int getPage(int decimal);


int main(int argc, char *argv[]){

    int decimal, count = 0, pageFault = 0;
    int time = 0, lock = 0, found = NOT_IN_MEMORY;
    int countTLB = 0, hitTLB = 0, lock2 = 0;

    FILE *file_txt;
    FILE *file_bin;
    fpos_t position;

    file_txt = fopen(argv[1] , "r");
    file_bin = fopen("BACKING_STORE.bin", "r");
    fgetpos(file_txt, &position);


    while(fscanf(file_txt, "%d", &decimal)!= EOF){  
        int page = getPage(decimal);
        int offset = getOffset(decimal);
        time++;
        found = NOT_IN_MEMORY;

        for(int i = 0; i < SIZE_TLB; i++){
            if(tlb[i].page == page){
                found = IN_MEMORY;
                pageTable[page].uses = time;
                tlb[i].uses = time;
                hitTLB++;
            }
        }

        if(found == NOT_IN_MEMORY){
            if(pageTable[page].isCreated == NOT_IN_MEMORY){
                pageFault++;


                if (count < SIZE_MEMORY && lock == 0){
                    pageTable[page].indexMemory = count;
                    count++; 
                }
                    
                else if(!strcmp(argv[2], "fifo")){
                    lock = 1;
                    if (count == SIZE_MEMORY){ 
                        count = 0;
                    }
                    for(int i = 0; i < SIZE_PAGE_TABLE; i++){
                        if(pageTable[i].isCreated == IN_MEMORY && pageTable[i].indexMemory == count){
                            pageTable[i].isCreated = NOT_IN_MEMORY; //na pageTable mas não aponta para memoria
                            pageTable[i].indexMemory = -1;
                        }
                    }
                    pageTable[page].indexMemory = count;
                    count++;
                }

                else if(!strcmp(argv[2], "lru")){
                    lock = 1;
                    int lower = time;
                    int value = page;

                    for(int i = 0; i < SIZE_PAGE_TABLE; i++){
                        if(pageTable[i].isCreated == IN_MEMORY && pageTable[i].uses < lower){
                            lower = pageTable[i].uses;
                            value = i;
                        }
                    }
                    pageTable[page].indexMemory = pageTable[value].indexMemory;
                    pageTable[value].indexMemory = -1;
                    pageTable[value].isCreated = NOT_IN_MEMORY;               
                }
                
                pageTable[page].isCreated = IN_MEMORY;
                time--;
                fsetpos(file_txt, &position);
                continue;

            }else{
                if(!strcmp(argv[3], "fifo")){
                    if(countTLB == SIZE_TLB){
                        countTLB = 0;
                    }
                }
                if(!strcmp(argv[3], "lru")){
                    if(countTLB == SIZE_TLB){
                        lock2 = 1;
                        int lowerTLB = time;
                        int position = 0;
                        
                        for(int i = 0; i < SIZE_TLB; i++){
                            if(tlb[i].uses < lowerTLB){
                                lowerTLB = tlb[i].uses;
                                position = i;
                            }
                        }
                        tlb[position].page = page;
                        tlb[position].uses = time;
                    }
                }
                if(lock2 == 0){
                    tlb[countTLB].page = page;
                    tlb[countTLB].uses = time;
                    countTLB++;
                }
                pageTable[page].uses = time;
            } 
        }
        fgetpos(file_txt, &position);
        fseek(file_bin, (page * 256), SEEK_SET);
        fread(memoryStack[pageTable[page].indexMemory].bin, 256, 1,file_bin);

        printf("Virtual address: %d Physical address: %d Value: %d\n", decimal, (((pageTable[page].indexMemory) * 256) + offset), memoryStack[pageTable[page].indexMemory].bin[offset]);
    }
    printf("Number of Translated Addresses = %d\n", time);
    printf("Page Faults = %d\n", pageFault);
    printf("Page Fault Rate = %0.3f\n", (float)pageFault/time);
    printf("TLB hits = %d\n", hitTLB);
    printf("TLB rate = %0.3f\n", (float)hitTLB/time);

    fclose(file_txt);
    fclose(file_bin);  
    return 0;
}

unsigned int getOffset(int decimal){
    unsigned int value = decimal;
    value = value << 24;
    value = value >> 24;
    return value;
}

unsigned int getPage(int decimal){
    unsigned int value = decimal;
    value = value << 16;
    value = value >> 24;
    return value;
}
