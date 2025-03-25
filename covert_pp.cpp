#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "gates.hpp"

struct cache_line{
    char vals[64];
} typedef cache_line;

cache_line eviction_set[256];

void prime(){
    for(int i = 0; i < 256; i++){
        maccess(&eviction_set[i]);
    }
}
void probe(char* shared){
    for(int i = 0; i < 256; i++){
        amplification1(eviction_set[i].vals,shared+(i*4096));
    }

}
void flush_shared(char* shared){
    for(int i = 0; i < 256; i++){
        flush(shared+(i*4096));
    }
}
void reload_shared(char* shared){
    for(int i = 0; i < 256; i++){
        maccess(shared+(i*4096));
    }
}
int main(){
    // mmap a shared region of 256 pages
    char* shared = (char*)mmap(0, 4096*256, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    int pid = fork();
    if(pid == 0){
        while(1){
            prime();
            probe(shared);
        }
    }else{
        // parent process
        // read from the shared region
        for(int i = 0; i < 256; i++){
            if(shared[i*4096] != 1){
                std::cout << "Error: shared region is not shared" << std::endl;
                return 1;
            }
        }
    }

}