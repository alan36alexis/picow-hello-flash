#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// Definir el offset para almacenar datos (256 KB desde el inicio, como en los ejemplos)
#define FLASH_TARGET_OFFSET (256 * 1024)
#define FLASH_SECTOR_SIZE (1u << 12) // 4 KB por sector

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

// Función para guardar un valor de 32 bits en una posición específica dentro del sector
bool save_variable_at_position(uint32_t value, uint32_t position) {
    // Validar que la posición esté dentro del sector y permita escribir 4 bytes
    if (position > (FLASH_SECTOR_SIZE - 4)) {
        printf("Error: Posición %u fuera del rango del sector (0 a %u)\n", position, FLASH_SECTOR_SIZE - 4);
        return false;
    }
    // Asegurarse de que la posición esté alineada a 4 bytes (para uint32_t)
    if (position % 4 != 0) {
        printf("Error: Posición %u no está alineada a 4 bytes\n", position);
        return false;
    }

    // Leer el contenido actual del sector
    uint8_t buffer[FLASH_SECTOR_SIZE];
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
        buffer[i] = flash_target_contents[i];
    }

    // Modificar el buffer con el nuevo valor en la posición especificada
    buffer[position] = (value >> 24) & 0xFF;
    buffer[position + 1] = (value >> 16) & 0xFF;
    buffer[position + 2] = (value >> 8) & 0xFF;
    buffer[position + 3] = value & 0xFF;

    // Deshabilitar interrupciones para escritura segura
    uint32_t interrupts = save_and_disable_interrupts();

    // Borrar el sector
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    // Escribir el buffer modificado en la Flash
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_SECTOR_SIZE);

    // Restaurar interrupciones
    restore_interrupts(interrupts);

    return true;
}

// Función para leer un valor de 32 bits desde una posición específica
uint32_t read_variable_at_position(uint32_t position) {
    // Validar que la posición esté dentro del sector y permita leer 4 bytes
    if (position > (FLASH_SECTOR_SIZE - 4)) {
        printf("Error: Posición %u fuera del rango del sector (0 a %u)\n", position, FLASH_SECTOR_SIZE - 4);
        return 0;
    }
    // Asegurarse de que la posición esté alineada a 4 bytes
    if (position % 4 != 0) {
        printf("Error: Posición %u no está alineada a 4 bytes\n", position);
        return 0;
    }

    // Leer los 4 bytes desde la posición especificada
    return (flash_target_contents[position] << 24) |
           (flash_target_contents[position + 1] << 16) |
           (flash_target_contents[position + 2] << 8) |
           flash_target_contents[position + 3];
}

void run_example_1(void) {
    printf("Ejemplo 1: Guardar y leer un valor de 32 bits en la Flash\n");

    // Esperar a que la conexión USB se estabilice
    sleep_ms(2000);

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
}
void run_example_2(void) {
    printf("Ejemplo 2: Guardar y leer un valor de 32 bits en una posición específica\n");

    // Esperar a que la conexión USB se estabilice
    sleep_ms(2000);

    // Leer y verificar los valores en diferentes posiciones
    printf("Leyendo valores en posiciones 0 y 4 antes de guardar\n");
    uint32_t value1 = read_variable_at_position(0);
    printf("Valor leído en posición 0: %u\n", value1);

    uint32_t value2 = read_variable_at_position(4);
    printf("Valor leído en posición 4: %u\n", value2);

    // Ejemplo: Guardar valores en diferentes posiciones dentro del sector
    printf("Guardando valor n1 en posición 0\n");
    value1++;
    save_variable_at_position(value1, 0);

    printf("Guardando valor n2 en posición 4\n");
    value2++;
    save_variable_at_position(value2, 4);

    // Leer y verificar los valores
    sleep_ms(1000);
    value1 = read_variable_at_position(0);
    printf("Valor leído en posición 0: %u\n", value1);

    value2 = read_variable_at_position(4);
    printf("Valor leído en posición 4: %u\n", value2);

    // Ejemplo de error: posición inválida
    printf("Intentando guardar en posición inválida 4096\n");
    save_variable_at_position(9999, 4096); // Debería fallar

    // Ejemplo de error: posición no alineada
    printf("Intentando guardar en posición no alineada 2\n");
    save_variable_at_position(9999, 2); // Debería fallar
}

int main() {
    stdio_init_all();

    run_example_1();
    //run_example_2();

    while (true) {
        sleep_ms(1000);
    }
}

