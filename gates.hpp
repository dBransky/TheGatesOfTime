#ifndef GATES_H
#define GATES_H

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

void not_gate(char* in, char* out);
void xor_gate(char* in1, char* in2, char* out);
void nor_gate(char* in1, char* in2, char* out);
void nand_gate(char* in1, char* in2, char* out);
void and_gate(char* in1, char* in2, char* out);
void or_gate(char* in1, char* in2, char* out);
void amplification2(char* in, char* out1, char* out2);
void amplification1(char* in, char* out);
void run_nbt_not_gate(char* in, char* out);
void adder(char* in1, char* in2, char* carry, char* sum, char* carry_out);
#endif // GATES_H