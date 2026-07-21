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
#   ~/gem5/build/ARM/gem5.opt --outdir=gem5_output scripts/run_gem5.py
# =============================================================================

import m5
from m5.objects import *

# Importar el acelerador registrado en GEM5
from m5.objects.ImageAccelerator import ImageAccelerator

import os

# -----------------------------------------------------------------------------
# Parametros
# -----------------------------------------------------------------------------
SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)

DRIVER_BIN  = os.path.join(PROJECT_DIR, 'driver', 'accelerator_driver.elf')
INPUT_IMG   = os.path.join(PROJECT_DIR, 'images', 'input.raw')

RAM_BASE    = 0x00000000
RAM_SIZE    = '64MiB'           # MiB para evitar el warning de base 10
ACCEL_BASE  = 0x10000000

# -----------------------------------------------------------------------------
# Verificar archivos necesarios
# -----------------------------------------------------------------------------
if not os.path.exists(DRIVER_BIN):
    print(f"[run_gem5] ERROR: Driver no encontrado: {DRIVER_BIN}")
    print("[run_gem5] Compilar con: aarch64-linux-gnu-gcc -static -O2 -march=armv8-a -o driver/accelerator_driver.elf driver/accelerator_driver.c")
    exit(1)

if not os.path.exists(INPUT_IMG):
    print(f"[run_gem5] ERROR: Imagen de entrada no encontrada: {INPUT_IMG}")
    exit(1)

print(f"[run_gem5] Driver:  {DRIVER_BIN}")
print(f"[run_gem5] Input:   {INPUT_IMG} ({os.path.getsize(INPUT_IMG)} bytes)")

# -----------------------------------------------------------------------------
# Sistema raiz
# -----------------------------------------------------------------------------
root = Root(full_system=False)

# -----------------------------------------------------------------------------
# Sistema
# -----------------------------------------------------------------------------
system = System()

system.clk_domain           = SrcClockDomain()
system.clk_domain.clock     = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode   = 'atomic'
system.mem_ranges = [AddrRange(RAM_BASE, size=RAM_SIZE)]

# -----------------------------------------------------------------------------
# CPU ARM64
# -----------------------------------------------------------------------------
system.cpu = AtomicSimpleCPU()
system.cpu.clk_domain = SrcClockDomain(
    clock='1GHz',
    voltage_domain=VoltageDomain()
)

# -----------------------------------------------------------------------------
# Bus del sistema
# -----------------------------------------------------------------------------
system.membus = SystemXBar()

system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports
system.cpu.createInterruptController()

# -----------------------------------------------------------------------------
# Memoria RAM (64 MiB)
# -----------------------------------------------------------------------------
system.mem_ctrl = SimpleMemory(
    range   = AddrRange(RAM_BASE, size=RAM_SIZE),
    latency = '10ns'
)
system.mem_ctrl.port = system.membus.mem_side_ports

# -----------------------------------------------------------------------------
# Acelerador de imagen
# -----------------------------------------------------------------------------
system.accel          = ImageAccelerator()
system.accel.cpu_side = system.membus.mem_side_ports

# -----------------------------------------------------------------------------
# Programa bare-metal (driver compilado para ARM64)
# -----------------------------------------------------------------------------
system.workload = SEWorkload.init_compatible(DRIVER_BIN)

process         = Process()
process.executable = DRIVER_BIN
process.cmd        = [DRIVER_BIN]

system.cpu.workload = process
system.cpu.createThreads()

# -----------------------------------------------------------------------------
# Iniciar simulacion
# -----------------------------------------------------------------------------
root.system = system

print("=" * 60)
print(" GEM5 Virtual Prototype — RGB to Grayscale Accelerator")
print(f" CPU:   ARM64 AtomicSimpleCPU @ 1GHz")
print(f" RAM:   {RAM_SIZE} @ 0x{RAM_BASE:08X}")
print(f" Accel: registers @ 0x{ACCEL_BASE:08X}")
print("=" * 60)

m5.instantiate()

print("[run_gem5] Simulacion iniciada...")
exit_event = m5.simulate()

print(f"[run_gem5] Simulacion terminada: {exit_event.getCause()}")
print("=" * 60)