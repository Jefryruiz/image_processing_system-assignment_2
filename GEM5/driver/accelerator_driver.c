/**
 * accelerator_driver.c
 *
 * Driver del acelerador RGB->Gris para GEM5 modo SE.
 *
 * En modo SE de GEM5 no hay acceso a memoria fisica.
 * El acelerador se modela como conversion software BT.601
 * dentro del espacio del proceso, representando la interaccion
 * con el hardware acelerador via el flujo de datos.
 *
 * Flujo:
 *   1. Leer imagen RGB desde archivo
 *   2. Escribir en buffer de entrada (simula escritura a RAM)
 *   3. Configurar parametros del acelerador (simula escritura de registros)
 *   4. Ejecutar conversion (simula procesamiento del acelerador)
 *   5. Leer resultado (simula lectura de RAM)
 *   6. Guardar imagen de salida
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Parametros de imagen                                                */
/* ------------------------------------------------------------------ */
#define IMG_WIDTH    1920
#define IMG_HEIGHT   1080
#define IMG_CHANNELS 3
#define IMG_SIZE_RGB  (IMG_WIDTH * IMG_HEIGHT * IMG_CHANNELS)
#define IMG_SIZE_GRAY (IMG_WIDTH * IMG_HEIGHT)
#define TOTAL_PIXELS  (IMG_WIDTH * IMG_HEIGHT)

/* ------------------------------------------------------------------ */
/* Registros del acelerador (estructuras en memoria del proceso)       */
/* Modelan el mapa de registros del hardware                          */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t base_in;      /* 0x00: direccion base imagen entrada */
    uint32_t base_out;     /* 0x04: direccion base imagen salida  */
    uint32_t num_pixels;   /* 0x08: total de pixeles              */
    uint32_t control;      /* 0x0C: escribir 1 = iniciar          */
    uint32_t status;       /* 0x10: 0=ocupado, 1=listo            */
} AccelRegs;

/* ------------------------------------------------------------------ */
/* Buffers — simulan las regiones de RAM del sistema                  */
/* ------------------------------------------------------------------ */
static uint8_t  ram_input [IMG_SIZE_RGB];   /* @ INPUT_IMG_ADDR  */
static uint8_t  ram_output[IMG_SIZE_GRAY];  /* @ OUTPUT_IMG_ADDR */
static AccelRegs accel_regs = {0};          /* @ ACCEL_BASE      */

/* ------------------------------------------------------------------ */
/* Acelerador: conversion BT.601 fila por fila                        */
/* Modela el SC_THREAD process_image() del hardware                   */
/* ------------------------------------------------------------------ */
static void accel_process(void) {
    uint8_t *base_in  = (uint8_t *)(uintptr_t)accel_regs.base_in;
    uint8_t *base_out = (uint8_t *)(uintptr_t)accel_regs.base_out;

    for (int y = 0; y < IMG_HEIGHT; y++) {
        uint8_t *row_rgb  = base_in  + y * IMG_WIDTH * IMG_CHANNELS;
        uint8_t *row_gray = base_out + y * IMG_WIDTH;

        for (int x = 0; x < IMG_WIDTH; x++) {
            uint8_t r = row_rgb[x * 3 + 0];
            uint8_t g = row_rgb[x * 3 + 1];
            uint8_t b = row_rgb[x * 3 + 2];
            /* BT.601: gray = 0.299R + 0.587G + 0.114B */
            row_gray[x] = (uint8_t)(
                (299 * r + 587 * g + 114 * b) / 1000);
        }
    }
    accel_regs.status = 1;
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    const char *input_path  = (argc > 1) ? argv[1] : "input.raw";
    const char *output_path = (argc > 2) ? argv[2] : "output.raw";

    printf("========================================\n");
    printf(" ARM64 GEM5 Accelerator Driver\n");
    printf(" RGB -> Grayscale (BT.601)\n");
    printf("========================================\n");

    /* ---- Paso 1: Cargar imagen en RAM (buffer ram_input) ---- */
    printf("[Driver] Paso 1: Cargando imagen desde '%s'...\n", input_path);
    FILE *f = fopen(input_path, "rb");
    if (!f) {
        printf("[Driver] ERROR: no se pudo abrir '%s'\n", input_path);
        return -1;
    }
    size_t n = fread(ram_input, 1, IMG_SIZE_RGB, f);
    fclose(f);
    if (n != IMG_SIZE_RGB) {
        printf("[Driver] ERROR: leidos %zu de %d bytes\n", n, IMG_SIZE_RGB);
        return -1;
    }
    printf("[Driver] Imagen cargada en RAM: %zu bytes\n", n);

    /* ---- Paso 2: Configurar registros del acelerador ---- */
    printf("[Driver] Paso 2: Configurando acelerador...\n");
    accel_regs.base_in    = (uint32_t)(uintptr_t)ram_input;
    accel_regs.base_out   = (uint32_t)(uintptr_t)ram_output;
    accel_regs.num_pixels = TOTAL_PIXELS;
    printf("[Driver]   REG_BASE_IN    = 0x%08X\n", accel_regs.base_in);
    printf("[Driver]   REG_BASE_OUT   = 0x%08X\n", accel_regs.base_out);
    printf("[Driver]   REG_NUM_PIXELS = %u\n",     accel_regs.num_pixels);

    /* ---- Paso 3: Iniciar acelerador ---- */
    printf("[Driver] Paso 3: Iniciando acelerador (CONTROL=1)...\n");
    accel_regs.control = 1;
    accel_process();  /* modelo del SC_THREAD process_image() */

    /* ---- Paso 4: Esperar status ---- */
    printf("[Driver] Paso 4: Status = %u\n", accel_regs.status);

    /* ---- Paso 5: Verificar BT.601 en primeros 10 pixeles ---- */
    printf("[Driver] Paso 5: Verificando conversion BT.601...\n");
    int mismatches = 0;
    for (int i = 0; i < 10; i++) {
        uint8_t r = ram_input[i*3+0];
        uint8_t g = ram_input[i*3+1];
        uint8_t b = ram_input[i*3+2];
        uint8_t expected = (uint8_t)((299*r + 587*g + 114*b) / 1000);
        uint8_t got      = ram_output[i];
        int diff = (int)expected - (int)got;
        if (diff < -1 || diff > 1) {
            printf("[Driver] ERROR pixel %d: esp=%d got=%d\n",
                   i, expected, got);
            mismatches++;
        }
    }

    /* ---- Paso 6: Guardar imagen de salida ---- */
    printf("[Driver] Paso 6: Guardando resultado en '%s'...\n", output_path);
    FILE *fo = fopen(output_path, "wb");
    if (fo) {
        fwrite(ram_output, 1, IMG_SIZE_GRAY, fo);
        fclose(fo);
        printf("[Driver] Imagen guardada: %d bytes\n", IMG_SIZE_GRAY);
    } else {
        printf("[Driver] WARN: no se pudo guardar '%s'\n", output_path);
    }

    printf("========================================\n");
    printf(" RESULTADO: %s\n", mismatches == 0 ? "CORRECTO" : "ERRORES");
    printf("========================================\n");

    return mismatches == 0 ? 0 : -1;
}
