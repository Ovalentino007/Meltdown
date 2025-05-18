# Meltdown

Este repositorio contiene una prueba simplificada del ataque Meltdown, que permite acceder a datos privilegiados de memoria del kernel desde espacio de usuario mediante ejecución especulativa y un canal lateral basado en caché (Flush+Reload).


## ¿Qué es Meltdown?

Meltdown es una vulnerabilidad descubierta en 2018 que afecta a múltiples procesadores Intel. Permite a un proceso sin privilegios leer información protegida en memoria del kernel, aprovechando la ejecución fuera de orden y la ejecución especulativa del procesador.

---

## ¿Qué hace este exploit?

- Intenta leer una dirección de memoria privilegiada (kernel_addr) usando ejecución especulativa.
- Utiliza el canal lateral Flush+Reload para inferir el valor cacheado por acceso ilegal.
- Reconstruye el contenido de esa dirección secreta byte a byte.

---

## Requisitos

- CPU Intel vulnerable (anteriores a 8ª generación sin microcódigo actualizado).
- Kernel Linux vulnerable (sin mitigaciones como KPTI activadas).
- Virtualización o ejecución directa en entornos de laboratorio.
- Compilador `gcc` con soporte para intrinsics Intel.

---

## Compilación

gcc -O0 -o meltdown meltdown.c
