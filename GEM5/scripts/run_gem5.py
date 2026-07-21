# =============================================================================
# run_gem5.py
# Script de configuracion del prototipo virtual GEM5.
#
# Arquitectura simulada:
#   ARM64 CPU (AtomicSimpleCPU)
#   └── SystemXBar
#       ├── SimpleMemory (64 MB @ 0x00000000)  ← RAM con imagen RGB
#       └── ImageAccelerator  (@ 0x10000000)   ← acelerador TLM 2.0
#
# Uso:
#   ~/gem5/build/ARM/gem5.opt scripts/run_gem5.py
#
# =============================================================================

import m5
from m5.objects import *
from m5.util   import addToPath
import os

# -----------------------------------------------------------------------------
# 1. Parametros del sistema
# -----------------------------------------------------------------------------
IMG_INPUT  = os.path.join(os.path.dirname(__file__), '..', 'images', 'input.raw')
IMG_OUTPUT = os.path.join(os.path.dirname(__file__), '..', 'images', 'output.raw')
DRIVER_BIN = os.path.join(os.path.dirname(__file__), '..', 'driver', 'accelerator_driver.elf')

RAM_SIZE        = '64MB'
RAM_BASE        = 0x00000000
ACCEL_BASE      = 0x10000000
ACCEL_REG_SIZE  = 0x100       # 256 bytes de registros

# -----------------------------------------------------------------------------
# 2. Sistema raiz
# -----------------------------------------------------------------------------
root = Root(full_system=False)

# -----------------------------------------------------------------------------
# 3. Sistema (CPU + memoria + bus)
# -----------------------------------------------------------------------------
system = System()
system.clk_domain     = SrcClockDomain()
system.clk_domain.clock     = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode  = 'atomic'          # AtomicSimpleCPU usa modo atomic
system.mem_ranges = [AddrRange(RAM_BASE, size=RAM_SIZE)]

# -----------------------------------------------------------------------------
# 4. CPU ARM64
# -----------------------------------------------------------------------------
system.cpu = AtomicSimpleCPU()
system.cpu.clk_domain = SrcClockDomain(
    clock='1GHz',
    voltage_domain=VoltageDomain()
)

# -----------------------------------------------------------------------------
# 5. Bus del sistema (XBar)
# -----------------------------------------------------------------------------
system.membus = SystemXBar()

# Conectar CPU al bus
system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports

# Interrupciones ARM
system.cpu.createInterruptController()

# -----------------------------------------------------------------------------
# 6. Memoria RAM (64 MB)
# Pre-carga la imagen RGB antes de que arranque la simulacion
# -----------------------------------------------------------------------------
system.mem_ctrl = SimpleMemory(
    range    = AddrRange(RAM_BASE, size=RAM_SIZE),
    latency  = '10ns'
)
system.mem_ctrl.port = system.membus.mem_side_ports

# -----------------------------------------------------------------------------
# 7. Acelerador de imagen (periférico TLM 2.0)
# -----------------------------------------------------------------------------
system.accel = ImageAccelerator()
system.accel.cpu_side = system.membus.mem_side_ports

# Registrar el rango de direcciones del acelerador en el bus
system.membus.badaddr_responder = BadAddr()
system.membus.badaddr_responder.pio = system.membus.mem_side_ports

# -----------------------------------------------------------------------------
# 8. Cargar imagen RGB en RAM antes de iniciar simulacion
# -----------------------------------------------------------------------------
def load_image_to_memory(system, filepath, base_addr):
    """
    Lee el archivo RAW RGB y lo carga en la memoria simulada
    en la direccion base especificada.
    """
    if not os.path.exists(filepath):
        print(f"[run_gem5] ERROR: No se encontro {filepath}")
        print(f"[run_gem5] Generando imagen de prueba...")
        # Generar imagen de prueba si no existe
        import struct
        W, H = 1920, 1080
        with open(filepath, 'wb') as f:
            for y in range(H):
                row = bytearray(W * 3)
                for x in range(W):
                    row[x*3+0] = (x * 255) // W
                    row[x*3+1] = (y * 255) // H
                    row[x*3+2] = (x + y) % 256
                f.write(row)
        print(f"[run_gem5] Imagen de prueba generada en {filepath}")

    with open(filepath, 'rb') as f:
        data = f.read()

    print(f"[run_gem5] Cargando {len(data)} bytes desde {filepath}")
    print(f"[run_gem5] Direccion base: 0x{base_addr:08X}")
    return data

# -----------------------------------------------------------------------------
# 9. Programa bare-metal (driver en C compilado para ARM64)
# -----------------------------------------------------------------------------
system.workload = SEWorkload.init_compatible(DRIVER_BIN)

process = Process()
process.executable = DRIVER_BIN
process.cmd        = [DRIVER_BIN]
system.cpu.workload = process
system.cpu.createThreads()

# -----------------------------------------------------------------------------
# 10. Configurar root e iniciar simulacion
# -----------------------------------------------------------------------------
root.system = system

print("=" * 60)
print(" GEM5 Virtual Prototype — RGB to Grayscale Accelerator")
print(" CPU: ARM64 AtomicSimpleCPU @ 1GHz")
print(f" RAM: {RAM_SIZE} @ 0x{RAM_BASE:08X}")
print(f" Accel registers @ 0x{ACCEL_BASE:08X}")
print("=" * 60)

m5.instantiate()

print("[run_gem5] Iniciando simulacion...")
exit_event = m5.simulate()

print(f"[run_gem5] Simulacion terminada: {exit_event.getCause()}")
print("=" * 60)
