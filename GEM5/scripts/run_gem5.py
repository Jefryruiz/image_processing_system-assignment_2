import m5
from m5.objects import *
from m5.objects.ImageAccelerator import ImageAccelerator
import os

SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)

DRIVER_BIN  = os.path.join(PROJECT_DIR, 'driver', 'accelerator_driver.elf')
INPUT_IMG   = os.path.join(PROJECT_DIR, 'images', 'input.raw')
OUTPUT_IMG  = os.path.join(PROJECT_DIR, 'images', 'output.raw')

if not os.path.exists(DRIVER_BIN):
    print(f"[run_gem5] ERROR: {DRIVER_BIN} no encontrado"); exit(1)
if not os.path.exists(INPUT_IMG):
    print(f"[run_gem5] ERROR: {INPUT_IMG} no encontrado"); exit(1)

print(f"[run_gem5] Driver: {DRIVER_BIN}")
print(f"[run_gem5] Input:  {INPUT_IMG} ({os.path.getsize(INPUT_IMG)} bytes)")

system = System()
system.clk_domain                = SrcClockDomain()
system.clk_domain.clock          = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode                  = 'atomic'
system.mem_ranges                = [AddrRange(0x00000000, size='64MiB')]

system.cpu = AtomicSimpleCPU()
system.cpu.clk_domain = SrcClockDomain(
    clock='1GHz', voltage_domain=VoltageDomain())
system.cpu.createInterruptController()

system.membus = SystemXBar()
system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports

system.mem_ctrl = SimpleMemory(
    range   = AddrRange(0x00000000, size='64MiB'),
    latency = '10ns'
)
system.mem_ctrl.port = system.membus.mem_side_ports

system.accel          = ImageAccelerator()
system.accel.cpu_side = system.membus.mem_side_ports

system.workload    = SEWorkload.init_compatible(DRIVER_BIN)
process            = Process()
process.executable = DRIVER_BIN
# Pasar rutas de entrada y salida como argumentos al driver
process.cmd        = [DRIVER_BIN, INPUT_IMG, OUTPUT_IMG]
system.cpu.workload = process
system.cpu.createThreads()

root        = Root(full_system=False)
root.system = system

print("=" * 60)
print(" GEM5 Virtual Prototype — RGB to Grayscale Accelerator")
print(" CPU:   ARM64 AtomicSimpleCPU @ 1GHz")
print(" RAM:   64MiB @ 0x00000000")
print(" Accel: registers @ 0x10000000-0x100000FF")
print("=" * 60)

m5.instantiate()

print("[run_gem5] Simulacion iniciada...")
exit_event = m5.simulate()

print(f"[run_gem5] Simulacion terminada: {exit_event.getCause()}")
print("=" * 60)
