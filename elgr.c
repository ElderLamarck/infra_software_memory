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
    char bin[256];
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
    if(argc < 4){
        printf("quantidade insuficiente de argumentos\n");
        exit(-1);
    }
    if(strcmp(argv[2], "lru") != 0 && strcmp(argv[2], "fifo") != 0){
        printf("argumento de ordenação da PageTable fora do padrão ('lru' ou 'fifo')\n");
        exit(-1);
    }
    else if(strcmp(argv[3], "lru") != 0 && strcmp(argv[3], "fifo") != 0){
        printf("argumento de ordenação da TLB fora do padrão ('lru' ou 'fifo')\n");
        exit(-1);
    }

    int decimal, count = 0, pageFault = 0;
    int time = 0, lock = 0, found = NOT_IN_MEMORY;
    int countTLB = 0, hitTLB = 0, lock2 = 0;

    FILE *file_txt = fopen(argv[1] , "r");
    FILE *file_bin = fopen("BACKING_STORE.bin", "r");
    FILE *file_correct = fopen("correct.txt", "w");
    fpos_t position;

    if(file_txt == NULL){
        printf("arquivo de texto com os endereços não pode ser aberto\n");
        exit(-1);
    }
    fgetpos(file_txt, &position);

    for(int i = 0; i < SIZE_TLB; i++){
        tlb[i].page = -1;
    }

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
                break;
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
                fseek(file_bin, (page * 256), SEEK_SET);
                fread(memoryStack[pageTable[page].indexMemory].bin, 256, 1,file_bin);
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

        fprintf(file_correct,"Virtual address: %d ", decimal);
        fprintf(file_correct,"Physical address: %d ",(((pageTable[page].indexMemory) * 256) + offset));
        fprintf(file_correct,"Value: %d\n",memoryStack[pageTable[page].indexMemory].bin[offset]);
    }
    fprintf(file_correct,"Number of Translated Addresses = %d\n", time);
    fprintf(file_correct,"Page Faults = %d\n", pageFault);
    fprintf(file_correct,"Page Fault Rate = %0.3f\n", (float)pageFault/time);
    fprintf(file_correct,"TLB Hits = %d\n", hitTLB);
    fprintf(file_correct,"TLB Hit Rate = %0.3f\n", (float)hitTLB/time);

    fclose(file_txt);
    fclose(file_bin); 
    fclose(file_correct); 
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
