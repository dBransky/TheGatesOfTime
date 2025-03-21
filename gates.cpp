#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <map>
#include <vector>
#include "gates.hpp"

#define MIN_CACHE_MISS_CYCLES (200)
#define TRAIN_ITER 2000000
#define SLOW_DOWN 7
#define SLOW_DOWN_NOR 7
#define SLOW_DOWN_NAND 7

void _not(char wet_run,char* in,char* out){
    for ( int i = 0; i < 128; i ++) { asm volatile ( " " ) ;}
    if (wet_run==*in){
        return;}
    if(!wet_run){
        return;
        }
    for(int i=0;i<SLOW_DOWN;i++){
        asm volatile("nop");
    }
    maccess(out);
}
void not_gate(char* in,char* out){
    char train_in;
    char train_out;
    train_in = 1;
    train_out = 1;
    for (int i = 0; i < TRAIN_ITER; i++){
        _not(0,&train_in,&train_out);
    }
    _not(1,in,out);
}
void _nor(char wet_run,char* in1,char* in2,char* out){
    for ( int i = 0; i < 128; i ++) { asm volatile ( " " ) ;}
    if (wet_run==*in1)
        return;
    if (wet_run==*in2)
        return;
    if(!wet_run){
        return;
        }
    for(int i=0;i<SLOW_DOWN_NOR;i++){
        asm volatile("nop");
    }
    maccess(out);
}
void nor_gate(char* in1,char* in2,char* out){
    char train_in1;
    char train_in2;
    char train_out;
    train_in1 = 1;
    train_in2 = 1;
    train_out = 1;
    for (int i = 0; i < TRAIN_ITER; i++){
        _nor(0,&train_in1,&train_in2,&train_out);
    }
    _nor(1,in1,in2,out);
}
void _nand(char wet_run,char* in1,char* in2,char* out){
    for ( int i = 0; i < 128; i ++) { asm volatile ( " " ) ;}
    if (wet_run==*in1+*in2)
        return;
    if(!wet_run){
        return;
        }
    for(int i=0;i<SLOW_DOWN_NAND;i++){
        asm volatile("nop");
    }
    maccess(out);
}
void nand_gate(char* in1,char* in2,char* out){
    char train_in1;
    char train_in2;
    char train_out;
    train_in1 = 1;
    train_in2 = 1;
    train_out = 1;
    for (int i = 0; i < TRAIN_ITER; i++){
        _nand(0,&train_in1,&train_in2,&train_out);
    }
    _nand(2,in1,in2,out);
}