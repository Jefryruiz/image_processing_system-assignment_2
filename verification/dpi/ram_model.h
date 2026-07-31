#ifndef RAM_MODEL_H
#define RAM_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shared RAM behavioral model.
 *
 * This is the single C implementation of the 64 MB RAM used on both sides
 * of the DPI-C boundary:
 *  - the SystemVerilog/UVM testbench imports the byte/file functions below
 *    via `import "DPI-C"` to backdoor-load images and to check the RTL
 *    axi_ram.sv DUT against a golden reference,
 *  - the SystemC model (verification/systemc-image-processing-platform/src/ram_mem.h)
 *    links against this same object file and calls the bulk pointer API
 *    directly, so the RAM storage backing the TLM model is the very same
 *    module referenced by the RTL testbench.
 */

/* Allocates (or resets) the backing store. Safe to call multiple times. */
void ram_model_init(uint64_t size);

uint64_t ram_model_size(void);

/* Bulk pointer API - used directly from C/C++ (SystemC side). */
void ram_model_write(uint64_t addr, const uint8_t *data, uint32_t len);
void ram_model_read(uint64_t addr, uint8_t *data, uint32_t len);

/* Byte-granular API - safe to import from SystemVerilog via DPI-C
 * (SV has no raw pointer type, so bulk transfers from SV go through the
 * file helpers below instead of this pair). */
void ram_model_write_byte(uint64_t addr, uint8_t data);
uint8_t ram_model_read_byte(uint64_t addr);

/* File-backed helpers, also DPI-C importable (SV 'string' maps to const char*).
 * Returns 0 on success, -1 on error. */
int ram_model_load_file(uint64_t addr, const char *path, uint32_t len);
int ram_model_dump_file(uint64_t addr, const char *path, uint32_t len);

void ram_model_free(void);

#ifdef __cplusplus
}
#endif

#endif /* RAM_MODEL_H */
