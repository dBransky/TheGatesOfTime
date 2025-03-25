#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "gates.hpp"
#include <stdarg.h>

#define MIN_CACHE_MISS_CYCLES (200)
#define TRAIN_ITER 5000000
#define SLOW_DOWN 7
#define SPEC_SLOW_PARAM 70
#define SLOW_DOWN_NOR 7
#define SLOW_DOWN_NAND 7
void _nbt_not_gate (int counter, char* addr_in, char* addr_out) {
    switch ((counter&7)+(*addr_in) ) {
    case 0x0 : if (counter == 0x0) return; break ;
    case 0x1 : if (counter == 0x1) return; break ;
    case 0x2 : if (counter == 0x2) return; break ;
    case 0x3 : if (counter == 0x3) return; break ;
    case 0x4 : if (counter == 0x4) return; break ;
    case 0x5 : if (counter == 0x5) return; break ;
    case 0x6 : if (counter == 0x6) return; break ;
    case 0x7 : if (counter == 0x7) return; break ;
    }
    volatile int dummy = 0;
    for ( int i = 0; i < SPEC_SLOW_PARAM ; i ++)
        dummy *= dummy ;
    maccess(addr_out);
    return;
}
void run_nbt_not_gate (char* in, char* out) {
    static int counter = 0;
    _nbt_not_gate (counter, in, out) ;
    counter ++;
    counter%=8;
}

void _not(char wet_run,char* in,int outs,...){
    va_list args;
    va_start(args, outs);
    for ( int i = 0; i < 128; i ++) { asm volatile ( " " ) ;}
    if (wet_run==*in){
        return;}
    if(!wet_run){
        return;
        }
    for(int i=0;i<SLOW_DOWN;i++){
        asm volatile("nop");
    }
    for(int i=0;i<outs;i++){
        maccess(va_arg(args, char*));
    }
}
void not_gate(char* in,char* out){
    char train_in;
    char train_out;
    train_in = 1;
    train_out = 1;
    for (int i = 0; i < TRAIN_ITER; i++){
        _not(0,&train_in,1,&train_out);
    }
    _not(1,in,1,out);
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

void amplification2(char* in,char* out1,char* out2){
    char* temp = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    temp[0] = 1;
    flush(temp);
    not_gate(in,temp);
    char train_in;
    char train_out1;
    char train_out2;
    train_in = 1;
    train_out1 = 1;
    train_out2 = 1;
    for (int i = 0; i < TRAIN_ITER; i++){
        _not(0,&train_in,2,&train_out1,&train_out2);
    }
    _not(1,temp,2,out1,out2);
}
void amplification1(char* in,char* out){
    char train_in;
    char train_out;
    train_in = 1;
    train_out = 1;
    for (int i = 0; i < TRAIN_ITER; i++){
        _not(0,&train_in,1,&train_out);
    }
    _not(1,in,1,out);
}
void and_gate(char* in1,char* in2,char* out){
    char* temp = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    temp[0] = 1;
    flush(temp);
    nand_gate(in1,in2,temp);
    not_gate(temp,out);
}
void or_gate(char* in1,char* in2,char* out){
    char* temp = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    temp[0] = 1;
    flush(temp);
    nor_gate(in1,in2,temp);
    not_gate(temp,out);
}
void xor_gate(char* in1,char* in2,char* out){
    char* in1_dup1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in1_dup2 = in1_dup1 + 1024;
    char* not_in1 = in1_dup2 + 1024;
    char* in2_dup1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2_dup2 = in2_dup1 + 1024;
    char* not_in2 = in2_dup2 + 1024;
    in1_dup1[0] = 1;
    in1_dup2[0] = 1;
    not_in1[0] = 1;
    in2_dup1[0] = 1;
    in2_dup2[0] = 1;
    not_in2[0] = 1;
    flush(in1_dup1);
    flush(in1_dup2);
    flush(not_in1);
    flush(in2_dup1);
    flush(in2_dup2);
    flush(not_in2);
    amplification2(in1,in1_dup1,in1_dup2);
    // size_t time = rdtsc();
    // maccess(in1_dup1);
    // size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    // time = rdtsc();
    // maccess(in1_dup2);
    // delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    amplification2(in2,in2_dup1,in2_dup2);
    // printf("delta: %ld\n", delta);
    not_gate(in1_dup1,not_in1);
    not_gate(in2_dup1,not_in2);
    flush(in1_dup1);
    flush(in2_dup1);
    and_gate(not_in1,in2_dup2,in1_dup1);
    and_gate(not_in2,in1_dup2,in2_dup1);
    or_gate(in1_dup1,in2_dup1,out);
}
void adder(char* in1,char* in2,char* carry,char* sum,char* carry_out){
    char* dup1_in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* dup2_in1 = dup1_in1 + 1024;
    char* dup1_in2 = dup2_in1 + 1024;
    char* dup2_in2 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* dup1_carry = dup2_in2 + 1024;
    char* dup2_carry = dup1_carry + 1024;
    char* aXORb = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* aXORb_dup1 = aXORb + 1024;
    char* aXORb_dup2 = aXORb_dup1 + 1024;
    dup1_in1[0] = 1;
    dup2_in1[0] = 1;
    dup1_in2[0] = 1;
    dup2_in2[0] = 1;
    dup1_carry[0] = 1;
    dup2_carry[0] = 1;
    aXORb[0] = 1;
    aXORb_dup1[0] = 1;
    aXORb_dup2[0] = 1;
    flush(dup1_in1);
    flush(dup2_in1);
    flush(dup1_in2);
    flush(dup2_in2);
    flush(dup1_carry);
    flush(dup2_carry);
    flush(aXORb);
    flush(aXORb_dup1);
    flush(aXORb_dup2);
    amplification2(in1,dup1_in1,dup2_in1);
    amplification2(in2,dup1_in2,dup2_in2);
    amplification2(carry,dup1_carry,dup2_carry);
    xor_gate(dup1_in1,dup1_in2,aXORb);
    amplification2(aXORb,aXORb_dup1,aXORb_dup2);
    xor_gate(aXORb_dup1,dup1_carry,sum);
    and_gate(aXORb_dup2,dup1_carry,dup1_carry);
    and_gate(dup2_in1,dup2_in2,aXORb);
    or_gate(aXORb,dup2_carry,carry_out);
}
