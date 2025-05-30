#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// Definir el offset para almacenar datos (256 KB desde el inicio, como en los ejemplos)
#define FLASH_TARGET_OFFSET (256 * 1024)
#define FLASH_SECTOR_SIZE (4 * 1024) // 4 KB por sector

// Dirección absoluta en la Flash
const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

void save_variable(uint32_t value) {
    // Preparar el buffer con el valor a guardar
    uint8_t buffer[FLASH_SECTOR_SIZE] = {0};
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;

    // Deshabilitar interrupciones para escritura segura
    uint32_t interrupts = save_and_disable_interrupts();

    // Borrar el sector
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    // Escribir el valor en la Flash
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_SECTOR_SIZE);

    // Restaurar interrupciones
    restore_interrupts(interrupts);
}

uint32_t read_variable() {
    // Leer los primeros 4 bytes desde la Flash
    return (flash_target_contents[0] << 24) |
           (flash_target_contents[1] << 16) |
           (flash_target_contents[2] << 8) |
           flash_target_contents[3];
}

int main() {
    stdio_init_all();

    // Leer el valor inicial
    uint32_t my_variable = read_variable();
    printf("Valor leído de la Flash: %u\n", my_variable);

    // Incrementar y guardar
    my_variable++;
    printf("Guardando nuevo valor: %u\n", my_variable);
    save_variable(my_variable);

    // Verificar lectura después de guardar
    sleep_ms(1000);
    printf("Valor leído después de guardar: %u\n", read_variable());

    while (true) {
        sleep_ms(1000);
    }
}