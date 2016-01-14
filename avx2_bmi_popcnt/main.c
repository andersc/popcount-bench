// Bench counting the number of set bits ('1') per every 64 bits using POPCNT,BMI2 and AVX2.
// Made by Anders Cedronius 2014 (anders.cedronius (you know what) gmail.com)

#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <math.h>
#include <string.h>
#include "CPUID.h"

#define DISPLAY_HEIGHT  4
#define DISPLAY_WIDTH   32
#define NUM_DATA_OBJECTS  40000000
#define ITTERATIONS 20

//Declare ASM functions
void popcnt_popcnt(long long unsigned[],unsigned int,long long unsigned[]);
void popcnt_bmi2(long long unsigned[],unsigned int,long long unsigned[]);
void popcnt_avx2(long long unsigned[],unsigned int,long long unsigned[],unsigned char[]);

// The source data (+32 to avoid the quantization out of memory problem)
__attribute__ ((aligned(32))) static uint64_t data[NUM_DATA_OBJECTS+32]={};
//__attribute__ ((aligned(32))) static uint64_t data_out[NUM_DATA_OBJECTS+32]={};
__declspec(align(32)) static uint64_t data_out[NUM_DATA_OBJECTS+32]={};

__attribute__ ((aligned(32))) static unsigned char k1[32*3]={
    0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
    0x00,0x01,0x01,0x02,0x01,0x02,0x02,0x03,0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04,0x00,0x01,0x01,0x02,0x01,0x02,0x02,0x03,0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

//Fill all data_objects with random data.
void populate_data()
{
    for(unsigned int i = 0; i < NUM_DATA_OBJECTS; i++)
    {
        data[i] = rand();
    }
}

//For debugging look at the source data
void display_source_data()
{
    printf ("\r\nData in(start):\r\n");
    for (unsigned int j = 0; j < DISPLAY_HEIGHT; j++)
    {
        for (unsigned int i = 0; i < DISPLAY_WIDTH; i++)
        {
            printf ("0x%02llx,",data[i+(j*DISPLAY_WIDTH)]);
        }
        printf ("\r\n");
    }
}

//For debugging look at the produced data
void display_dest_data()
{
    printf ("\r\nData out:\r\n");
    for (unsigned int j = 0; j < DISPLAY_HEIGHT; j++)
    {
        for (unsigned int i = 0; i < DISPLAY_WIDTH; i++)
        {
            printf ("0x%02llx,",data_out[i+(j*DISPLAY_WIDTH)]);
        }
        printf ("\r\n");
    }
}

//Calculate the average result, skips the first run.
long calc_avrg( long* elapsed)
{
    long avrg=0;
    for (unsigned int i = 1; i < ITTERATIONS; i++){
        avrg+=elapsed[i];
    }
    return avrg/(ITTERATIONS-1.0);
}

//POPCNT
void bench_popcnt()
{
    for(unsigned int i = 0; i < NUM_DATA_OBJECTS; i++)
    {
        data_out[i] = _mm_popcnt_u64(data[i]);
    }
}

//Just move the data
void bench_move_data_memcpy()
{
    memcpy(data_out,data,NUM_DATA_OBJECTS*8);
}


//BMI2
// (_tzcnt_u64) is not declared in x86intrin.h LLVM using __tzcnt_u64 instead.
void bench_bmi2()
{
    for(unsigned int i = 0; i < NUM_DATA_OBJECTS; i++)
    {
        //This routine does not produce the same output as the other methods (values are all wrong).
        data_out[i]=__tzcnt_u64(~_pext_u64(data[i],data[i]));
    }
}

//The popcnt asm function
void asm_popcnt()
{
    popcnt_popcnt(data,NUM_DATA_OBJECTS,data_out);
}

//The BMI2 asm function
void asm_bmi2()
{
    popcnt_bmi2(data,NUM_DATA_OBJECTS,data_out);
}

//The AVX2 asm function
void asm_avx2()
{
    popcnt_avx2(data,(unsigned int)ceil((NUM_DATA_OBJECTS*8)/32.0),data_out,k1);
}

//The benchmarking function

void bench_me(void (*f)(),long* elapsed,char* text)
{
    struct timeval t0,t1;
    for (unsigned int i = 0; i < ITTERATIONS; i++)
    {
        populate_data();
        gettimeofday(&t0, 0);
        (*f)();
        gettimeofday(&t1, 0);
        elapsed[i]= (((t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec)/1000);
        printf ("%s %ld\n",text,elapsed[i]);
    }
}

//Bench all functions

int main() {
    
    long elapsed[ITTERATIONS]={0};
    
    bench_me(bench_move_data_memcpy,&elapsed[0],"Time_to_move_data_without_processing:");
    printf ("Average time_to_move_data: %ld\n",calc_avrg(&elapsed[0]));
    
    bench_me(bench_popcnt,&elapsed[0],"popcnt:");
    printf ("Average popcnt: %ld\n",calc_avrg(&elapsed[0]));
    
    bench_me(bench_bmi2,&elapsed[0],"bmi2:");
    printf ("Average bmi2: %ld\n",calc_avrg(&elapsed[0]));
    
    bench_me(asm_popcnt,&elapsed[0],"popcnt_asm:");
    printf ("Average popcnt_asm: %ld\n",calc_avrg(&elapsed[0]));
    
    bench_me(asm_bmi2,&elapsed[0],"bmi2_asm:");
    printf ("Average bmi2_asm: %ld\n",calc_avrg(&elapsed[0]));
    
    bench_me(asm_avx2,&elapsed[0],"avx2_asm:");
    printf ("Average avx2_asm: %ld\n",calc_avrg(&elapsed[0]));
    
    return 0;
}


