#include "ram_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *g_mem = NULL;
static uint64_t g_size = 0;

void ram_model_init(uint64_t size) {
    if (g_mem != NULL && g_size == size) {
        memset(g_mem, 0, g_size);
        return;
    }
    free(g_mem);
    g_mem = (uint8_t *)calloc(1, size);
    g_size = size;
}

uint64_t ram_model_size(void) {
    return g_size;
}

void ram_model_write(uint64_t addr, const uint8_t *data, uint32_t len) {
    if (g_mem == NULL || addr + len > g_size) {
        return;
    }
    memcpy(g_mem + addr, data, len);
}

void ram_model_read(uint64_t addr, uint8_t *data, uint32_t len) {
    if (g_mem == NULL || addr + len > g_size) {
        memset(data, 0, len);
        return;
    }
    memcpy(data, g_mem + addr, len);
}

void ram_model_write_byte(uint64_t addr, uint8_t data) {
    if (g_mem == NULL || addr >= g_size) {
        return;
    }
    g_mem[addr] = data;
}

uint8_t ram_model_read_byte(uint64_t addr) {
    if (g_mem == NULL || addr >= g_size) {
        return 0;
    }
    return g_mem[addr];
}

int ram_model_load_file(uint64_t addr, const char *path, uint32_t len) {
    if (g_mem == NULL || addr + len > g_size) {
        return -1;
    }
    FILE *f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    size_t n = fread(g_mem + addr, 1, len, f);
    fclose(f);
    return (n == len) ? 0 : -1;
}

int ram_model_dump_file(uint64_t addr, const char *path, uint32_t len) {
    if (g_mem == NULL || addr + len > g_size) {
        return -1;
    }
    FILE *f = fopen(path, "wb");
    if (!f) {
        return -1;
    }
    size_t n = fwrite(g_mem + addr, 1, len, f);
    fclose(f);
    return (n == len) ? 0 : -1;
}

void ram_model_free(void) {
    free(g_mem);
    g_mem = NULL;
    g_size = 0;
}
