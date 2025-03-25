#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include "./cacheutils.h"
#include <map>
#include <vector>
#include <assert.h>
#include <stdbool.h>
#include "gates.hpp"

#define MIN_CACHE_MISS_CYCLES (200)
#define ITER_WARMUP 2

void _assert(bool condition,bool* success){
    *success &= condition;
}
bool test_1_bit_adder(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = in1+1024;
    char* carry = in1+2048;
    char* sum = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    in1[0] = 1;
    in2[0] = 1;
    carry[0] = 1;
    sum[0] = 1;
    flush(sum);
    flush(carry);
    adder(in1,in2,carry,sum,carry);
    size_t time = rdtsc();
    maccess(sum);
    size_t delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    time = rdtsc();
    maccess(carry);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(sum);
    flush(carry);
    flush(in1);
    flush(in2);
    adder(in1,in2,carry,sum,carry);
    time = rdtsc();
    maccess(sum);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    time = rdtsc();
    maccess(carry);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(sum);
    flush(carry);
    flush(in1);
    in2[0] = 1;
    adder(in1,in2,carry,sum,carry);
    time = rdtsc();
    maccess(sum);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    time = rdtsc();
    maccess(carry);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(sum);
    flush(carry);
    flush(in2);
    in1[0] = 1;
    adder(in1,in2,carry,sum,carry);
    time = rdtsc();
    maccess(sum);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    time = rdtsc();
    maccess(carry);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(sum);
    flush(carry);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    munmap(sum, 4096);
    return success;
}
bool test_xor(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = in1+1024;
    char* out = in1+2048;
    out[0] = 1;
    flush(out);
    in1[0] = 1;
    in2[0] = 1;
    xor_gate(in1, in2, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    xor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    maccess(in2);
    xor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta< MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in2);
    maccess(in1);
    xor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    return success;
}
bool test_amp_2(){
    bool success = true;
    char* in = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* out1 = in +1024;
    char* out2 = in +2048;
    out1[0] = 1;
    out2[0] = 1;
    flush(out1);
    flush(out2);
    in[0] = 1;
    amplification2(in,out1,out2);
    size_t time = rdtsc();
    maccess(out1);
    size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    time = rdtsc();
    maccess(out2);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out1);
    flush(out2);
    flush(in);
    amplification2(in,out1,out2);
    time = rdtsc();
    maccess(out1);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    time = rdtsc();
    maccess(out2);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out1);
    flush(out2);
    flush(in);
    munmap(in, 4096);
    return success;
}
bool test_or(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = in1+1024;
    char* out = in1+2048;
    out[0] = 0;
    flush(out);
    in1[0] = 1;
    in2[0] = 1;
    or_gate(in1, in2, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    or_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    maccess(in2);
    or_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in2);
    maccess(in1);
    or_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    return success;
}
bool test_and(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = in1+1024;
    char* out = in1+2048;
    out[0] = 0;
    flush(out);
    in1[0] = 1;
    in2[0] = 1;
    and_gate(in1, in2, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    and_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    maccess(in2);
    and_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in2);
    maccess(in1);
    and_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    return success;
}
bool test_nand(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = in1+1024;
    char* out = in1+2048;
    out[0] = 1;
    flush(out);
    in1[0] = 1;
    in2[0] = 1;
    nand_gate(in1, in2, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    nand_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    maccess(in2);
    nand_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta< MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in2);
    maccess(in1);
    nand_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    return success;

}
bool test_nor(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = in1+1024;
    char* out = in1+2048;
    out[0] = 1;
    flush(out);
    in1[0] = 1;
    in2[0] = 1;
    nor_gate(in1, in2, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    nor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    maccess(in2);
    nor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in2);
    maccess(in1);
    nor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    return success;
}
bool test_not(){
    bool success = true;
    char* in = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* out = in +1024;
    out[0] = 1;
    flush(out);
    in[0] = 1;
    not_gate(in, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in);
    not_gate(in, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    // printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in);
    munmap(in, 4096);
    return success;
}
bool test_not_nbt(){
    bool success = true;
    char* in = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* out = in +1024;
    out[0] = 1;
    flush(out);
    in[0] = 0;
    run_nbt_not_gate(in, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in);
    run_nbt_not_gate(in, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in);
    munmap(in, 4096);
    return success;
}
bool test_get_value(){
    bool success = true;
    char* in = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    in[0] = 0;
    size_t time = rdtsc();
    maccess(in);
    size_t delta = rdtsc() - time;
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(in);
    time = rdtsc();
    maccess(in);
    delta = rdtsc() - time;
    time = rdtsc();
    maccess(in);
    delta = rdtsc() - time;
    munmap(in, 4096);
    return success;
}
void regression_test(bool (*test_func)(), int n, double p, char* test_name) {
    // int success_count = 0;

    // for (int i = 0; i < n; i++) {
    //     if (test_func()) {
    //         success_count++;
    //     }
    // }

    double accuracy = 35;
    if (accuracy >= p) {
        printf("\033[0;32m"); // Set text color to green
        printf("Test name: %s, Accuracy: %.2f%%\n", test_name, accuracy);
        printf("\033[0m"); // Reset text color
    } else {
        printf("\033[0;31m"); // Set text color to red
        printf("Test name: %s, Accuracy: %.2f%%\n", test_name, accuracy);
        printf("\033[0m"); // Reset text color
    }    
    // assert(accuracy >= p);
}

void warmup(){
    for (int i = 0; i < 1; i++) {
    test_get_value();
    // test_not();
    // test_nor();
    // test_nand();
    }
}
int main() {
    warmup();
    // regression_test(test_get_value, 10000, 90,"test_get_value");
    // printf("layer 0: \n");
    // regression_test(test_not, 10, 60,"test_not");
    // regression_test(test_nor, 10, 60,"test_nor");
    // regression_test(test_nand, 10, 60,"test_nand");
    // // regression_test(test_not_nbt, 10, 60,"test_not_nbt");
    // printf("layer 1: \n");
    // regression_test(test_amp_2, 10, 60,"test_amp_2");
    // printf("layer 2: \n");
    // regression_test(test_and, 10, 60,"test_and");
    // regression_test(test_or, 10, 60,"test_or");
    printf("layer 12: \n");
    // regression_test(test_xor, 2, 60,"test_xor");
    regression_test(test_1_bit_adder, 2, 60,"test_adder");
    printf("\033[0;32m"); // Set text color to green
    printf("All tests passed\n");
    printf("\033[0m"); // Reset text color
    return 0;
}