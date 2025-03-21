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
bool test_nand(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* out = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    out[0] = 1;
    flush(out);
    in1[0] = 1;
    in2[0] = 1;
    nand_gate(in1, in2, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    nand_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    maccess(in2);
    nand_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta< MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in2);
    maccess(in1);
    nand_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    munmap(in2, 4096);
    munmap(out, 4096);
    return success;

}
bool test_nor(){
    bool success = true;
    char* in1 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* in2 = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* out = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    out[0] = 1;
    flush(out);
    in1[0] = 1;
    in2[0] = 1;
    nor_gate(in1, in2, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    nor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    maccess(in2);
    nor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in2);
    maccess(in1);
    nor_gate(in1, in2, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in1);
    flush(in2);
    munmap(in1, 4096);
    munmap(in2, 4096);
    munmap(out, 4096);
    return success;
}
bool test_not(){
    bool success = true;
    char* in = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    char* out = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    out[0] = 1;
    flush(out);
    in[0] = 1;
    not_gate(in, out);
    size_t time = rdtsc();
    maccess(out);
    size_t delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta >= MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in);
    not_gate(in, out);
    time = rdtsc();
    maccess(out);
    delta = rdtsc() - time;
    printf("delta: %ld\n", delta);
    _assert(delta < MIN_CACHE_MISS_CYCLES,&success);
    flush(out);
    flush(in);
    munmap(in, 4096);
    munmap(out, 4096);
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
    int success_count = 0;

    for (int i = 0; i < n; i++) {
        if (test_func()) {
            success_count++;
        }
    }

    double accuracy = (double)success_count / n * 100;
    if (accuracy >= p) {
        printf("\033[0;32m"); // Set text color to green
        printf("Test name: %s, Accuracy: %.2f%%\n", test_name, accuracy);
        printf("\033[0m"); // Reset text color
    } else {
        printf("\033[0;31m"); // Set text color to red
        printf("Test name: %s, Accuracy: %.2f%%\n", test_name, accuracy);
        printf("\033[0m"); // Reset text color
    }    
    assert(accuracy >= p);
}

void warmup(){
    for (int i = 0; i < ITER_WARMUP; i++) {
    test_get_value();
    // test_not();
    // test_nor();
    // test_nand();
    }
}
int main() {
    warmup();
    regression_test(test_get_value, 10000, 95,"test_get_value");
    regression_test(test_not, 10, 70,"test_not");
    regression_test(test_nor, 10, 70,"test_nor");
    regression_test(test_nand, 10, 70,"test_nand");
    printf("\033[0;32m"); // Set text color to green
    printf("All tests passed\n");
    printf("\033[0m"); // Reset text color
    return 0;
}