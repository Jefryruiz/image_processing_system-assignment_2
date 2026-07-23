/**
 * accelerator_driver.c
 * 
 * Controlador bare-metal para el acelerador RGB a escala de grises.
 * Se ejecuta en ARM64 dentro del prototipo virtual de GEM5.
 * 
 * El acelerador está mapeado como un periférico de memoria.
 * La CPU interactúa con él escribiendo/leyendo registros de control
 * directamente a través de direcciones físicas.
 * 
 * Mapa de memoria:
 *   Base de RAM:          0x00000000  (64 MB)
 *   Imagen de entrada (RGB): 0x00000000  (6,220,800 B)
 *   Imagen de salida (Gris): 0x00600000  (2,073,600 B)
 *   Registros del acelerador: 0x10000000
 */
 
#include <stdint.h>
#include <string.h>
 
/* Memory map (must match defines.h / sys_cfg namespace)              */
#define RAM_BASE            0x00000000UL
#define INPUT_IMG_ADDR      0x00000000UL
#define OUTPUT_IMG_ADDR     0x00600000UL
 
#define ACCEL_BASE          0x10000000UL
#define REG_BASE_IN         (ACCEL_BASE + 0x00)
#define REG_BASE_OUT        (ACCEL_BASE + 0x04)
#define REG_NUM_PIXELS      (ACCEL_BASE + 0x08)
#define REG_CONTROL         (ACCEL_BASE + 0x0C)
#define REG_STATUS          (ACCEL_BASE + 0x10)
 
/* Image parameters                                                    */
#define IMG_WIDTH           1920
#define IMG_HEIGHT          1080
#define IMG_CHANNELS        3
#define IMG_SIZE_RGB        (IMG_WIDTH * IMG_HEIGHT * IMG_CHANNELS)  /* 6,220,800 B */
#define IMG_SIZE_GRAY       (IMG_WIDTH * IMG_HEIGHT)                 /* 2,073,600 B */
#define TOTAL_PIXELS        (IMG_WIDTH * IMG_HEIGHT)
 

/* Register access macros (volatile for memory-mapped I/O)            */

#define REG_WRITE(addr, val)  (*((volatile uint32_t *)(uintptr_t)(addr)) = (uint32_t)(val))
#define REG_READ(addr)        (*((volatile uint32_t *)(uintptr_t)(addr)))
 
#define MEM_WRITE8(addr, val) (*((volatile uint8_t  *)(uintptr_t)(addr)) = (uint8_t)(val))
#define MEM_READ8(addr)       (*((volatile uint8_t  *)(uintptr_t)(addr)))
 
/* Simple print (bare-metal: writes to GEM5 terminal via UART or m5)  */
/* 
 * In a real GEM5 bare-metal setup, stdout goes to the simulated UART.
 * We use a simple puts-style write here; replace with your BSP's
 * uart_puts() if needed.
 */
static void print_str(const char *s) {
    /* GEM5 ARM64 bare-metal: write to UART0 at 0x1C090000 (ARM Versatile) */
    volatile uint32_t *uart = (volatile uint32_t *)0x1C090000UL;
    while (*s) {
        while (uart[6] & (1 << 5)); /* wait until TX FIFO not full */
        uart[0] = (uint32_t)(*s++);
    }
}
 
static void print_hex(uint32_t val) {
    char buf[11];
    const char *hex = "0123456789ABCDEF";
    buf[0]  = '0'; buf[1] = 'x';
    buf[2]  = hex[(val >> 28) & 0xF];
    buf[3]  = hex[(val >> 24) & 0xF];
    buf[4]  = hex[(val >> 20) & 0xF];
    buf[5]  = hex[(val >> 16) & 0xF];
    buf[6]  = hex[(val >> 12) & 0xF];
    buf[7]  = hex[(val >>  8) & 0xF];
    buf[8]  = hex[(val >>  4) & 0xF];
    buf[9]  = hex[(val >>  0) & 0xF];
    buf[10] = '\0';
    print_str(buf);
}
 
/* ------------------------------------------------------------------ */
/* Step 1: Load image from storage into RAM                           */
/**
 * In bare-metal GEM5, the input image is pre-loaded into the
 * simulated RAM by the GEM5 script before simulation starts.
 * This function verifies the first few bytes are non-zero
 * to confirm the image was loaded correctly.
 *
 * Returns: 0 on success, -1 if memory appears empty.
 */
static int verify_input_image(void) {
    volatile uint8_t *img = (volatile uint8_t *)(uintptr_t)INPUT_IMG_ADDR;
    uint32_t nonzero = 0;
    uint32_t i;
 
    for (i = 0; i < 64; i++) {
        if (img[i] != 0) nonzero++;
    }
 
    return (nonzero > 0) ? 0 : -1;
}
 
/* ------------------------------------------------------------------ */
/* Step 2: Configure accelerator registers                            */
/* ------------------------------------------------------------------ */
static void accel_configure(uint32_t base_in,
                             uint32_t base_out,
                             uint32_t num_pixels) {
    print_str("[Driver] Configuring accelerator...\n");
 
    REG_WRITE(REG_BASE_IN,    base_in);
    REG_WRITE(REG_BASE_OUT,   base_out);
    REG_WRITE(REG_NUM_PIXELS, num_pixels);
 
    print_str("[Driver]   REG_BASE_IN    = "); print_hex(base_in);    print_str("\n");
    print_str("[Driver]   REG_BASE_OUT   = "); print_hex(base_out);   print_str("\n");
    print_str("[Driver]   REG_NUM_PIXELS = "); print_hex(num_pixels); print_str("\n");
}
 
/* ------------------------------------------------------------------ */
/* Step 3: Start accelerator                                          */
/* ------------------------------------------------------------------ */
static void accel_start(void) {
    print_str("[Driver] Starting accelerator (REG_CONTROL = 1)...\n");
    REG_WRITE(REG_CONTROL, 1);
}
 
/* ------------------------------------------------------------------ */
/* Step 4: Poll REG_STATUS until done                                 */
/* ------------------------------------------------------------------ */
/**
 * Polls REG_STATUS every ~1000 iterations (bare-metal busy-wait).
 * In a real system this would be replaced by an interrupt handler.
 *
 * Returns: number of poll iterations until done.
 */
static uint32_t accel_wait_done(void) {
    uint32_t count = 0;
    volatile uint32_t status;
 
    print_str("[Driver] Waiting for accelerator...\n");
 
    do {
        /* Busy-wait delay between polls */
        volatile uint32_t delay = 1000;
        while (delay--);
 
        status = REG_READ(REG_STATUS);
        count++;
    } while (status == 0);
 
    print_str("[Driver] Accelerator done after ");
    print_hex(count);
    print_str(" poll iterations.\n");
 
    return count;
}
 
/* ------------------------------------------------------------------ */
/* Step 5: Verify output image                                        */
/* ------------------------------------------------------------------ */
/**
 * Reads the first row of the grayscale output and checks it is
 * not identical to the input (basic sanity check).
 *
 * Returns: 0 if output looks valid, -1 if suspicious.
 */
static int verify_output_image(void) {
    volatile uint8_t *rgb  = (volatile uint8_t *)(uintptr_t)INPUT_IMG_ADDR;
    volatile uint8_t *gray = (volatile uint8_t *)(uintptr_t)OUTPUT_IMG_ADDR;
    uint32_t i;
    uint32_t mismatches = 0;
 
    /* Check first 10 pixels against BT.601 reference */
    for (i = 0; i < 10; i++) {
        uint8_t r = rgb[i * 3 + 0];
        uint8_t g = rgb[i * 3 + 1];
        uint8_t b = rgb[i * 3 + 2];
        uint8_t expected = (uint8_t)((299 * r + 587 * g + 114 * b) / 1000);
        uint8_t got      = gray[i];
 
        /* Allow ±1 rounding difference */
        int32_t diff = (int32_t)expected - (int32_t)got;
        if (diff < -1 || diff > 1) mismatches++;
    }
 
    if (mismatches == 0) {
        print_str("[Driver] Output verification PASSED.\n");
        return 0;
    } else {
        print_str("[Driver] Output verification FAILED.\n");
        return -1;
    }
}
 
/* ------------------------------------------------------------------ */
/* Main entry point                                                   */
/* ------------------------------------------------------------------ */
int main(void) {
    int ret;
 
    print_str("========================================\n");
    print_str(" ARM64 Bare-Metal Accelerator Driver\n");
    print_str(" RGB -> Grayscale (BT.601) via TLM 2.0\n");
    print_str("========================================\n");
 
    /* ---- Step 1: Verify input image is loaded in RAM ---- */
    print_str("[Driver] Step 1: Verifying input image in RAM...\n");
    ret = verify_input_image();
    if (ret != 0) {
        print_str("[Driver] ERROR: Input image not found in RAM.\n");
        return -1;
    }
    print_str("[Driver] Input image OK at ");
    print_hex(INPUT_IMG_ADDR);
    print_str(" (6,220,800 B RGB888)\n");
 
    /* ---- Step 2: Configure accelerator ---- */
    accel_configure(
        (uint32_t)INPUT_IMG_ADDR,   /* base_in  */
        (uint32_t)OUTPUT_IMG_ADDR,  /* base_out */
        (uint32_t)TOTAL_PIXELS      /* 1920 x 1080 */
    );
 
    /* ---- Step 3: Start accelerator ---- */
    accel_start();
 
    /* ---- Step 4: Wait for completion ---- */
    accel_wait_done();
 
    /* ---- Step 5: Verify output ---- */
    print_str("[Driver] Step 5: Verifying output image...\n");
    ret = verify_output_image();
 
    print_str("========================================\n");
    if (ret == 0) {
        print_str(" Simulation COMPLETE - SUCCESS\n");
    } else {
        print_str(" Simulation COMPLETE - CHECK OUTPUT\n");
    }
    print_str("========================================\n");
 
    /* GEM5: trigger simulation end */
    /* m5_exit(0) — uncomment if using m5 ops */
 
    return ret;
}